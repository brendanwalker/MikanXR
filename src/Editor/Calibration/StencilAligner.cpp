#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "CameraMath.h"
#include "Colors.h"
#include "SdlCommon.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "InputManager.h"
#include "StencilAligner.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "ModelStencilComponent.h"
#include "SceneComponent.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"
#include "VRDeviceView.h"

#include <algorithm>
#include <atomic>
#include <thread>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

struct StencilAlignmentState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;

	// Sample State
	t_opencv_point2d_list pixelSamples;
	t_opencv_point3d_list cvLocalVertexSamples;
	std::vector<glm::vec3> glLocalVertexSamples;

	void init(VideoSourceViewPtr videoSourceView)
	{
		// Get the current mono camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);
		inputCameraIntrinsics= cameraIntrinsics.getMonoIntrinsics();

		resetCalibration();
	}

	void resetCalibration()
	{
		pixelSamples.clear();
		cvLocalVertexSamples.clear();
		glLocalVertexSamples.clear();
	}
};

//-- MonoDistortionCalibrator ----
StencilAligner::StencilAligner(
	VRDevicePoseViewPtr cameraTrackingPuckPoseView,
	VideoFrameDistortionView* distortionView,
	ModelStencilComponentPtr modelStencil)
	: m_calibrationState(new StencilAlignmentState)
	, m_cameraTrackingPuckPoseView(cameraTrackingPuckPoseView)
	, m_distortionView(distortionView)
	, m_modelStencil(modelStencil)
{
	m_frameWidth = distortionView->getFrameWidth();
	m_frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(distortionView->getVideoSourceView());
}

StencilAligner::~StencilAligner()
{
	delete m_calibrationState;
}

bool StencilAligner::hasFinishedPointSampling() const
{
	return
		m_calibrationState->pixelSamples.size() >= DESIRED_SAMPLE_COUNT && 
		m_calibrationState->cvLocalVertexSamples.size() >= DESIRED_SAMPLE_COUNT;
}

void StencilAligner::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

void StencilAligner::samplePixel(const glm::vec2& pixel)
{
	const cv::Point2d cv_pointSample(pixel.x, pixel.y);

	m_calibrationState->pixelSamples.push_back(cv_pointSample);
}

void StencilAligner::sampleVertex(const glm::vec3& localVertex)
{
	// Convert the local vertex to OpenCV coordinates
	// * Mikan units are in meters, but SolvePnP want points in millimeters
	// * Convert from OpenGL coordinate system to OpenCV coordinate system
	// (x, y, z) -> (x, -y, -z)
	const cv::Point3d cv_localVertex(
		localVertex.x * k_meters_to_millimeters, 
		-localVertex.y * k_meters_to_millimeters, 
		-localVertex.z * k_meters_to_millimeters);

	m_calibrationState->cvLocalVertexSamples.push_back(cv_localVertex);
	m_calibrationState->glLocalVertexSamples.push_back(localVertex);
}

bool StencilAligner::computeStencilTransform(glm::mat4& outStencilTransform)
{
	if (m_calibrationState->pixelSamples.size() < DESIRED_SAMPLE_COUNT ||
		m_calibrationState->cvLocalVertexSamples.size() < DESIRED_SAMPLE_COUNT)
		return false;

	// Make sure mono camera intrinsics are available
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_distortionView->getVideoSourceView()->getCameraIntrinsics(cameraIntrinsics);
	if (cameraIntrinsics.intrinsics_type != MONO_CAMERA_INTRINSICS)
	{
		return false;
	}

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the calibration pattern relative to the camera
	cv::Quatd cv_cameraToStencilRot;
	cv::Vec3d cv_cameraToStencilVecMM; // Millimeters
	const auto& monoIntrinsics= cameraIntrinsics.getMonoIntrinsics();
	if (!computeOpenCVCameraRelativePatternTransform(
		monoIntrinsics,
		m_calibrationState->pixelSamples,
		m_calibrationState->cvLocalVertexSamples,
		cv_cameraToStencilRot,
		cv_cameraToStencilVecMM))
	{
		return false;
	}

	// Convert OpenCV pose (in mm) to OpenGL pose (in meters)
	glm::dmat4 cameraToStencilXform;
	convertOpenCVCameraRelativePoseToGLMMat(
		cv_cameraToStencilRot, cv_cameraToStencilVecMM,
		cameraToStencilXform);

	// Compute world transform from the current camera pose
	glm::mat4 cameraPose;
	if (!m_distortionView->getVideoSourceView()->getCameraPose(m_cameraTrackingPuckPoseView, cameraPose))
	{
		return false;
	}

	outStencilTransform = glm_composite_xform(cameraToStencilXform, cameraPose);

	return true;
}

void StencilAligner::renderPixelSamples()
{
	glm::vec3 glm_points[4];
	const int pointCount = (int)m_calibrationState->pixelSamples.size();
	assert(pointCount <= 4);

	for (int i = 0; i < pointCount; i++)
	{
		const cv::Point2f& imagePoint = m_calibrationState->pixelSamples[i];

		glm_points[i] = glm::vec3(imagePoint.x, imagePoint.y, 0.5f);

		TextStyle style = getDefaultTextStyle();
		style.horizontalAlignment = eHorizontalTextAlignment::Middle;
		style.verticalAlignment = eVerticalTextAlignment::Bottom;
		drawTextAtCameraPosition(
			style,
			m_frameWidth, m_frameHeight,
			glm_points[i],
			L"P%d", i);

		if (i > 0)
		{
			glm::vec3 color = Colors::White;
			switch (i)
			{
				case 1: color = Colors::Red; break;
				case 2: color = Colors::Green; break;
				case 3: color = Colors::Blue; break;
			}
			drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[i], color, color);
		}
	}

	drawPointList2d(m_frameWidth, m_frameHeight, glm_points, pointCount, Colors::Yellow, 2.f);
}

void StencilAligner::renderVertexSamples()
{
	const glm::mat4& xform= m_modelStencil->getWorldTransform();
	const int pointCount = (int)m_calibrationState->glLocalVertexSamples.size();

	for (int i = 0; i < pointCount; i++)
	{
		const glm::vec3& localVertex = m_calibrationState->glLocalVertexSamples[i];
		glm::vec3 worldVertex = glm::vec3(xform * glm::vec4(localVertex, 1.f));

		TextStyle style = getDefaultTextStyle();
		style.horizontalAlignment = eHorizontalTextAlignment::Middle;
		style.verticalAlignment = eVerticalTextAlignment::Bottom;
		drawTextAtWorldPosition(
			style,
			worldVertex,
			L"P%d", i);

		if (i > 0)
		{
			glm::vec3 color = Colors::White;
			switch (i)
			{
				case 1: color = Colors::Red; break;
				case 2: color = Colors::Green; break;
				case 3: color = Colors::Blue; break;
			}
			drawSegment(xform, m_calibrationState->glLocalVertexSamples[0], localVertex, color, color);
		}
	}
}