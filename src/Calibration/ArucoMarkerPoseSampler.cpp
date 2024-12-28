#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "CameraMath.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "MathGLM.h"
#include "ArucoMarkerPoseSampler.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"
#include "VRDeviceView.h"

#include <algorithm>
#include <atomic>
#include <thread>

struct ArucoMarkerPoseSamplerState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;
	ProfileConfigConstPtr profileConfig;
	int desiredSampleCount;
	bool bApplyVRDeviceOffset;

	// Computed every frame
	glm::dmat4 cameraToMarkerXform;
	glm::dmat4 vrSpaceMarkerXform;
	bool hasValidCapture;

	// Sample State
	int capturedSampleCount;
	std::vector<cv::Quatd> cameraOffsetQuats;
	std::vector<cv::Vec3d> cameraOffsetPositions; // meters

	// Result
	glm::dquat rotationOffset;
	glm::dvec3 translationOffset;

	void init(
		ProfileConfigConstPtr config, 
		VideoSourceViewPtr videoSourceView, 
		int sampleCount,
		bool applyVRDeviceOffset)
	{
		profileConfig= config;

		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

		inputCameraIntrinsics = cameraIntrinsics.getMonoIntrinsics();
		desiredSampleCount = sampleCount;
		bApplyVRDeviceOffset = applyVRDeviceOffset;

		resetCalibration();
	}

	void resetCalibration()
	{
		// Reset the capture state
		cameraToMarkerXform = glm::dmat4(1.0);
		vrSpaceMarkerXform = glm::dmat4(1.0);
		hasValidCapture= false;

		capturedSampleCount = 0;
		cameraOffsetQuats.clear();
		cameraOffsetPositions.clear();

		// Reset the output
		rotationOffset= glm::dquat();
		translationOffset= glm::dvec3(0.0);
	}
};

//-- MonoDistortionCalibrator ----
ArucoMarkerPoseSampler::ArucoMarkerPoseSampler(
	ProfileConfigConstPtr profileConfig,
	VRDeviceViewPtr cameraTrackingPuckView,
	VideoFrameDistortionView* distortionView,
	int desiredSampleCount,
	bool bApplyVRDeviceOffset)
	: m_calibrationState(new ArucoMarkerPoseSamplerState)
	, m_cameraTrackingPuckView(cameraTrackingPuckView)
	, m_distortionView(distortionView)
	, m_markerFinder(new CalibrationPatternFinder_Aruco(
		distortionView,
		profileConfig->vrCenterArucoId,
		profileConfig->vrCenterMarkerLengthMM,
		profileConfig->charucoDictionaryType))
{
	frameWidth = distortionView->getFrameWidth();
	frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(
		profileConfig, 
		distortionView->getVideoSourceView(), 
		desiredSampleCount,
		bApplyVRDeviceOffset);
}

ArucoMarkerPoseSampler::~ArucoMarkerPoseSampler()
{
	delete m_markerFinder;
	delete m_calibrationState;
}

bool ArucoMarkerPoseSampler::hasFinishedSampling() const
{
	return m_calibrationState->capturedSampleCount >= m_calibrationState->desiredSampleCount;
}

float ArucoMarkerPoseSampler::getCalibrationProgress() const
{
	const float samplePercentage =
		(float)m_calibrationState->capturedSampleCount
		/ (float)m_calibrationState->desiredSampleCount;

	return samplePercentage;
}

void ArucoMarkerPoseSampler::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

bool ArucoMarkerPoseSampler::computeVRSpaceMarkerXform()
{
	// Gather inputs
	//---------------

	// Mark the last capture as invalid
	m_calibrationState->hasValidCapture= false;

	// Get tracking puck poses
	if (!m_cameraTrackingPuckView->getIsPoseValid())
	{
		return false;
	}

	// Compute the camera pose in VRSpace
	const glm::dmat4 cameraXform_VRSpace=
		m_distortionView->getVideoSourceView()->getCameraPose(
			m_cameraTrackingPuckView, m_calibrationState->bApplyVRDeviceOffset);

	// Look for the calibration pattern in the latest video frame
	glm::dmat4 cameraToPatternXform;
	if (!m_markerFinder->estimateNewCalibrationPatternPose(cameraToPatternXform))
	{
		return false;
	}

	// Compute the marker transform in VR tracking space
	const glm::dmat4 patternXform_VRSpace = glm_composite_xform(cameraToPatternXform, cameraXform_VRSpace);

	// Save the the last computed transform to the calibration state
	m_calibrationState->cameraToMarkerXform= cameraToPatternXform;
	m_calibrationState->vrSpaceMarkerXform= patternXform_VRSpace;
	m_calibrationState->hasValidCapture= true;

	return true;
}

void ArucoMarkerPoseSampler::sampleLastVRSpaceMarkerXform()
{
	if (!m_calibrationState->hasValidCapture)
		return;

	// Extract the rotation and translation offsets from the vrSpaceMarkerXform transform
	glm::dvec3 glm_translationOffset = m_calibrationState->vrSpaceMarkerXform[3];
	glm::dquat glm_rotationOffset = glm::quat_cast(m_calibrationState->vrSpaceMarkerXform);

	// Need to store this as OpenCV types for averaging operation in computeCalibratedMarkerPose
	cv::Vec3d cv_translationOffset = glm_dvec3_to_cv_vec3d(glm_translationOffset);
	cv::Quatd cv_rotationOffset = glm_dquat_to_cv_quatd(glm_rotationOffset);

	m_calibrationState->cameraOffsetQuats.push_back(cv_rotationOffset);
	m_calibrationState->cameraOffsetPositions.push_back(cv_translationOffset);
	m_calibrationState->capturedSampleCount++;
}

bool ArucoMarkerPoseSampler::computeCalibratedMarkerPose(
	MikanQuatd& outRotation, 
	MikanVector3d& outTranslation)
{
	cv::Vec3d cv_cameraOffsetPosition;
	cv::Quatd cv_cameraOffsetQuat;

	if (hasFinishedSampling() &&
		opencv_quaternion_compute_average(m_calibrationState->cameraOffsetQuats, cv_cameraOffsetQuat) &&
		opencv_vec3d_compute_average(m_calibrationState->cameraOffsetPositions, cv_cameraOffsetPosition))
	{
		outRotation= cv_quatd_to_MikanQuatd(cv_cameraOffsetQuat);
		outTranslation= cv_vec3d_to_MikanVector3d(cv_cameraOffsetPosition);

		return true;
	}

	return false;
}

void ArucoMarkerPoseSampler::renderCameraSpaceCalibrationState()
{
	// Draw the most recently capture chessboard in camera space
	m_markerFinder->renderCalibrationPattern2D();

	// Draw the camera relative transforms of the pattern (computed from solvePnP)
	// and the mat puck location offset from the pattern origin
	if (m_calibrationState->hasValidCapture)
	{
		const glm::mat4 cameraToMarkerXform = glm::mat4(m_calibrationState->cameraToMarkerXform);

		drawTransformedAxes(cameraToMarkerXform, 0.1f);

		TextStyle style = getDefaultTextStyle();
		drawTextAtWorldPosition(style, glm_mat4_get_position(cameraToMarkerXform), L"Marker");
	}
}

void ArucoMarkerPoseSampler::renderVRSpaceCalibrationState()
{
	// Compute the camera pose in VRSpace
	const glm::dmat4 cameraXform_VRSpace =
		m_distortionView->getVideoSourceView()->getCameraPose(
			m_cameraTrackingPuckView,
			m_calibrationState->bApplyVRDeviceOffset);

	// Draw the marker transform
	const glm::mat4 markerXform = glm::mat4(m_calibrationState->vrSpaceMarkerXform);
	drawTransformedAxes(markerXform, 0.1f);

	// Draw the most recently derived camera transform derived from the mat puck
	const float hfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.hfov);
	const float vfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.vfov);
	const float zNear= fmaxf(m_calibrationState->inputCameraIntrinsics.znear, 0.1f);
	const float zFar = fminf(m_calibrationState->inputCameraIntrinsics.zfar, 2.0f);
	drawTransformedFrustum(
		cameraXform_VRSpace,
		hfov_radians, vfov_radians,
		zNear, zFar,
		Colors::Yellow);
	drawTransformedAxes(cameraXform_VRSpace, 0.1f);
}