//-- includes -----
#include "MoGeInference.h"

#include "Logger.h"
#include "OnnxSession.h"

#include <opencv2/opencv.hpp>

#include <cmath>
#include <filesystem>

//-- constants -----
/// The shift solve runs on a nearest-downsampled point map, matching the
/// reference implementation. 64x64 is an approximation that is accurate to
/// well below the model's own error and keeps the solve trivially cheap.
static constexpr int k_shiftSolveResolution= 64;

//-- helpers -----
/// UV grid with corners at (+-width/diagonal, +-height/diagonal), sampled at
/// pixel centers of a grid downsampled with INTER_NEAREST semantics. The
/// focal recovered against these UVs is relative to half the image diagonal.
static void computeViewPlaneUv(int sourceWidth, int sourceHeight, int gridWidth, int gridHeight, cv::Mat& outUv)
{
	const float aspectRatio= (float)sourceWidth / (float)sourceHeight;
	const float spanX= aspectRatio / std::sqrt(1.f + aspectRatio * aspectRatio);
	const float spanY= 1.f / std::sqrt(1.f + aspectRatio * aspectRatio);

	// The UVs live on the SOURCE pixel grid (linspace over source pixels);
	// nearest-downsampling then picks the same source pixels cv::resize picks
	// for the point map, keeping uv and xyz paired exactly as the reference
	// implementation pairs them.
	cv::Mat fullUv(sourceHeight, sourceWidth, CV_32FC2);
	for (int y= 0; y < sourceHeight; ++y)
	{
		cv::Vec2f* row= fullUv.ptr<cv::Vec2f>(y);
		const float v= (sourceHeight > 1) ? spanY * (2.f * (float)y / (float)(sourceHeight - 1) - 1.f)
												* (float)(sourceHeight - 1) / (float)sourceHeight
										  : 0.f;
		for (int x= 0; x < sourceWidth; ++x)
		{
			const float u= (sourceWidth > 1) ? spanX * (2.f * (float)x / (float)(sourceWidth - 1) - 1.f)
												   * (float)(sourceWidth - 1) / (float)sourceWidth
											 : 0.f;
			row[x]= cv::Vec2f(u, v);
		}
	}

	cv::resize(fullUv, outUv, cv::Size(gridWidth, gridHeight), 0.0, 0.0, cv::INTER_NEAREST);
}

float MoGeInference::solveDepthShift(const cv::Mat& pointsCam, const cv::Mat& validMask, float focal)
{
	const cv::Size gridSize(k_shiftSolveResolution, k_shiftSolveResolution);

	cv::Mat pointsLr, maskLr, uvLr;
	cv::resize(pointsCam, pointsLr, gridSize, 0.0, 0.0, cv::INTER_NEAREST);
	cv::resize(validMask, maskLr, gridSize, 0.0, 0.0, cv::INTER_NEAREST);
	computeViewPlaneUv(pointsCam.cols, pointsCam.rows, gridSize.width, gridSize.height, uvLr);

	std::vector<cv::Vec2f> xy, uv;
	std::vector<float> z;
	for (int y= 0; y < gridSize.height; ++y)
	{
		const cv::Vec3f* pointRow= pointsLr.ptr<cv::Vec3f>(y);
		const cv::Vec2f* uvRow= uvLr.ptr<cv::Vec2f>(y);
		const uint8_t* maskRow= maskLr.ptr<uint8_t>(y);
		for (int x= 0; x < gridSize.width; ++x)
		{
			if (maskRow[x] == 0)
				continue;
			xy.push_back(cv::Vec2f(pointRow[x][0], pointRow[x][1]));
			z.push_back(pointRow[x][2]);
			uv.push_back(uvRow[x]);
		}
	}
	if (z.size() < 2)
		return 0.f;

	// 1-D Levenberg-Marquardt. The residual is smooth wherever every
	// denominator z + shift stays positive - which the true solution
	// guarantees, since z + shift IS the depth. Starting at zero is only safe
	// when all z are already positive; an affine map with negative z puts a
	// pole between zero and the solution and the solve stalls against it. So
	// start just past the largest pole instead. (The reference implementation
	// starts scipy's LM at zero unconditionally; this is the one deliberate
	// difference, and it agrees with the reference wherever the reference
	// converges.)
	const auto computeCost= [&](double shift) -> double
	{
		double cost= 0.0;
		for (size_t i= 0; i < z.size(); ++i)
		{
			const double denominator= (double)z[i] + shift;
			const double du= (double)focal * (double)xy[i][0] / denominator - (double)uv[i][0];
			const double dv= (double)focal * (double)xy[i][1] / denominator - (double)uv[i][1];
			cost+= du * du + dv * dv;
		}
		return cost;
	};

	float minZ= z[0], maxZ= z[0];
	for (float zi : z)
	{
		minZ= std::min(minZ, zi);
		maxZ= std::max(maxZ, zi);
	}
	const double feasibilityMargin= 1e-3 * std::max(1.f, maxZ - minZ);

	double shift= std::max(0.0, (double)-minZ + feasibilityMargin);
	double damping= 1e-3;
	double cost= computeCost(shift);
	for (int iteration= 0; iteration < 20; ++iteration)
	{
		double gradient= 0.0, hessian= 0.0;
		for (size_t i= 0; i < z.size(); ++i)
		{
			const double denominator= (double)z[i] + shift;
			const double du= (double)focal * (double)xy[i][0] / denominator - (double)uv[i][0];
			const double dv= (double)focal * (double)xy[i][1] / denominator - (double)uv[i][1];
			const double ju= -(double)focal * (double)xy[i][0] / (denominator * denominator);
			const double jv= -(double)focal * (double)xy[i][1] / (denominator * denominator);
			gradient+= ju * du + jv * dv;
			hessian+= ju * ju + jv * jv;
		}

		const double step= -gradient / (hessian * (1.0 + damping));
		const double newCost= computeCost(shift + step);
		if (newCost < cost)
		{
			shift+= step;
			if (std::fabs(cost - newCost) < 1e-3 * cost)
				break;
			cost= newCost;
			damping= std::max(damping * 0.3, 1e-9);
		}
		else
		{
			damping*= 10.0;
		}
	}

	return (float)shift;
}

//-- private types -----
struct MoGeInference::Impl
{
	Config config;
	OnnxSession session;

	// Output tensor indices resolved by NAME at startup - exporters do not
	// order outputs consistently, and points/normal share a shape signature.
	int pointsOutputIndex= -1;
	int normalOutputIndex= -1;
	int maskOutputIndex= -1;
	int metricScaleOutputIndex= -1;

	bool resolveOutputIndices()
	{
		struct OutputToFind
		{
			int* index;
			const char* name;
		};
		const OutputToFind outputs[]= {{&pointsOutputIndex, "points"},
									   {&normalOutputIndex, "normal"},
									   {&maskOutputIndex, "mask"},
									   {&metricScaleOutputIndex, "metric_scale"}};

		for (const OutputToFind& output : outputs)
		{
			*output.index= -1;
			for (size_t i= 0; i < session.getOutputCount(); ++i)
			{
				if (session.getOutputName(i) == output.name)
				{
					*output.index= (int)i;
					break;
				}
			}
			if (*output.index < 0)
			{
				MIKAN_LOG_ERROR("MoGeInference::startup") << "Model is missing expected output '" << output.name << "'";
				return false;
			}
		}
		return true;
	}
};

//-- MoGeInference -----
MoGeInference::MoGeInference()
	: m_impl(std::make_unique<Impl>())
{
}

MoGeInference::~MoGeInference() { shutdown(); }

const char* MoGeInference::getActiveExecutionProvider() const { return m_impl ? m_impl->session.activeEp() : "none"; }

bool MoGeInference::startup(const Config& config)
{
	shutdown();
	m_impl->config= config;

	const std::filesystem::path modelPath= std::filesystem::path(config.modelDirectory) / "model.onnx";
	if (!std::filesystem::exists(modelPath))
	{
		MIKAN_LOG_ERROR("MoGeInference::startup") << "Missing model file: " << modelPath.string();
		return false;
	}

	const std::string preferredEp= config.preferGpu ? "directml" : "cpu";
	if (!m_impl->session.create(modelPath.string(), preferredEp))
		return false;

	if (!m_impl->resolveOutputIndices())
	{
		shutdown();
		return false;
	}

	MIKAN_LOG_INFO("MoGeInference::startup")
		<< "Loaded MoGe-2 model from " << config.modelDirectory << " (EP: " << m_impl->session.activeEp() << ", "
		<< config.numTokens << " tokens)";

	m_bIsInitialized= true;
	return true;
}

void MoGeInference::shutdown() { m_bIsInitialized= false; }

bool MoGeInference::run(const cv::Mat& bgrImage, float fovXDegrees, Result& outResult)
{
	if (!m_bIsInitialized)
		return false;

	if (bgrImage.empty() || bgrImage.type() != CV_8UC3)
	{
		MIKAN_LOG_ERROR("MoGeInference::run") << "Expected a non-empty CV_8UC3 image";
		return false;
	}
	if (fovXDegrees <= 0.f || fovXDegrees >= 180.f)
	{
		MIKAN_LOG_ERROR("MoGeInference::run") << "Invalid horizontal FOV: " << fovXDegrees;
		return false;
	}

	const int height= bgrImage.rows;
	const int width= bgrImage.cols;

	// -- preprocess: plain RGB in [0,1] at native resolution, NCHW. ImageNet
	// normalization is baked into the exported graph.
	cv::Mat rgb;
	cv::cvtColor(bgrImage, rgb, cv::COLOR_BGR2RGB);
	cv::Mat asFloat;
	rgb.convertTo(asFloat, CV_32FC3, 1.0 / 255.0);

	std::vector<float> imageNchw((size_t)3 * height * width);
	{
		std::vector<cv::Mat> channels(3);
		for (int c= 0; c < 3; ++c)
			channels[c]= cv::Mat(height, width, CV_32FC1, imageNchw.data() + (size_t)c * height * width);
		cv::split(asFloat, channels);
	}

	const std::vector<int64_t> imageShape= {1, 3, height, width};
	Ort::Value imageTensor= Ort::Value::CreateTensor<float>(OnnxSession::getCpuMemoryInfo(), imageNchw.data(),
															imageNchw.size(), imageShape.data(), imageShape.size());

	int64_t numTokensValue= (int64_t)m_impl->config.numTokens;
	Ort::Value numTokensTensor=
		Ort::Value::CreateTensor<int64_t>(OnnxSession::getCpuMemoryInfo(), &numTokensValue, 1, nullptr, 0);

	Ort::Value inputs[2]= {std::move(imageTensor), std::move(numTokensTensor)};
	const int outputIndices[4]= {m_impl->pointsOutputIndex, m_impl->normalOutputIndex, m_impl->maskOutputIndex,
								 m_impl->metricScaleOutputIndex};
	std::vector<Ort::Value> outputs= m_impl->session.runOutputs(inputs, 2, outputIndices, 4);
	if (outputs.size() != 4)
	{
		MIKAN_LOG_ERROR("MoGeInference::run") << "Inference failed";
		return false;
	}

	// Raw model outputs, still in OpenCV camera convention (+Y down, +Z fwd).
	// The tensors are [1,H,W,C] / [1,H,W]; wrap without copying.
	const float* pointsData= outputs[0].GetTensorData<float>();
	const float* normalData= outputs[1].GetTensorData<float>();
	const float* maskProbabilityData= outputs[2].GetTensorData<float>();
	const float metricScale= outputs[3].GetTensorData<float>()[0];

	const cv::Mat rawPoints(height, width, CV_32FC3, const_cast<float*>(pointsData));
	const cv::Mat rawNormals(height, width, CV_32FC3, const_cast<float*>(normalData));
	const cv::Mat maskProbability(height, width, CV_32FC1, const_cast<float*>(maskProbabilityData));

	cv::Mat mask;
	cv::compare(maskProbability, 0.5, mask, cv::CMP_GT); // 255 where valid
	mask/= 255;                                          // -> 0/1

	// -- recover the Z shift against the calibrated focal, then apply the
	// metric scale. Focal here is relative to half the image diagonal.
	const float aspectRatio= (float)width / (float)height;
	const float focal=
		aspectRatio / std::sqrt(1.f + aspectRatio * aspectRatio) / std::tan(fovXDegrees * 3.14159265f / 360.f);
	const float shift= solveDepthShift(rawPoints, mask, focal);

	// Normalized intrinsics: fx in units of width, fy of height, center 0.5.
	const float fx= focal / 2.f * std::sqrt(1.f + aspectRatio * aspectRatio) / aspectRatio;
	const float fy= focal / 2.f * std::sqrt(1.f + aspectRatio * aspectRatio);

	outResult.depth.create(height, width, CV_32FC1);
	outResult.points.create(height, width, CV_32FC3);
	outResult.normals.create(height, width, CV_32FC3);
	outResult.mask.create(height, width, CV_8UC1);

	const float infinity= std::numeric_limits<float>::infinity();
	for (int y= 0; y < height; ++y)
	{
		const cv::Vec3f* rawPointRow= rawPoints.ptr<cv::Vec3f>(y);
		const cv::Vec3f* rawNormalRow= rawNormals.ptr<cv::Vec3f>(y);
		const uint8_t* maskRow= mask.ptr<uint8_t>(y);

		float* depthRow= outResult.depth.ptr<float>(y);
		cv::Vec3f* pointRow= outResult.points.ptr<cv::Vec3f>(y);
		cv::Vec3f* normalRow= outResult.normals.ptr<cv::Vec3f>(y);
		uint8_t* outMaskRow= outResult.mask.ptr<uint8_t>(y);

		for (int x= 0; x < width; ++x)
		{
			// Depth must be positive after the shift; a non-positive value means
			// the affine map put the pixel behind the camera and it is invalid.
			const float shiftedZ= rawPointRow[x][2] + shift;
			const bool bValid= maskRow[x] != 0 && shiftedZ > 0.f;
			outMaskRow[x]= bValid ? 1 : 0;

			if (!bValid)
			{
				depthRow[x]= infinity;
				pointRow[x]= cv::Vec3f(infinity, infinity, infinity);
				normalRow[x]= cv::Vec3f(0.f, 0.f, 0.f);
				continue;
			}

			const float depthMeters= shiftedZ * metricScale;
			depthRow[x]= depthMeters;

			// force_projection: rebuild the point from depth through the
			// calibrated intrinsics so points reproject exactly, then convert
			// OpenCV camera space to Mikan camera space (+Y up, -Z forward).
			const float u= ((float)x + 0.5f) / (float)width;
			const float v= ((float)y + 0.5f) / (float)height;
			pointRow[x]= cv::Vec3f((u - 0.5f) / fx * depthMeters, -(v - 0.5f) / fy * depthMeters, -depthMeters);

			// Same Y/Z negation for the normals.
			normalRow[x]= cv::Vec3f(rawNormalRow[x][0], -rawNormalRow[x][1], -rawNormalRow[x][2]);
		}
	}

	return true;
}
