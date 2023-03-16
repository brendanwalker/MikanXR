#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "CameraMath.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "InputManager.h"
#include "FastenerCalibrator.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "MikanClientTypes.h"
#include "Renderer.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"
#include "VRDeviceView.h"

#include <algorithm>
#include <atomic>
#include <thread>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

static const glm::vec3 k_fastenerRayColors[3] = {Colors::Red, Colors::Green, Colors::Blue};

struct FastenerCalibrationState
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
		initialPointSamples[0]= glm::vec2();
		initialPointSamples[1]= glm::vec2();
		initialPointSamples[2]= glm::vec2();
		initialCameraPoseSample = glm::mat4(1.f);
		initialPointSampleCount= 0;
		lastWorldTriangulatedPoint= glm::vec3();
		triangulatedPointSamples[0] = glm::vec3();
		triangulatedPointSamples[1] = glm::vec3();
		triangulatedPointSamples[2] = glm::vec3();
		triangulatedPointSampleCount= 0;
	}
};

//-- MonoDistortionCalibrator ----
FastenerCalibrator::FastenerCalibrator(
	const ProfileConfig* profileConfig,
	VRDeviceViewPtr cameraTrackingPuckView,
	VideoFrameDistortionView* distortionView)
	: m_profileConfig(profileConfig)
	, m_calibrationState(new FastenerCalibrationState)
	, m_cameraTrackingPuckView(cameraTrackingPuckView)
	, m_distortionView(distortionView)
{
	m_frameWidth = distortionView->getFrameWidth();
	m_frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(distortionView->getVideoSourceView());
}

FastenerCalibrator::~FastenerCalibrator()
{
	delete m_calibrationState;
}

bool FastenerCalibrator::hasFinishedInitialPointSampling() const
{
	return m_calibrationState->initialPointSampleCount >= 3;
}

bool FastenerCalibrator::hasFinishedTriangulatedPointSampling() const
{
	return m_calibrationState->triangulatedPointSampleCount >= 3;
}

void FastenerCalibrator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

void FastenerCalibrator::sampleMouseScreenPosition()
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

glm::vec2 FastenerCalibrator::computeMouseScreenPosition() const
{
	int mouseScreenX = 0, mouseScreenY;
	InputManager::getInstance()->getMouseScreenPosition(mouseScreenX, mouseScreenY);

	Renderer* renderer = Renderer::getInstance();
	const float screenWidth = renderer->getSDLWindowWidth();
	const float screenHeight = renderer->getSDLWindowHeight();

	glm::vec2 pointSample(
		((float)mouseScreenX * m_frameWidth) / screenWidth,
		((float)mouseScreenY * m_frameHeight) / screenHeight);

	return pointSample;
}

void FastenerCalibrator::sampleCameraPose()
{
	VideoSourceViewPtr videoSource = m_distortionView->getVideoSourceView();
	const glm::mat4 glm_camera_xform = videoSource->getCameraPose(m_cameraTrackingPuckView);

	m_calibrationState->initialCameraPoseSample= glm_camera_xform;
}

void FastenerCalibrator::computeCurrentTriangulation()
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
	const glm::mat4 triangulatingCameraXform = videoSource->getCameraPose(m_cameraTrackingPuckView);
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
	m_calibrationState->lastWorldTriangulatedPoint=
		glm_closest_point_between_rays(
			triangulatingPointRayStart,
			triangulatingPointRayDirection,
			initialPointRayStart,
			initialPointRayDirection);
}

bool FastenerCalibrator::computeFastenerPoints(MikanSpatialFastenerInfo* fastener)
{
	if (m_calibrationState->triangulatedPointSampleCount < 3)
		return false;

	// Compute the fastener world to local transform
	const glm::mat4 localToWorldXform = m_profileConfig->getFastenerWorldTransform(fastener);
	const glm::mat4 worldToLocalXform = glm::inverse(localToWorldXform);

	// Convert triangulated world space points into local space
	for (int index = 0; index < 3; ++index)
	{
		const glm::vec3 worldPoint = m_calibrationState->triangulatedPointSamples[index];
		const glm::vec3 localPoint = worldToLocalXform * glm::vec4(worldPoint, 1.f);

		fastener->fastener_points[index] = glm_vec3_to_MikanVector3f(localPoint);
	}

	return true;
}

void FastenerCalibrator::renderInitialPoint2dSegements()
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

void FastenerCalibrator::renderInitialPoint3dRays()
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

		drawSegment(glm::mat4(1.f), rayStart, rayEnd, k_fastenerRayColors[i]);
	}
}

void FastenerCalibrator::renderCurrentPointTriangulation()
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
	drawSegment(glm::mat4(1.f), initialPointRayStart, initialPointRayEnd, k_fastenerRayColors[sampleIndex]);
	
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

void FastenerCalibrator::computeCameraRayAtPixel(
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
	extractCameraIntrinsicMatrixParameters(
		m_calibrationState->inputCameraIntrinsics.camera_matrix,
		focal_length_x,
		focal_length_y,
		principal_point_x,
		principal_point_y);

	const float local_x = (imagePoint.x - principal_point_x) / focal_length_x;
	const float local_y = (principal_point_y - imagePoint.y) / focal_length_y; // flip y-axis
	
	outRayStart= glmCameraCenter;
	outRayDirection =
		glm::normalize(
			local_x * glmCameraRight
			+ local_y * glmCameraUp
			+ glmCameraForward);
}

void FastenerCalibrator::renderAllTriangulatedPoints(bool bShowCameraFrustum)
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