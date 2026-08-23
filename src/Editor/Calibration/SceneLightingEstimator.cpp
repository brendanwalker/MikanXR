//-- includes -----
#include "SceneLightingEstimator.h"

#include "Logger.h"

#include <opencv2/opencv.hpp>

#include <cmath>

//-- SceneLightingEstimator -----
bool SceneLightingEstimator::startup(const Config& config)
{
	m_config= config;

	MarigoldInference::Config inferenceConfig;
	inferenceConfig.modelDirectory= config.modelDirectory;
	inferenceConfig.processingResolution= config.processingResolution;
	inferenceConfig.preferGpu= config.preferGpu;
	inferenceConfig.seed= config.seed;
	// Normals come from MoGe-2 below; skipping the normals UNet drops 3.4GB of
	// model load and one of the two denoise loops.
	inferenceConfig.bEnableNormals= false;

	if (!m_inference.startup(inferenceConfig))
		return false;

	MoGeInference::Config geometryConfig;
	geometryConfig.modelDirectory= config.mogeModelDirectory;
	geometryConfig.preferGpu= config.preferGpu;

	if (!m_geometryInference.startup(geometryConfig))
	{
		m_inference.shutdown();
		return false;
	}

	MIKAN_LOG_INFO("SceneLightingEstimator::startup")
		<< "Ready (IID: " << m_inference.getActiveExecutionProvider()
		<< ", geometry: " << m_geometryInference.getActiveExecutionProvider() << ")";

	return true;
}

void SceneLightingEstimator::shutdown()
{
	m_inference.shutdown();
	m_geometryInference.shutdown();
}

void SceneLightingEstimator::requestCancel()
{
	m_inference.requestCancel();
	m_geometryInference.requestCancel();
}

bool SceneLightingEstimator::estimate(const cv::Mat& bgrImage, const glm::mat3& cameraToWorldRotation,
									  float fovXDegrees, Result& outResult, const Progress& progress)
{
	if (!getIsInitialized())
	{
		MIKAN_LOG_ERROR("SceneLightingEstimator::estimate") << "Not initialized";
		return false;
	}

	const auto report= [&progress](eSceneLightingEstimatePhase phase, int completedUnits, int totalUnits)
	{
		if (progress.onProgress)
			progress.onProgress(phase, completedUnits, totalUnits);
	};
	const auto bIsCancelled= [&progress]() { return progress.isCancelled && progress.isCancelled(); };

	if (bIsCancelled())
		return false;

	// -- diffuse shading, the long one: a VAE encode, the DDIM loop, and three
	// decodes, each of which reports a unit as it lands.
	report(eSceneLightingEstimatePhase::decomposingShading, 0, 1);
	MarigoldInference::Result modelOutputs;
	const auto stepCallback= [&report, &bIsCancelled](int completedUnits, int totalUnits) -> bool
	{
		report(eSceneLightingEstimatePhase::decomposingShading, completedUnits, totalUnits);
		return !bIsCancelled();
	};
	if (!m_inference.run(bgrImage, modelOutputs, stepCallback))
		return false;

	if (bIsCancelled())
		return false;

	// -- surface normals: one opaque forward pass, so no sub-progress to report.
	report(eSceneLightingEstimatePhase::estimatingGeometry, 0, 1);
	MoGeInference::Result geometry;
	if (!m_geometryInference.run(bgrImage, fovXDegrees, geometry))
		return false;

	if (bIsCancelled())
		return false;

	report(eSceneLightingEstimatePhase::fittingLighting, 0, 1);

	// MoGe-2 normals are already in the fit's camera convention (+Z toward the
	// viewer) and zeroed where the validity mask rejected a pixel - a zero
	// normal fails the fit's unit-length check, so masking needs no extra code.
	modelOutputs.normals= geometry.normals;

	return fitFromModelOutputs(modelOutputs, cameraToWorldRotation, outResult);
}

cv::Mat SceneLightingEstimator::renderReconstructionImage(const Result& result, eReconstructionView view)
{
	const cv::Mat& normals= result.modelOutputs.normals;
	const cv::Mat& albedo= result.modelOutputs.albedo;
	const cv::Mat& shading= result.modelOutputs.shading;

	if (shading.empty() || shading.type() != CV_32FC3)
		return cv::Mat();
	if (view != eReconstructionView::modelShading && (normals.empty() || normals.size() != shading.size()))
		return cv::Mat();

	const int height= shading.rows;
	const int width= shading.cols;

	cv::Mat image(height, width, CV_8UC3);

	// Everything the models emit is linear - Marigold's prediction_space is
	// linear and the SH solve is radiometric - while a plate on screen is
	// display encoded. The same display transform is applied to every view so
	// they stay comparable to each other and to the video frame. This is a
	// display step only; it changes nothing about the fit.
	const auto encodeForDisplay= [](float linearValue) -> uint8_t
	{
		const float clamped= std::fmin(std::fmax(linearValue, 0.f), 1.f);
		return (uint8_t)(std::pow(clamped, 1.f / 2.2f) * 255.f + 0.5f);
	};

	for (int y= 0; y < height; ++y)
	{
		const cv::Vec3f* shadingRow= shading.ptr<cv::Vec3f>(y);
		const cv::Vec3f* normalRow= normals.empty() ? nullptr : normals.ptr<cv::Vec3f>(y);
		const cv::Vec3f* albedoRow= albedo.empty() ? nullptr : albedo.ptr<cv::Vec3f>(y);
		cv::Vec3b* outputRow= image.ptr<cv::Vec3b>(y);

		for (int x= 0; x < width; ++x)
		{
			// The model outputs are RGB - preprocess converts BGR to RGB before
			// inference - so index 0 is red here.
			glm::vec3 linearColor(0.f);

			if (view == eReconstructionView::modelShading)
			{
				linearColor= glm::vec3(shadingRow[x][0], shadingRow[x][1], shadingRow[x][2]);
			}
			else if (normalRow != nullptr)
			{
				const cv::Vec3f& n= normalRow[x];

				// The same rejection the fit uses: a normal that is not unit
				// length means the model had no confident surface there, so
				// there is nothing to light.
				const float lengthSquared= n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
				if (std::fabs(lengthSquared - 1.f) <= 0.2f)
				{
					linearColor= result.cameraSpaceEnvironment.evalIrradiance(glm::vec3(n[0], n[1], n[2]));
					// Order-2 SH rings negative around sharp lights; clamp it
					// exactly as the renderer and the Unreal skydome do.
					linearColor= glm::max(linearColor, glm::vec3(0.f));

					if (view == eReconstructionView::relit && albedoRow != nullptr)
						linearColor*= glm::vec3(albedoRow[x][0], albedoRow[x][1], albedoRow[x][2]);
				}
			}

			// OpenCV images are BGR.
			outputRow[x]= cv::Vec3b(encodeForDisplay(linearColor.b), encodeForDisplay(linearColor.g),
									encodeForDisplay(linearColor.r));
		}
	}

	return image;
}

bool SceneLightingEstimator::fitFromModelOutputs(const MarigoldInference::Result& modelOutputs,
												 const glm::mat3& cameraToWorldRotation, Result& outResult) const
{
	const cv::Mat& normals= modelOutputs.normals;
	const cv::Mat& shading= modelOutputs.shading;

	if (normals.empty() || shading.empty() || normals.size() != shading.size() || normals.type() != CV_32FC3
		|| shading.type() != CV_32FC3)
	{
		MIKAN_LOG_ERROR("SceneLightingEstimator::fitFromModelOutputs") << "Model outputs missing or malformed";
		return false;
	}

	SHLightingSolver solver;
	int rejected= 0;

	for (int y= 0; y < normals.rows; ++y)
	{
		const cv::Vec3f* normalRow= normals.ptr<cv::Vec3f>(y);
		const cv::Vec3f* shadingRow= shading.ptr<cv::Vec3f>(y);

		for (int x= 0; x < normals.cols; ++x)
		{
			const cv::Vec3f& n= normalRow[x];
			const cv::Vec3f& s= shadingRow[x];

			// A normal that is not unit length means the model had no confident
			// surface there.
			const float lengthSquared= n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
			if (std::fabs(lengthSquared - 1.f) > 0.2f)
			{
				rejected++;
				continue;
			}

			const float minShading= std::min(s[0], std::min(s[1], s[2]));
			const float maxShading= std::max(s[0], std::max(s[1], s[2]));

			// Fully black carries only visibility, and clipped-white is a lower
			// bound rather than a measurement. Both bias the fit.
			if (minShading <= m_config.blackThreshold || maxShading >= m_config.saturationThreshold)
			{
				rejected++;
				continue;
			}

			solver.addSample(glm::vec3(n[0], n[1], n[2]), glm::vec3(s[0], s[1], s[2]));
		}
	}

	if (!solver.solve(outResult.cameraSpaceEnvironment, m_config.bandRidge))
	{
		MIKAN_LOG_ERROR("SceneLightingEstimator::fitFromModelOutputs")
			<< "SH solve failed (" << solver.getSampleCount() << " usable samples of " << (normals.rows * normals.cols)
			<< ")";
		return false;
	}

	// The model returns camera-space normals, so the solved environment is in
	// camera space too. Rotate it into Mikan world space with the tracked pose.
	outResult.environment= outResult.cameraSpaceEnvironment.rotated(cameraToWorldRotation);

	outResult.directionality= outResult.environment.getDirectionality();
	outResult.keyLightDirection= outResult.environment.getDominantDirection();
	outResult.negativeSolidAngleFraction= outResult.environment.computeNegativeSolidAngleFraction();
	outResult.sampleCount= solver.getSampleCount();
	outResult.rejectedPixelCount= rejected;
	outResult.modelOutputs= modelOutputs;

	MIKAN_LOG_INFO("SceneLightingEstimator")
		<< "Fit " << outResult.sampleCount << " samples (" << rejected << " rejected)"
		<< ", directionality(l1/l0)=" << outResult.directionality << ", key dir=(" << outResult.keyLightDirection.x
		<< ", " << outResult.keyLightDirection.y << ", " << outResult.keyLightDirection.z << ")";

	if (outResult.directionality < 0.25f)
	{
		MIKAN_LOG_WARNING("SceneLightingEstimator")
			<< "Low directionality (" << outResult.directionality
			<< "): the scene is close to uniform ambient and the key light direction is not"
			   " meaningful. Treat the ambient term as the useful result.";
	}

	return true;
}
