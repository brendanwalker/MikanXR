#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "Graphs/CompositorNodeGraph.h"
#include "CameraMath.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlTriangulatedMesh.h"
#include "Logger.h"
#include "DepthMeshGenerator.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "MikanClientTypes.h"
#include "SyntheticDepthEstimator.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"
#include "VRDeviceView.h"

#include <algorithm>
#include <atomic>
#include <thread>

struct MonoLensDepthMeshCaptureState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;
	OpenCVCalibrationGeometry inputCVObjectGeometry;
	OpenGLCalibrationGeometry inputGLObjectGeometry;
	ProfileConfigConstPtr profileConfig;

	// Rendering state
	glm::dmat4 cameraToPatternXform;
	std::vector<glm::dvec3> cameraRelativePatternPoints;

	// Generated mesh
	double disparityBias;
	double disparityScale;
	GlTriangulatedMeshPtr depthMeshPtr;

	void init(ProfileConfigConstPtr config, VideoSourceViewPtr videoSourceView)
	{
		profileConfig= config;

		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

		inputCameraIntrinsics = cameraIntrinsics.intrinsics.mono;

		resetCalibration();
	}

	void resetCalibration()
	{
		disparityBias= 0.0;
		disparityScale= 1.0;
		depthMeshPtr= nullptr;
	}

	void computeCameraRelativePatternPoints(std::vector<glm::dvec3>& outCameraRelativePoints)
	{
		// Compute the camera relative points of the calibration pattern
		// using the cameraToPatternXform computed by OpenCV
		outCameraRelativePoints.clear();
		for (int pointIndex= 0; pointIndex < 0; pointIndex++)
		{
			// Compute the 3D point in the pattern's local space
			const glm::dvec4 localPoint(inputGLObjectGeometry.points[pointIndex], 1.0);

			// Transform the pattern point into the camera's space
			const glm::dvec3 cameraPoint = cameraToPatternXform * localPoint;

			// Store the camera relative point
			outCameraRelativePoints.push_back(cameraPoint);
		}
	}

	void sampleSyntheticDisparityMap(
		CalibrationPatternFinderPtr patternFinder,
		const t_opencv_point2d_list& patternPoints,
		const cv::Mat* syntheticDisparityDnnBuffer,
		std::vector<float> outSyntheticDepths)
	{
		const float patternFrameWidth = patternFinder->getFrameWidth();
		const float patternFrameHeight = patternFinder->getFrameHeight();
		const float depthFrameWidth = (float)syntheticDisparityDnnBuffer->cols;
		const float depthFrameHeight = (float)syntheticDisparityDnnBuffer->rows;

		// Sample the synthetic disparity map from the camera at the 2d pattern locations
		for (const cv::Point2f& patternPoint : patternPoints)
		{
			// Convert the pattern point to the depth map space
			const int depthX = int((patternPoint.x / patternFrameWidth) * depthFrameWidth);
			const int depthY = int((patternPoint.y / patternFrameHeight) * depthFrameHeight);

			// Sample the synthetic disparity at the pattern point
			const float syntheticDisparity = syntheticDisparityDnnBuffer->at<float>(depthY, depthX);
			outSyntheticDepths.push_back(syntheticDisparity);
		}
	}

	void computeDisparityBiasAndScale(
		SyntheticDepthEstimatorPtr depthEstimator,
		CalibrationPatternFinderPtr patternFinder,
		const t_opencv_point2d_list& imagePoints)
	{
		VideoFrameDistortionView* distortionView = patternFinder->getDistortionView();
		const cv::Mat* floatDepthDnnBuffer = depthEstimator->getFloatDepthDnnBuffer();

		// True depth is computed using the formula: Z = 1.0 / (A + (B * synthetic_disparity))
		// where A and B are bias and scale constants needed to map disparity values from the MiDaS DNN model.
		// Given the true depth from the calibration pattern, we can derive A and B using a linear regression
		// of the form y = slope*x + intercept, where y = 1.0 / Z, x = synthetic_disparity, slope = B, intercept = A.

		// Compute camera relative chessboard points using the cameraToPatternXform computed by OpenCV
		computeCameraRelativePatternPoints(cameraRelativePatternPoints);

		// Sample the synthetic disparity map from the camera at the 2d pattern locations
		std::vector<float> syntheticDepts;
		sampleSyntheticDisparityMap(patternFinder, imagePoints, floatDepthDnnBuffer, syntheticDepts);

		// Merge the real depth and synthetic disparity pairs into a single array
		std::vector<cv::Vec2d> depthDisparityPairs; // (1/true_z, disparity)
		for (int i = 0; i < imagePoints.size(); i++)
		{
			const glm::dvec3 cameraRelativePoint = cameraRelativePatternPoints[i];
			const double trueZ = cameraRelativePoint.z;
			const double invTrueZ = 1.0f / trueZ;
			const double syntheticDisparity = syntheticDepts[i];

			depthDisparityPairs.push_back(cv::Vec2f(invTrueZ, syntheticDisparity));
		}
		
		// Run a linear regression on the pairs to compute the disparity bias and scale
		cv::Vec4d lineParams;
		cv::fitLine(depthDisparityPairs, lineParams, cv::DIST_L2, 0, 0.01, 0.01);
		const double vx= lineParams[0];
		const double vy= lineParams[1];
		const double x0 = lineParams[2];
		const double y0 = lineParams[3];

		disparityScale = vy / vx;
		disparityBias = y0 - (disparityScale*x0);
	}

	bool createDepthMesh(
		VideoFrameDistortionViewPtr distortionView,
		SyntheticDepthEstimatorPtr depthEstimator)
	{
		cv::Matx33d intrinsicMatrix = MikanMatrix3d_to_cv_mat33d(inputCameraIntrinsics.camera_matrix);
		cv::Matx33d invIntrinsicMatrix = intrinsicMatrix.inv();

		cv::Mat* syntheticDisparityDnnBuffer= depthEstimator->getFloatDepthDnnBuffer();
		const float frameWidth = (float)distortionView->getFrameWidth();
		const float frameHeight = (float)distortionView->getFrameHeight();
		const int depthFrameWidth = syntheticDisparityDnnBuffer->cols;
		const int depthFrameHeight = syntheticDisparityDnnBuffer->rows;
		const float frameVStep = frameWidth / depthFrameWidth;
		const float frameUStep = frameHeight / depthFrameHeight;

		glm::vec3* meshVertices = new glm::vec3[depthFrameWidth * depthFrameHeight];
		const uint32_t triangleCount = (depthFrameWidth - 1) * (depthFrameHeight - 1) * 2;
		uint32_t* meshIndices = new uint32_t[triangleCount * 3];

		float frameU= 0.0f;
		float frameV= 0.0f;
		for (int depthV = 0; depthV < depthFrameHeight; depthV++)
		{
			for (int depthU = 0; depthU < depthFrameWidth; depthU++)
			{
				// Fetch depth from the synthetic disparity map
				const float disparity = syntheticDisparityDnnBuffer->at<float>(depthV, depthU);
				const float depth = 1.0f / (disparityBias + (disparityScale * disparity));

				// Compute the 3D point in the camera space
				const cv::Vec3d cameraPoint = invIntrinsicMatrix * cv::Vec3d(frameU, frameV, 1.0);
				const float openCV_x = cameraPoint[0] * depth;
				const float openCV_y = cameraPoint[1] * depth;
				const float openCV_z = depth;

				// OpenCV -> OpenGL coordinate system transform
				// Rendering world units in meters, not mm
				glm::vec3 openGLPoint(
					openCV_x * k_millimeters_to_meters,
					-openCV_y * k_millimeters_to_meters,
					-openCV_z * k_millimeters_to_meters);

				// Store the 3D point in the vertex array
				*meshVertices = openGLPoint;
				meshVertices++;

				frameU+= frameUStep;
			}

			frameU= 0;
			frameV+= frameVStep;
		}

		depthMeshPtr = std::make_shared<GlTriangulatedMesh>(
			"depth_mesh",
			CompositorNodeGraph::getStencilModelVertexDefinition(),
			(const uint8_t*)meshVertices,
			4, // 4 verts
			(const uint8_t*)meshIndices,
			sizeof(uint32_t), // 4 bytes per index
			triangleCount, 
			true); // mesh owns the vertex data
		if (!depthMeshPtr->createBuffers())
		{
			MIKAN_LOG_ERROR("DrawLayerNode::createLayerQuadMeshes()") << "Failed to create layer mesh";
			return false;
		}

		return true;
	}
};

//-- MonoDistortionCalibrator ----
DepthMeshGenerator::DepthMeshGenerator(
	ProfileConfigConstPtr profileConfig,
	VideoFrameDistortionViewPtr distortionView,
	SyntheticDepthEstimatorPtr depthEstimator)
	: m_calibrationState(new MonoLensDepthMeshCaptureState)
	, m_distortionView(distortionView)
	, m_patternFinder(CalibrationPatternFinder::allocatePatternFinderSharedPtr(profileConfig, distortionView.get()))
	, m_depthEstimator(depthEstimator)
{

	frameWidth = distortionView->getFrameWidth();
	frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(profileConfig, distortionView->getVideoSourceView());

	// Cache the 3d geometry of the calibration pattern in the calibration state
	m_patternFinder->getOpenCVSolvePnPGeometry(&m_calibrationState->inputCVObjectGeometry);
	m_patternFinder->getOpenGLSolvePnPGeometry(&m_calibrationState->inputGLObjectGeometry);
}

DepthMeshGenerator::~DepthMeshGenerator()
{
	m_patternFinder= nullptr;
	delete m_calibrationState;
}

bool DepthMeshGenerator::hasFinishedSampling() const
{
	return m_calibrationState->depthMeshPtr != nullptr;
}

void DepthMeshGenerator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

bool DepthMeshGenerator::captureMesh()
{
	// Look for the calibration pattern in the latest video frame
	if (!m_patternFinder->findNewCalibrationPattern())
	{
		return false;
	}

	cv::Point2f boundingQuad[4];
	t_opencv_point2d_list imagePoints;
	m_patternFinder->fetchLastFoundCalibrationPattern(imagePoints, boundingQuad);

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the calibration pattern relative to the camera
	cv::Quatd cv_cameraToPatternRot;
	cv::Vec3d cv_cameraToPatternVecMM; // Millimeters
	if (!computeOpenCVCameraRelativePatternTransform(
			m_calibrationState->inputCameraIntrinsics,
			imagePoints,
			m_calibrationState->inputCVObjectGeometry.points,
			cv_cameraToPatternRot,
			cv_cameraToPatternVecMM))
	{
		return false;
	}
	convertOpenCVCameraRelativePoseToGLMMat(
		cv_cameraToPatternRot, cv_cameraToPatternVecMM, 
		m_calibrationState->cameraToPatternXform);

	// Compute the disparity bias and scale using the calibration pattern
	m_calibrationState->computeDisparityBiasAndScale(
		m_depthEstimator,
		m_patternFinder, 
		imagePoints);

	// Convert the true depth map to a textured mesh
	// Store data in a GlStaticMeshInstancePtr for temp rendering
	m_calibrationState->createDepthMesh(m_distortionView, m_depthEstimator);

	return true;
}

void DepthMeshGenerator::renderCameraSpaceCalibrationState()
{
	// Draw the most recently capture chessboard in camera space
	m_patternFinder->renderCalibrationPattern2D();

	// Draw the camera relative transforms of the pattern (computed from solvePnP)
	// and the mat puck location offset from the pattern origin
	if (m_calibrationState->depthMeshPtr != nullptr)
	{
		const glm::mat4 patternXform = glm::mat4(m_calibrationState->cameraToPatternXform);

		// Compute the mat puck location relative to the mat transform we computed
		ProfileConfigConstPtr config = m_calibrationState->profileConfig;
		const float xOffset = -config->puckHorizontalOffsetMM * k_millimeters_to_meters;
		const float yOffset = config->puckDepthOffsetMM * k_millimeters_to_meters;
		const float zOffset = config->puckVerticalOffsetMM * k_millimeters_to_meters;
		const glm::mat4 matPuckOffsetXform =
			glm::translate(
				glm::mat4(1.0),
				glm::vec3(xOffset, yOffset, zOffset));
		const glm::mat4 matPuckXForm = patternXform * matPuckOffsetXform;

		drawTransformedAxes(patternXform, 0.1f);
		drawTransformedAxes(matPuckXForm, 0.1f);
	}
}

void DepthMeshGenerator::renderVRSpaceCalibrationState()
{
#if 0
	// Draw the most recently captured chessboard projected into VR
	m_patternFinder->renderSolvePnPPattern3D(m_calibrationState->patternXform);

	// Draw the camera puck transform
	const glm::mat4 cameraPuckXform = glm::dmat4(m_cameraTrackingPuckView->getCalibrationPose());
	drawTransformedAxes(cameraPuckXform, 0.1f);

	// Draw the most recently derived camera transform derived from the mat puck
	const float hfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.hfov);
	const float vfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.vfov);
	const float zNear= fmaxf(m_calibrationState->inputCameraIntrinsics.znear, 0.1f);
	const float zFar = fminf(m_calibrationState->inputCameraIntrinsics.zfar, 2.0f);
	drawTransformedFrustum(
		m_calibrationState->cameraXform,
		hfov_radians, vfov_radians,
		zNear, zFar,
		Colors::Yellow);
	drawTransformedAxes(m_calibrationState->cameraXform, 0.1f);
#endif
}

bool DepthMeshGenerator::loadMeshFromObjFile(const std::filesystem::path& objPath)
{
	return true;
}

bool DepthMeshGenerator::saveMeshToObjFile(const std::filesystem::path& objPath)
{
	return true;
}

bool DepthMeshGenerator::loadTextureFromPNG(const std::filesystem::path& texturePath)
{
	//TODO
	return true;
}

bool DepthMeshGenerator::saveTextureToPNG(const std::filesystem::path& texturePath)
{
	//TODO
	//SdlUtility::saveTextureToPNG(colorTexture.get(), texturePath.c_str());

	return true;
}
