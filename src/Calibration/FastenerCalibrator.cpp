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

typedef std::vector<cv::Vec2d> cvFastenerPointArray;

struct FastenerCalibrationState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;

	// Sample State
	cvFastenerPointArray pointSamples[2];
	cv::Matx34f pinholeMatrixSamples[2];
	int sampledCameraPoseCount;

	// Result
	glm::vec3 fastenerPoints[3];

	void init(VideoSourceViewPtr videoSourceView)
	{
		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

		resetCalibration();
	}

	void resetCalibration()
	{
		pointSamples[0].clear();
		pointSamples[1].clear();
		pinholeMatrixSamples[0]= cv::Matx34f();
		pinholeMatrixSamples[1]= cv::Matx34f();
		sampledCameraPoseCount= 0;
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

bool FastenerCalibrator::hasFinishedSampling() const
{
	return 
		m_calibrationState->sampledCameraPoseCount >= 2 || 
		m_calibrationState->pointSamples[m_calibrationState->sampledCameraPoseCount].size() >= 3;
}

void FastenerCalibrator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

void FastenerCalibrator::sampleMouseScreenPosition()
{
	int mouseScreenX = 0, mouseScreenY;
	InputManager::getInstance()->getMouseScreenPosition(mouseScreenX, mouseScreenY);

	Renderer* renderer = Renderer::getInstance();
	const double screenWidth = renderer->getSDLWindowWidth();
	const double screenHeight = renderer->getSDLWindowHeight();

	cv::Vec2d cameraSample(
		((double)mouseScreenX * (double)m_frameWidth) / screenWidth,
		((double)mouseScreenY * (double)m_frameHeight) / screenHeight);

	if (m_calibrationState->sampledCameraPoseCount < 2)
	{
		const int triIndex= m_calibrationState->sampledCameraPoseCount;
		cvFastenerPointArray& fastenerPoints = m_calibrationState->pointSamples[triIndex];

		if (fastenerPoints.size() < 3)
		{
			fastenerPoints.push_back(cameraSample);
		}
	}
}

void FastenerCalibrator::sampleCameraPose()
{
	cv::Matx34f extrinsic_matrix;
	VideoSourceViewPtr videoSource = m_distortionView->getVideoSourceView();
	computeOpenCVCameraExtrinsicMatrix(videoSource, m_cameraTrackingPuckView, extrinsic_matrix);

	cv::Matx33f intrinsic_matrix;
	computeOpenCVCameraIntrinsicMatrix(
		videoSource,
		VideoFrameSection::Primary,
		intrinsic_matrix);

	if (m_calibrationState->sampledCameraPoseCount < 2)
	{
		const int triIndex= m_calibrationState->sampledCameraPoseCount;

		// Compute the pinhole camera matrix for each tracker that allows you to raycast
		// from the tracker center in world space through the screen location, into the world
		// See: http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html
		m_calibrationState->pinholeMatrixSamples[triIndex] = intrinsic_matrix * extrinsic_matrix;
		m_calibrationState->sampledCameraPoseCount++;
	}
}

bool FastenerCalibrator::computeFastenerPoints(MikanSpatialFastenerInfo* fastener)
{
	if (m_calibrationState->sampledCameraPoseCount >= 2)
		return false;

	// Triangulate the world position from the two cameras
	const int kNumPoints= 3;
	cv::Mat resultPoint3D(1, kNumPoints, CV_32FC4);
	cv::triangulatePoints(
		m_calibrationState->pinholeMatrixSamples[0],
		m_calibrationState->pinholeMatrixSamples[1], 
		m_calibrationState->pointSamples[0], 
		m_calibrationState->pointSamples[1], 
		resultPoint3D);

	// Compute the fastener world to local transform
	const glm::mat4 localToWorldXform = m_profileConfig->getFastenerWorldTransform(fastener);
	const glm::mat4 worldToLocalXform = glm::inverse(localToWorldXform);

	// Convert triangulated world space points into local space
	for (int index = 0; index < kNumPoints; ++index)
	{
		const float w = resultPoint3D.at<float>(3, index);
		const glm::vec3 worldPoint(
			resultPoint3D.at<float>(0, index) / w,
			resultPoint3D.at<float>(1, index) / w,
			resultPoint3D.at<float>(2, index) / w);
		const glm::vec3 localPoint = worldToLocalXform * glm::vec4(worldPoint, 1.f);

		m_calibrationState->fastenerPoints[index]= worldPoint;
		fastener->fastener_points[index] = glm_vec3_to_MikanVector3f(localPoint);
	}

	return true;
}

void FastenerCalibrator::renderCameraSpaceCalibrationState()
{
	const int currentTriangleIndex= (int)m_calibrationState->pointSamples->size() - 1;
	if (currentTriangleIndex < 0)
		return;

	const cvFastenerPointArray& pointArray= m_calibrationState->pointSamples[currentTriangleIndex];

	glm::vec3 glm_points[3];
	const int pointCount = (int)pointArray.size();

	for (int i = 0; i < pointCount; i++)
	{
		const cv::Vec2d& cv_point= pointArray[i];

		glm_points[i]= glm::vec3(cv_point(0), cv_point(1), 0.5f);

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
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[1], Colors::Red);
	}

	if (pointArray.size() >= 3)
	{
		drawSegment2d(m_frameWidth, m_frameHeight, glm_points[0], glm_points[2], Colors::Green);
	}

	drawPointList2d(m_frameWidth, m_frameHeight, glm_points, pointCount, Colors::Yellow, 2.f);
}

void FastenerCalibrator::renderVRSpaceCalibrationState()
{
	const glm::vec3 p0= m_calibrationState->fastenerPoints[0];
	const glm::vec3 p1= m_calibrationState->fastenerPoints[1];
	const glm::vec3 p2= m_calibrationState->fastenerPoints[2];

	drawSegment(glm::mat4(1.f), p0, p1, Colors::Red);
	drawSegment(glm::mat4(1.f), p0, p2, Colors::Green);

	TextStyle style = getDefaultTextStyle();
	const int kNumPoints= 3;
	for (int i = 0; i < kNumPoints; i++)
	{
		drawTextAtWorldPosition(style, m_calibrationState->fastenerPoints[i], L"P%d", i);
	}

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