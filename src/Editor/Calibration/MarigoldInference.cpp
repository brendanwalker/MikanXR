//-- includes -----
#include "MarigoldInference.h"

#include "Logger.h"
#include "OnnxSession.h"

#include <opencv2/opencv.hpp>

#include <nlohmann/json.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <random>

//-- private types -----
struct MarigoldInference::Impl
{
	Config config;

	OnnxSession vaeEncoder;
	OnnxSession vaeDecoder;
	OnnxSession unetIid;
	OnnxSession unetNormals;

	std::vector<float> emptyTextEmbedding;
	std::vector<int64_t> emptyTextEmbeddingShape;

	std::vector<double> alphasCumprod;
	std::vector<int> timesteps;
	int numTrainTimesteps= 1000;
	bool bClipSample= false;

	// -- helpers --------------------------------------------------------
	static Ort::Value makeTensor(std::vector<float>& storage, const std::vector<int64_t>& shape)
	{
		return Ort::Value::CreateTensor<float>(OnnxSession::getCpuMemoryInfo(), storage.data(), storage.size(),
											   shape.data(), shape.size());
	}

	/// Resize so the long edge matches the processing resolution, then pad by
	/// replication up to the alignment the exported graphs require.
	cv::Mat preprocess(const cv::Mat& bgrImage, cv::Size& outPaddedSize, cv::Size& outResizedSize) const
	{
		cv::Mat rgb;
		cv::cvtColor(bgrImage, rgb, cv::COLOR_BGR2RGB);

		const int maxEdge= std::max(rgb.cols, rgb.rows);
		const int newWidth= rgb.cols * config.processingResolution / maxEdge;
		const int newHeight= rgb.rows * config.processingResolution / maxEdge;

		cv::Mat resized;
		// INTER_AREA is the antialiased downscale, matching what Marigold's
		// preprocessing intends.
		cv::resize(rgb, resized, cv::Size(newWidth, newHeight), 0.0, 0.0, cv::INTER_AREA);
		outResizedSize= resized.size();

		const int padRight= (k_marigoldImageAlignment - newWidth % k_marigoldImageAlignment) % k_marigoldImageAlignment;
		const int padBottom=
			(k_marigoldImageAlignment - newHeight % k_marigoldImageAlignment) % k_marigoldImageAlignment;

		cv::Mat padded;
		cv::copyMakeBorder(resized, padded, 0, padBottom, 0, padRight, cv::BORDER_REPLICATE);
		outPaddedSize= padded.size();

		cv::Mat asFloat;
		padded.convertTo(asFloat, CV_32FC3, 2.0 / 255.0, -1.0);

		return asFloat;
	}

	static void hwcToNchw(const cv::Mat& hwc, std::vector<float>& outNchw)
	{
		const int height= hwc.rows;
		const int width= hwc.cols;
		outNchw.resize((size_t)3 * height * width);

		std::vector<cv::Mat> channels(3);
		for (int c= 0; c < 3; ++c)
			channels[c]= cv::Mat(height, width, CV_32FC1, outNchw.data() + (size_t)c * height * width);
		cv::split(hwc, channels);
	}

	static cv::Mat nchwToHwc(const float* nchw, int height, int width)
	{
		std::vector<cv::Mat> channels(3);
		for (int c= 0; c < 3; ++c)
			channels[c]= cv::Mat(height, width, CV_32FC1, const_cast<float*>(nchw) + (size_t)c * height * width);

		cv::Mat hwc;
		cv::merge(channels, hwc);

		return hwc.clone();
	}

	/// Undo the padding and resize back to the source resolution.
	cv::Mat postprocess(const cv::Mat& padded, const cv::Size& resizedSize, const cv::Size& originalSize) const
	{
		cv::Mat unpadded= padded(cv::Rect(0, 0, resizedSize.width, resizedSize.height));

		if (unpadded.size() == originalSize)
			return unpadded.clone();

		cv::Mat resized;
		cv::resize(unpadded, resized, originalSize, 0.0, 0.0, cv::INTER_LINEAR);

		return resized;
	}

	bool encodeImage(const cv::Mat& imageHwc, std::vector<float>& outLatent, int& outLatentH, int& outLatentW)
	{
		std::vector<float> input;
		hwcToNchw(imageHwc, input);

		const std::vector<int64_t> shape= {1, 3, imageHwc.rows, imageHwc.cols};
		Ort::Value tensor= makeTensor(input, shape);

		std::vector<Ort::Value> outputs= vaeEncoder.run(&tensor, 1);
		if (outputs.empty())
			return false;

		const auto info= outputs[0].GetTensorTypeAndShapeInfo();
		const std::vector<int64_t> outShape= info.GetShape();
		if (outShape.size() != 4 || outShape[1] != 4)
			return false;

		outLatentH= (int)outShape[2];
		outLatentW= (int)outShape[3];

		const float* data= outputs[0].GetTensorData<float>();
		const size_t count= info.GetElementCount();
		outLatent.resize(count);
		for (size_t i= 0; i < count; ++i)
			outLatent[i]= data[i] * k_marigoldVaeScale;

		return true;
	}

	cv::Mat decodeLatent(const float* latent, int latentH, int latentW)
	{
		std::vector<float> scaled((size_t)4 * latentH * latentW);
		for (size_t i= 0; i < scaled.size(); ++i)
			scaled[i]= latent[i] / k_marigoldVaeScale;

		const std::vector<int64_t> shape= {1, 4, latentH, latentW};
		Ort::Value tensor= makeTensor(scaled, shape);

		std::vector<Ort::Value> outputs= vaeDecoder.run(&tensor, 1);
		if (outputs.empty())
			return cv::Mat();

		const auto info= outputs[0].GetTensorTypeAndShapeInfo();
		const std::vector<int64_t> outShape= info.GetShape();
		if (outShape.size() != 4 || outShape[1] != 3)
			return cv::Mat();

		return nchwToHwc(outputs[0].GetTensorData<float>(), (int)outShape[2], (int)outShape[3]);
	}

	/// One DDIM step with v_prediction and eta = 0. Mirrors DDIMScheduler::step.
	void ddimStep(const float* modelOutput, int timestep, std::vector<float>& sample) const
	{
		const int stepSize= numTrainTimesteps / (int)timesteps.size();
		const int prevTimestep= timestep - stepSize;

		const double alphaT= alphasCumprod[timestep];
		// set_alpha_to_one is false for these checkpoints, so the final step
		// falls back to alphas_cumprod[0] rather than 1.0.
		const double alphaPrev= (prevTimestep >= 0) ? alphasCumprod[prevTimestep] : alphasCumprod[0];
		const double betaT= 1.0 - alphaT;

		const double sqrtAlphaT= std::sqrt(alphaT);
		const double sqrtBetaT= std::sqrt(betaT);
		const double sqrtAlphaPrev= std::sqrt(alphaPrev);
		const double sqrtOneMinusAlphaPrev= std::sqrt(1.0 - alphaPrev);

		for (size_t i= 0; i < sample.size(); ++i)
		{
			const double v= (double)modelOutput[i];
			const double s= (double)sample[i];

			double predOriginal= sqrtAlphaT * s - sqrtBetaT * v;
			const double predEpsilon= sqrtAlphaT * v + sqrtBetaT * s;

			if (bClipSample)
				predOriginal= std::min(1.0, std::max(-1.0, predOriginal));

			sample[i]= (float)(sqrtAlphaPrev * predOriginal + sqrtOneMinusAlphaPrev * predEpsilon);
		}
	}

	bool denoise(OnnxSession& unet, const std::vector<float>& imageLatent, int latentH, int latentW, int targetCount,
				 unsigned int seed, std::vector<float>& outPredLatent)
	{
		const size_t planeSize= (size_t)latentH * latentW;
		const size_t predSize= planeSize * 4 * targetCount;

		// Deterministic Gaussian initialization.
		std::mt19937 generator(seed);
		std::normal_distribution<float> gaussian(0.f, 1.f);
		outPredLatent.resize(predSize);
		for (size_t i= 0; i < predSize; ++i)
			outPredLatent[i]= gaussian(generator);

		std::vector<float> concatenated(imageLatent.size() + predSize);
		const std::vector<int64_t> sampleShape= {1, (int64_t)(4 * (1 + targetCount)), latentH, latentW};

		for (int timestep : timesteps)
		{
			std::copy(imageLatent.begin(), imageLatent.end(), concatenated.begin());
			std::copy(outPredLatent.begin(), outPredLatent.end(), concatenated.begin() + imageLatent.size());

			Ort::Value sampleTensor= makeTensor(concatenated, sampleShape);

			int64_t timestepValue= timestep;
			Ort::Value timestepTensor=
				Ort::Value::CreateTensor<int64_t>(OnnxSession::getCpuMemoryInfo(), &timestepValue, 1, nullptr, 0);

			Ort::Value textTensor= Ort::Value::CreateTensor<float>(
				OnnxSession::getCpuMemoryInfo(), emptyTextEmbedding.data(), emptyTextEmbedding.size(),
				emptyTextEmbeddingShape.data(), emptyTextEmbeddingShape.size());

			Ort::Value inputs[3]= {std::move(sampleTensor), std::move(timestepTensor), std::move(textTensor)};
			std::vector<Ort::Value> outputs= unet.run(inputs, 3);
			if (outputs.empty())
				return false;

			ddimStep(outputs[0].GetTensorData<float>(), timestep, outPredLatent);
		}

		return true;
	}
};

//-- MarigoldInference -----
MarigoldInference::MarigoldInference()
	: m_impl(std::make_unique<Impl>())
{
}

MarigoldInference::~MarigoldInference() { shutdown(); }

const char* MarigoldInference::getActiveExecutionProvider() const
{
	return m_impl ? m_impl->unetIid.activeEp() : "none";
}

bool MarigoldInference::startup(const Config& config)
{
	shutdown();
	m_impl->config= config;

	const std::filesystem::path root(config.modelDirectory);
	const std::string preferredEp= config.preferGpu ? "directml" : "cpu";

	struct ModelToLoad
	{
		OnnxSession* session;
		const char* fileName;
	};
	const ModelToLoad models[]= {{&m_impl->vaeEncoder, "vae_encoder.onnx"},
								 {&m_impl->vaeDecoder, "vae_decoder.onnx"},
								 {&m_impl->unetIid, "unet_iid_lighting.onnx"},
								 {&m_impl->unetNormals, "unet_normals.onnx"}};

	for (const ModelToLoad& model : models)
	{
		const std::filesystem::path path= root / model.fileName;
		if (!std::filesystem::exists(path))
		{
			MIKAN_LOG_ERROR("MarigoldInference::startup") << "Missing model file: " << path.string();
			shutdown();
			return false;
		}

		if (!model.session->create(path.string(), preferredEp))
		{
			shutdown();
			return false;
		}
	}

	// Empty text embedding. Marigold tokenizes the empty prompt with
	// padding="do_not_pad", so this is [1,2,1024], NOT the usual 77 tokens.
	const std::filesystem::path embeddingPath= root / "empty_text_embedding.bin";
	std::ifstream embeddingFile(embeddingPath, std::ios::binary | std::ios::ate);
	if (!embeddingFile)
	{
		MIKAN_LOG_ERROR("MarigoldInference::startup") << "Missing " << embeddingPath.string();
		shutdown();
		return false;
	}
	const std::streamsize embeddingBytes= embeddingFile.tellg();
	embeddingFile.seekg(0);
	m_impl->emptyTextEmbedding.resize((size_t)embeddingBytes / sizeof(float));
	embeddingFile.read((char*)m_impl->emptyTextEmbedding.data(), embeddingBytes);

	// Scheduler constants.
	const std::filesystem::path schedulerPath= root / "scheduler.json";
	std::ifstream schedulerFile(schedulerPath);
	if (!schedulerFile)
	{
		MIKAN_LOG_ERROR("MarigoldInference::startup") << "Missing " << schedulerPath.string();
		shutdown();
		return false;
	}

	try
	{
		nlohmann::json schedulerJson;
		schedulerFile >> schedulerJson;

		m_impl->numTrainTimesteps= schedulerJson.value("num_train_timesteps", 1000);
		m_impl->bClipSample= schedulerJson.value("clip_sample", false);
		m_impl->alphasCumprod= schedulerJson.at("alphas_cumprod").get<std::vector<double>>();
		m_impl->timesteps= schedulerJson.at("timesteps_4step").get<std::vector<int>>();
		m_impl->emptyTextEmbeddingShape= schedulerJson.at("empty_text_embedding_shape").get<std::vector<int64_t>>();

		const std::string predictionType= schedulerJson.value("prediction_type", std::string("v_prediction"));
		if (predictionType != "v_prediction")
		{
			// ddimStep only implements v_prediction; anything else would run and
			// silently produce garbage rather than fail.
			MIKAN_LOG_ERROR("MarigoldInference::startup")
				<< "Unsupported scheduler prediction_type '" << predictionType << "' (expected v_prediction)";
			shutdown();
			return false;
		}
	}
	catch (const std::exception& e)
	{
		MIKAN_LOG_ERROR("MarigoldInference::startup") << "Failed to parse scheduler.json: " << e.what();
		shutdown();
		return false;
	}

	MIKAN_LOG_INFO("MarigoldInference::startup")
		<< "Loaded Marigold models from " << config.modelDirectory << " (EP: " << m_impl->unetIid.activeEp() << ", "
		<< m_impl->timesteps.size() << " denoise steps)";

	m_bIsInitialized= true;
	return true;
}

void MarigoldInference::shutdown()
{
	m_bIsInitialized= false;
	if (m_impl)
	{
		m_impl->emptyTextEmbedding.clear();
		m_impl->alphasCumprod.clear();
		m_impl->timesteps.clear();
	}
}

bool MarigoldInference::run(const cv::Mat& bgrImage, Result& outResult)
{
	if (!m_bIsInitialized)
		return false;

	if (bgrImage.empty() || bgrImage.type() != CV_8UC3)
	{
		MIKAN_LOG_ERROR("MarigoldInference::run") << "Expected a non-empty CV_8UC3 image";
		return false;
	}

	const cv::Size originalSize= bgrImage.size();
	cv::Size paddedSize, resizedSize;
	const cv::Mat preprocessed= m_impl->preprocess(bgrImage, paddedSize, resizedSize);

	std::vector<float> imageLatent;
	int latentH= 0, latentW= 0;
	if (!m_impl->encodeImage(preprocessed, imageLatent, latentH, latentW))
	{
		MIKAN_LOG_ERROR("MarigoldInference::run") << "VAE encode failed";
		return false;
	}

	const size_t planeSize= (size_t)latentH * latentW;

	// -- intrinsics: albedo, shading, residual --
	std::vector<float> iidLatent;
	if (!m_impl->denoise(m_impl->unetIid, imageLatent, latentH, latentW, 3, m_impl->config.seed, iidLatent))
	{
		MIKAN_LOG_ERROR("MarigoldInference::run") << "Intrinsics denoise failed";
		return false;
	}

	cv::Mat* const targets[3]= {&outResult.albedo, &outResult.shading, &outResult.residual};
	for (int targetIndex= 0; targetIndex < 3; ++targetIndex)
	{
		cv::Mat decoded= m_impl->decodeLatent(iidLatent.data() + (size_t)targetIndex * 4 * planeSize, latentH, latentW);
		if (decoded.empty())
		{
			MIKAN_LOG_ERROR("MarigoldInference::run") << "VAE decode failed for target " << targetIndex;
			return false;
		}

		// [-1,1] -> [0,1]
		cv::Mat clipped;
		cv::max(cv::min(decoded, 1.0), -1.0, clipped);
		clipped= (clipped + 1.0) * 0.5;

		*targets[targetIndex]= m_impl->postprocess(clipped, resizedSize, originalSize);
	}

	// -- normals --
	std::vector<float> normalsLatent;
	if (!m_impl->denoise(m_impl->unetNormals, imageLatent, latentH, latentW, 1, m_impl->config.seed + 1, normalsLatent))
	{
		MIKAN_LOG_ERROR("MarigoldInference::run") << "Normals denoise failed";
		return false;
	}

	cv::Mat decodedNormals= m_impl->decodeLatent(normalsLatent.data(), latentH, latentW);
	if (decodedNormals.empty())
	{
		MIKAN_LOG_ERROR("MarigoldInference::run") << "VAE decode failed for normals";
		return false;
	}

	cv::Mat clippedNormals;
	cv::max(cv::min(decodedNormals, 1.0), -1.0, clippedNormals);

	// This checkpoint sets use_full_z_range, so there is no z remap - just
	// renormalize to unit length.
	outResult.normals= m_impl->postprocess(clippedNormals, resizedSize, originalSize);
	for (int y= 0; y < outResult.normals.rows; ++y)
	{
		cv::Vec3f* row= outResult.normals.ptr<cv::Vec3f>(y);
		for (int x= 0; x < outResult.normals.cols; ++x)
		{
			const float length= std::sqrt(row[x][0] * row[x][0] + row[x][1] * row[x][1] + row[x][2] * row[x][2]);
			if (length > 1e-12f)
				row[x]/= length;
		}
	}

	return true;
}
