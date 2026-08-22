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

bool SceneLightingEstimator::estimate(const cv::Mat& bgrImage, const glm::mat3& cameraToWorldRotation,
									  float fovXDegrees, Result& outResult)
{
	if (!getIsInitialized())
	{
		MIKAN_LOG_ERROR("SceneLightingEstimator::estimate") << "Not initialized";
		return false;
	}

	MarigoldInference::Result modelOutputs;
	if (!m_inference.run(bgrImage, modelOutputs))
		return false;

	MoGeInference::Result geometry;
	if (!m_geometryInference.run(bgrImage, fovXDegrees, geometry))
		return false;

	// MoGe-2 normals are already in the fit's camera convention (+Z toward the
	// viewer) and zeroed where the validity mask rejected a pixel - a zero
	// normal fails the fit's unit-length check, so masking needs no extra code.
	modelOutputs.normals= geometry.normals;

	return fitFromModelOutputs(modelOutputs, cameraToWorldRotation, outResult);
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
