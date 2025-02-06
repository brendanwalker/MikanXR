#include "AnchorComponent.h"
#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "CameraMath.h"
#include "Colors.h"
#include "SdlCommon.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "InputManager.h"
#include "AnchorTriangulator.h"
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

static const glm::vec3 k_anchorRayColors[3] = {Colors::Red, Colors::Green, Colors::Blue};

struct AnchorTriangulationState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;

	// Sample State
	glm::vec2 initialPointSamples[3];
	glm::mat4 initialCameraPoseSample;
	int initialPointSampleCount;
	glm::vec3 lastWorldTriangulatedPoint;
	glm::vec3 triangulatedPointSamples[3];
	int triangulatedPointSampleCount;

	// Output State
	glm::mat4 anchorWorldXform;

	void init(VideoSourceViewPtr videoSourceView)
	{
		// Get the current mono camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		inputCameraIntrinsics= cameraIntrinsics.getMonoIntrinsics();

		initialCameraPoseSample = glm::mat4(1.f);
		resetCalibration();
	}

	void resetCalibration()
	{
		initialPointSamples[0]= glm::vec2();
		initialPointSamples[1]= glm::vec2();
		initialPointSamples[2]= glm::vec2();
		initialPointSampleCount= 0;
		lastWorldTriangulatedPoint= glm::vec3();
		triangulatedPointSamples[0] = glm::vec3();
		triangulatedPointSamples[1] = glm::vec3();
		triangulatedPointSamples[2] = glm::vec3();
		triangulatedPointSampleCount= 0;
		anchorWorldXform= glm::mat4(1.f);
	}
};

//-- MonoDistortionCalibrator ----
AnchorTriangulator::AnchorTriangulator(
	VRDevicePoseViewPtr cameraTrackingPuckPoseView,
	VideoFrameDistortionView* distortionView)
	: m_calibrationState(new AnchorTriangulationState)
	, m_cameraTrackingPuckPoseView(cameraTrackingPuckPoseView)
	, m_distortionView(distortionView)
{
	m_frameWidth = distortionView->getFrameWidth();
	m_frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(distortionView->getVideoSourceView());
}

AnchorTriangulator::~AnchorTriangulator()
{
	delete m_calibrationState;
}

bool AnchorTriangulator::hasFinishedInitialPointSampling() const
{
	return m_calibrationState->initialPointSampleCount >= 3;
}

bool AnchorTriangulator::hasFinishedTriangulatedPointSampling() const
{
	return m_calibrationState->triangulatedPointSampleCount >= 3;
}

void AnchorTriangulator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

void AnchorTriangulator::sampleMouseScreenPosition()
{
	const glm::vec2 pointSample= computeMouseScreenPosition();

	if (m_calibrationState->initialPointSampleCount < 3)
	{
		const int sampleCount= m_calibrationState->initialPointSampleCount;

		m_calibrationState->initialPointSamples[sampleCount]= pointSample;
		m_calibrationState->initialPointSampleCount= sampleCount + 1;
	}
	else if (m_calibrationState->triangulatedPointSampleCount < 3)
	{
		const int sampleCount= m_calibrationState->triangulatedPointSampleCount;

		m_calibrationState->triangulatedPointSamples[sampleCount]= m_calibrationState->lastWorldTriangulatedPoint;
		m_calibrationState->triangulatedPointSampleCount= sampleCount + 1;
	}
}

glm::vec2 AnchorTriangulator::computeMouseScreenPosition() const
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

void AnchorTriangulator::sampleCameraPose()
{
	VideoSourceViewPtr videoSource = m_distortionView->getVideoSourceView();
	glm::mat4 glm_camera_xform;
	if (videoSource->getCameraPose(m_cameraTrackingPuckPoseView, glm_camera_xform))
	{
		m_calibrationState->initialCameraPoseSample = glm_camera_xform;
	}
}

void AnchorTriangulator::computeCurrentTriangulation()
{
	const int sampleIndex= m_calibrationState->triangulatedPointSampleCount;
	if (sampleIndex >= 3)
		return;

	// Compute a ray for the initial sample pixel
	glm::vec3 initialPointRayStart;
	glm::vec3 initialPointRayDirection;
	computeCameraRayAtPixel(
		m_calibrationState->initialCameraPoseSample,
		m_calibrationState->initialPointSamples[sampleIndex],
		initialPointRayStart,
		initialPointRayDirection);

	// Compute a ray for triangulating new sample pixel
	VideoSourceViewPtr videoSource = m_distortionView->getVideoSourceView();	
	glm::mat4 triangulatingCameraXform;
	if (videoSource->getCameraPose(m_cameraTrackingPuckPoseView, triangulatingCameraXform))
	{
		const glm::vec2 triangulatingPointSample = computeMouseScreenPosition();
		glm::vec3 triangulatingPointRayStart;
		glm::vec3 triangulatingPointRayDirection;
		computeCameraRayAtPixel(
			triangulatingCameraXform,
			triangulatingPointSample,
			triangulatingPointRayStart,
			triangulatingPointRayDirection);

		// Triangulate the two points by finding the point on the 
		// initial ray closest to the triangulating ray
		float closestTime;
		glm::vec3 closesPoint;
		if (glm_closest_point_on_ray_to_ray(
			initialPointRayStart, initialPointRayDirection,
			triangulatingPointRayStart, triangulatingPointRayDirection,
			closestTime, closesPoint)
			&& closestTime >= 0.f)
		{
			m_calibrationState->lastWorldTriangulatedPoint = closesPoint;
		}
	}
}

bool AnchorTriangulator::computeAnchorTransform(AnchorTriangulatorInfo& anchorInfo)
{
	if (m_calibrationState->triangulatedPointSampleCount < 3)
		return false;

	// Compute the anchor world transform
	const glm::vec3 origin= m_calibrationState->triangulatedPointSamples[0];
	const glm::vec3 xAxis= glm::normalize(m_calibrationState->triangulatedPointSamples[1] - origin);
	const glm::vec3 uncorrectedYAxis = glm::normalize(m_calibrationState->triangulatedPointSamples[2] - origin);
	const glm::vec3 zAxis = glm::normalize(glm::cross(xAxis, uncorrectedYAxis));
	const glm::vec3 yAxis = glm::normalize(glm::cross(zAxis, xAxis));
	
	m_calibrationState->anchorWorldXform= 
		glm::mat4(
			glm::vec4(xAxis, 0.f),
			glm::vec4(yAxis, 0.f),
			glm::vec4(zAxis, 0.f),
			glm::vec4(origin, 1.f));
	anchorInfo.worldTransform= m_calibrationState->anchorWorldXform;

	return true;
}

void AnchorTriangulator::renderInitialPoint2dSegements()
{
	glm::vec3 glm_points[3];
	const int pointCount = m_calibrationState->initialPointSampleCount;

	for (int i = 0; i < pointCount; i++)
	{
		const glm::vec2& imagePoint= m_calibrationState->initialPointSamples[i];

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
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[1], Colors::Red, Colors::Green);
	}

	if (pointCount >= 3)
	{
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[2], Colors::Red, Colors::Blue);
	}

	drawPointList2d(m_frameWidth, m_frameHeight, glm_points, pointCount, Colors::Yellow, 2.f);
}

void AnchorTriangulator::renderInitialPoint3dRays()
{
	const glm::mat4 glmCameraXform = m_calibrationState->initialCameraPoseSample;

	// Draw the frustum for the initial camera pose
	const float hfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.hfov);
	const float vfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.vfov);
	const float zNear = fmaxf(m_calibrationState->inputCameraIntrinsics.znear, 0.1f);
	const float zFar = fminf(m_calibrationState->inputCameraIntrinsics.zfar, 2.0f);
	drawTransformedFrustum(
		glmCameraXform,
		hfov_radians, vfov_radians,
		zNear, zFar,
		Colors::Yellow);
	drawTransformedAxes(glmCameraXform, 0.1f);

	// Draw the rays corresponding to each pixel sample
	for (int i = 0; i < 3; i++)
	{
		// Get the next image point
		const glm::vec2& imagePoint= m_calibrationState->initialPointSamples[i];

		// Compute a ray for each sample pixel
		glm::vec3 rayStart;
		glm::vec3 rayDirection;
		computeCameraRayAtPixel(
			glmCameraXform,
			imagePoint,
			rayStart,
			rayDirection);
		glm::vec3 rayEnd= rayStart + rayDirection*1000.f;

		drawSegment(glm::mat4(1.f), rayStart, rayEnd, k_anchorRayColors[i]);
	}
}

void AnchorTriangulator::renderCurrentPointTriangulation()
{
	if (m_calibrationState->initialPointSampleCount < 3)
		return;

	// Get the next image point
	const int sampleIndex= m_calibrationState->triangulatedPointSampleCount;
	const glm::vec2& imagePoint = m_calibrationState->initialPointSamples[sampleIndex];

	// Draw the ray for the current initial sample point we are trying to triangulate
	glm::vec3 initialPointRayStart;
	glm::vec3 initialPointRayDirection;
	computeCameraRayAtPixel(
		m_calibrationState->initialCameraPoseSample,
		m_calibrationState->initialPointSamples[sampleIndex],
		initialPointRayStart,
		initialPointRayDirection);
	glm::vec3 initialPointRayEnd = initialPointRayStart + initialPointRayDirection * 1000.f;
	drawSegment(glm::mat4(1.f), initialPointRayStart, initialPointRayEnd, k_anchorRayColors[sampleIndex]);
	
	// Draw the most recently computed triangulation
	drawPoint(glm::mat4(1.f), m_calibrationState->lastWorldTriangulatedPoint, Colors::Yellow, 5.f);

	// Draw the label for the current point index
	TextStyle style = getDefaultTextStyle();
	style.horizontalAlignment = eHorizontalTextAlignment::Middle;
	style.verticalAlignment = eVerticalTextAlignment::Bottom;
	drawTextAtCameraPosition(
		style,
		m_frameWidth, m_frameHeight,
		m_calibrationState->lastWorldTriangulatedPoint,
		L"P%d", sampleIndex);
}

void AnchorTriangulator::computeCameraRayAtPixel(
	const glm::mat4 cameraXform,
	const glm::vec2& imagePoint,
	glm::vec3& outRayStart,
	glm::vec3& outRayDirection) const
{
	VideoSourceViewPtr videoSource = m_distortionView->getVideoSourceView();

	const glm::vec3 glmCameraRight = cameraXform[0];
	const glm::vec3 glmCameraUp = cameraXform[1];
	const glm::vec3 glmCameraForward = cameraXform[2] * -1.f; // camera forward is down -z
	const glm::vec3 glmCameraCenter = cameraXform[3];

	float focal_length_x;
	float focal_length_y;
	float principal_point_x;
	float principal_point_y;
	float skew;
	extractCameraIntrinsicMatrixParameters(
		m_calibrationState->inputCameraIntrinsics.undistorted_camera_matrix,
		focal_length_x,
		focal_length_y,
		principal_point_x,
		principal_point_y,
		skew);

	const float local_x = (imagePoint.x - principal_point_x) / focal_length_x;
	const float local_y = (principal_point_y - imagePoint.y) / focal_length_y; // flip y-axis
	
	outRayStart= glmCameraCenter;
	outRayDirection =
		glm::normalize(
			local_x * glmCameraRight
			+ local_y * glmCameraUp
			+ glmCameraForward);
}

void AnchorTriangulator::renderAllTriangulatedPoints(bool bShowCameraFrustum)
{
	const glm::vec3 p0= m_calibrationState->triangulatedPointSamples[0];
	const glm::vec3 p1= m_calibrationState->triangulatedPointSamples[1];
	const glm::vec3 p2= m_calibrationState->triangulatedPointSamples[2];

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
		glm::mat4 glm_camera_xform;
		if (m_distortionView->getVideoSourceView()->getCameraPose(m_cameraTrackingPuckPoseView, glm_camera_xform))
		{
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
}

void AnchorTriangulator::renderAnchorTransform()
{
	drawTransformedAxes(m_calibrationState->anchorWorldXform, 0.1f, 0.1f, 0.1f);
}