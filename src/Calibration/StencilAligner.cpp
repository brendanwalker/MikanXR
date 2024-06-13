#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "CameraMath.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "InputManager.h"
#include "StencilAligner.h"
#include "AnchorObjectSystem.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "MathGLM.h"
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
	t_opencv_point3d_list cvVertexSamples;
	std::vector<glm::vec3> glVertexSamples;

	// Output State
	glm::mat4 stencilWorldXform;

	void init(VideoSourceViewPtr videoSourceView)
	{
		// Get the current mono camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);
		inputCameraIntrinsics= cameraIntrinsics.intrinsics.mono;

		resetCalibration();
	}

	void resetCalibration()
	{
		pixelSamples.clear();
		cvVertexSamples.clear();
		glVertexSamples.clear();
		stencilWorldXform= glm::mat4(1.f);
	}
};

//-- MonoDistortionCalibrator ----
StencilAligner::StencilAligner(
	VRDeviceViewPtr cameraTrackingPuckView,
	VideoFrameDistortionView* distortionView)
	: m_calibrationState(new StencilAlignmentState)
	, m_cameraTrackingPuckView(cameraTrackingPuckView)
	, m_distortionView(distortionView)
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
		m_calibrationState->cvVertexSamples.size() >= DESIRED_SAMPLE_COUNT;
}

void StencilAligner::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

void StencilAligner::sampleMouseScreenPosition()
{
	const glm::vec2 pointSample= computeMouseScreenPosition();
	const cv::Point2d cv_pointSample(pointSample.x, pointSample.y);

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

	m_calibrationState->cvVertexSamples.push_back(cv_localVertex);
	m_calibrationState->glVertexSamples.push_back(localVertex);
}

glm::vec2 StencilAligner::computeMouseScreenPosition() const
{
	int mouseScreenX = 0, mouseScreenY;
	InputManager::getInstance()->getMouseScreenPosition(mouseScreenX, mouseScreenY);

	MainWindow* window = MainWindow::getInstance();
	const float screenWidth = window->getWidth();
	const float screenHeight = window->getHeight();

	glm::vec2 pointSample(
		((float)mouseScreenX * m_frameWidth) / screenWidth,
		((float)mouseScreenY * m_frameHeight) / screenHeight);

	return pointSample;
}

bool StencilAligner::computeStencilTransform(StencilAlignerInfo& stencilInfo)
{
	if (m_calibrationState->pixelSamples.size() < DESIRED_SAMPLE_COUNT ||
		m_calibrationState->cvVertexSamples.size() < DESIRED_SAMPLE_COUNT)
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
	if (!computeOpenCVCameraRelativePatternTransform(
		cameraIntrinsics.intrinsics.mono,
		m_calibrationState->pixelSamples,
		m_calibrationState->cvVertexSamples,
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
	glm::mat4 cameraPose= m_distortionView->getVideoSourceView()->getCameraPose(m_cameraTrackingPuckView);
	m_calibrationState->stencilWorldXform= glm_composite_xform(cameraToStencilXform, cameraPose);

	// Update the stencil component with the new world transform
	StencilComponentPtr stencilComponentPtr =
		StencilObjectSystem::getSystem()->getStencilById(stencilInfo.stencilId);
	stencilComponentPtr->setWorldTransform(m_calibrationState->stencilWorldXform);

	return true;
}

#if 0
void StencilAligner::renderInitialPoint2dSegements()
{
	glm::vec3 glm_points[3];
	const int pointCount = (int)m_calibrationState->pixelSamples.size();

	for (int i = 0; i < pointCount; i++)
	{
		const cv::Point2f& imagePoint= m_calibrationState->pixelSamples[i];

		glm_points[i]= glm::vec3(imagePoint.x, imagePoint.y, 0.5f);

		TextStyle style = getDefaultTextStyle();
		style.horizontalAlignment = eHorizontalTextAlignment::Middle;
		style.verticalAlignment = eVerticalTextAlignment::Bottom;
		drawTextAtCameraPosition(
			style,
			m_frameWidth, m_frameHeight,
			glm_points[i],
			L"P%d", i);
	}

	if (pointCount >= 2)
	{
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[1], Colors::DarkRed, Colors::Red);
	}

	if (pointCount >= 3)
	{
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[2], Colors::DarkGreen, Colors::Green);
	}

	if (pointCount >= 4)
	{
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[4], Colors::DarkBlue, Colors::Blue);
	}

	drawPointList2d(m_frameWidth, m_frameHeight, glm_points, pointCount, Colors::Yellow, 2.f);
}

void StencilAligner::renderAllTriangulatedPoints(bool bShowCameraFrustum)
{
	const glm::vec3 p0= m_calibrationState->glVertexSamples[0];
	const glm::vec3 p1= m_calibrationState->glVertexSamples[0];
	const glm::vec3 p2= m_calibrationState->glVertexSamples[0];

	drawSegment(glm::mat4(1.f), p0, p1, Colors::Red);
	drawSegment(glm::mat4(1.f), p0, p2, Colors::Green);

	TextStyle style = getDefaultTextStyle();
	for (int i = 0; i < 3; i++)
	{
		drawTextAtWorldPosition(style, m_calibrationState->triangulatedPointSamples[i], L"P%d", i);
	}

	if (bShowCameraFrustum)
	{
		// Draw the most recently derived camera transform derived from the mat puck
		const glm::mat4 glm_camera_xform = m_distortionView->getVideoSourceView()->getCameraPose(m_cameraTrackingPuckView);
		const float hfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.hfov);
		const float vfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.vfov);
		const float zNear = fmaxf(m_calibrationState->inputCameraIntrinsics.znear, 0.1f);
		const float zFar = fminf(m_calibrationState->inputCameraIntrinsics.zfar, 2.0f);
		drawTransformedFrustum(
			glm_camera_xform,
			hfov_radians, vfov_radians,
			zNear, zFar,
			Colors::Yellow);
		drawTransformedAxes(glm_camera_xform, 0.1f);
	}
}

void StencilAligner::renderAnchorTransform()
{
	drawTransformedAxes(m_calibrationState->stencilWorldXform, 0.1f, 0.1f, 0.1f);
}
#endif