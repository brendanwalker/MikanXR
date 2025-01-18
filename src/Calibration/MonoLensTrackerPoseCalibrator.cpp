#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "CameraMath.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "MathGLM.h"
#include "MonoLensTrackerPoseCalibrator.h"
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

struct MonoLensTrackerCalibrationState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;
	ProfileConfigConstPtr profileConfig;
	int desiredSampleCount;

	// Computed every frame
	glm::dmat4 patternXform_VRSpace;
	glm::dmat4 cameraXform_VRSpace;
	glm::dmat4 cameraToPatternXform;
	glm::dmat4 cameraToMatPuckXform;
	glm::dmat4 cameraToCameraPuckXform;
	glm::dmat4 patternToMatPuckXform;
	bool hasValidCapture;

	// Sample State
	int capturedSampleCount;
	std::vector<cv::Quatd> cameraOffsetQuats;
	std::vector<cv::Vec3d> cameraOffsetPositions; // meters

	// Result
	glm::dquat rotationOffset;
	glm::dvec3 translationOffset;

	void init(ProfileConfigConstPtr config, VideoSourceViewPtr videoSourceView, int patternCount)
	{
		profileConfig= config;

		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

		inputCameraIntrinsics = cameraIntrinsics.getMonoIntrinsics();
		desiredSampleCount = patternCount;

		resetCalibration();
	}

	void resetCalibration()
	{
		// Reset the capture state
		patternXform_VRSpace = glm::dmat4(1.0);
		cameraXform_VRSpace = glm::dmat4(1.0);
		cameraToPatternXform = glm::dmat4(1.0);
		cameraToCameraPuckXform = glm::dmat4(1.0);
		patternToMatPuckXform = glm::dmat4(1.0);
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
MonoLensTrackerPoseCalibrator::MonoLensTrackerPoseCalibrator(
	ProfileConfigConstPtr profileConfig,
	VRDeviceViewPtr cameraTrackingPuckView,
	VRDeviceViewPtr matTrackingPuckView,
	VideoFrameDistortionView* distortionView,
	int desiredSampleCount)
	: m_calibrationState(new MonoLensTrackerCalibrationState)
	, m_cameraTrackingPuckView(cameraTrackingPuckView)
	, m_matTrackingPuckView(matTrackingPuckView)
	, m_distortionView(distortionView)
	, m_patternFinder(CalibrationPatternFinder::allocatePatternFinder(profileConfig, distortionView))
{
	frameWidth = distortionView->getFrameWidth();
	frameHeight = distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(profileConfig, distortionView->getVideoSourceView(), desiredSampleCount);
}

MonoLensTrackerPoseCalibrator::~MonoLensTrackerPoseCalibrator()
{
	delete m_patternFinder;
	delete m_calibrationState;
}

bool MonoLensTrackerPoseCalibrator::hasFinishedSampling() const
{
	return m_calibrationState->capturedSampleCount >= m_calibrationState->desiredSampleCount;
}

float MonoLensTrackerPoseCalibrator::getCalibrationProgress() const
{
	const float samplePercentage =
		(float)m_calibrationState->capturedSampleCount
		/ (float)m_calibrationState->desiredSampleCount;

	return samplePercentage;
}

void MonoLensTrackerPoseCalibrator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

bool MonoLensTrackerPoseCalibrator::computeCameraToPuckXform()
{
	// Gather inputs
	//---------------

	// Mark the last capture as invalid
	m_calibrationState->hasValidCapture= false;

	// Get tracking puck poses
	if (!m_cameraTrackingPuckView->getIsPoseValid() || !m_matTrackingPuckView->getIsPoseValid())
	{
		return false;
	}

	// Fetch the calibration poses from the devices
	const glm::dmat4 cameraPuckXform_VRSpace= glm::dmat4(m_cameraTrackingPuckView->getDefaultComponentPose());
	const glm::dmat4 matPuckXform_VRSpace = glm::dmat4(m_matTrackingPuckView->getDefaultComponentPose());

	// Look for the calibration pattern in the latest video frame
	glm::dmat4 cameraToPatternXform;
	if (!m_patternFinder->estimateNewCalibrationPatternPose(cameraToPatternXform))
	{
		return false;
	}

	// Compute the VR tracking space location of the camera.
	// We start at the tracking puck on the mat and apply offsets to get to the camera.
	//---------------------------------------

	// Compute the VR tracking space offset from matPuck to calibration pattern
	// using the measured offsets on the paper calibration mat
	ProfileConfigConstPtr config= m_calibrationState->profileConfig;
	const double horizOffset = (double)config->puckHorizontalOffsetMM * k_millimeters_to_meters;
	const double vertOffset = (double)config->puckVerticalOffsetMM * k_millimeters_to_meters;
	const double depthOffset = (double)config->puckDepthOffsetMM * k_millimeters_to_meters;
	const glm::dmat4 puckYawRot90 = 
		glm::rotate(
			glm::dmat4(1.f), 
			k_real64_half_pi, 
			glm::dvec3(0.0, 1.f, 0.f));
	const glm::dmat4 translateToPatternXform =
		glm::translate(
			glm::dmat4(1.0),
			glm::dvec3(horizOffset, depthOffset, vertOffset));
	const glm::dmat4 matPuckToPatternXform = glm_composite_xform(puckYawRot90, translateToPatternXform);

	// Compute the transform from the camera to the mat puck
	const glm::dmat4 patternToMatPuckXform = glm::inverse(matPuckToPatternXform);
	const glm::dmat4 cameraToMatPuckXform= glm_composite_xform(patternToMatPuckXform, cameraToPatternXform);

	// Compute the pattern transform in VR tracking space
	const glm::dmat4 patternXform_VRSpace = glm_composite_xform(matPuckToPatternXform, matPuckXform_VRSpace);

	// Then compute the camera transform in VR tracking space
	// by applying the inverse of the cameraToPatternXform 
	// computed optically by OpenCV
	const glm::dmat4 patternToCameraXform = glm::inverse(cameraToPatternXform);
	const glm::dmat4 cameraXform_VRSpace = glm_composite_xform(patternToCameraXform, patternXform_VRSpace);

	// Finally compute the transform to go from the camera tracking puck to the camera 
	// (i.e. the relative offset of the camera from camera tracking puck)
	const glm::dmat4 invCameraPuckXform = glm::inverse(cameraPuckXform_VRSpace);
	const glm::dmat4 cameraToCameraPuckXform = glm_composite_xform(cameraXform_VRSpace, invCameraPuckXform);

	// Save the the last computed transform to the calibration state
	m_calibrationState->patternXform_VRSpace = patternXform_VRSpace;
	m_calibrationState->cameraXform_VRSpace = cameraXform_VRSpace;
	m_calibrationState->cameraToPatternXform= cameraToPatternXform;
	m_calibrationState->cameraToMatPuckXform= cameraToMatPuckXform;
	m_calibrationState->cameraToCameraPuckXform = cameraToCameraPuckXform;
	m_calibrationState->patternToMatPuckXform = glm::inverse(matPuckToPatternXform);
	m_calibrationState->hasValidCapture= true;

	return true;
}

glm::mat4 MonoLensTrackerPoseCalibrator::getLastCameraPose(VRDeviceViewPtr attachedVRDevicePtr) const
{
	const glm::mat4 cameraOffsetXform = glm::mat4(m_calibrationState->cameraToCameraPuckXform);
	const glm::mat4 vrDevicePose = attachedVRDevicePtr->getDefaultComponentPose();
	const glm::mat4 cameraPose = vrDevicePose * cameraOffsetXform;

	return cameraPose;
}

void MonoLensTrackerPoseCalibrator::sampleLastCameraToPuckXform()
{
	if (!m_calibrationState->hasValidCapture)
		return;

	// Extract the rotation and translation offsets from the cameraToCameraPuckXform transform
	glm::dvec3 glm_translationOffset = m_calibrationState->cameraToCameraPuckXform[3];
	glm::dquat glm_rotationOffset = glm::quat_cast(m_calibrationState->cameraToCameraPuckXform);

	// Need to store this as OpenCV types for averaging operation in computeCalibratedCameraTrackerOffset
	cv::Vec3d cv_translationOffset = glm_dvec3_to_cv_vec3d(glm_translationOffset);
	cv::Quatd cv_rotationOffset = glm_dquat_to_cv_quatd(glm_rotationOffset);

	m_calibrationState->cameraOffsetQuats.push_back(cv_rotationOffset);
	m_calibrationState->cameraOffsetPositions.push_back(cv_translationOffset);
	m_calibrationState->capturedSampleCount++;
}

bool MonoLensTrackerPoseCalibrator::computeCalibratedCameraTrackerOffset(
	MikanQuatd& outRotationOffset, 
	MikanVector3d& outTranslationOffset)
{
	cv::Vec3d cv_cameraOffsetPosition;
	cv::Quatd cv_cameraOffsetQuat;

	if (hasFinishedSampling() &&
		opencv_quaternion_compute_average(m_calibrationState->cameraOffsetQuats, cv_cameraOffsetQuat) &&
		opencv_vec3d_compute_average(m_calibrationState->cameraOffsetPositions, cv_cameraOffsetPosition))
	{
		outRotationOffset= cv_quatd_to_MikanQuatd(cv_cameraOffsetQuat);
		outTranslationOffset= cv_vec3d_to_MikanVector3d(cv_cameraOffsetPosition);

		return true;
	}

	return false;
}

void MonoLensTrackerPoseCalibrator::renderCameraSpaceCalibrationState()
{
	// Draw the most recently capture chessboard in camera space
	m_patternFinder->renderCalibrationPattern2D();

	// Draw the camera relative transforms of the pattern (computed from solvePnP)
	// and the mat puck location offset from the pattern origin
	if (m_calibrationState->hasValidCapture)
	{
		const glm::mat4 cameraToPatternXform = glm::mat4(m_calibrationState->cameraToPatternXform);
		const glm::mat4 cameraToMatPuckXform = glm::mat4(m_calibrationState->cameraToMatPuckXform);

		m_patternFinder->renderSolvePnPPattern3D(cameraToPatternXform);

		drawTransformedAxes(cameraToPatternXform, 0.1f);
		drawTransformedAxes(cameraToMatPuckXform, 0.1f);

		TextStyle style = getDefaultTextStyle();
		drawTextAtWorldPosition(style, glm_mat4_get_position(cameraToPatternXform), L"Mat");
		drawTextAtWorldPosition(style, glm_mat4_get_position(cameraToMatPuckXform), L"Puck");
	}
}

void MonoLensTrackerPoseCalibrator::renderVRSpaceCalibrationState()
{
	// Draw the most recently captured chessboard projected into VR
	m_patternFinder->renderSolvePnPPattern3D(m_calibrationState->patternXform_VRSpace);

	// Draw the camera puck transform
	const glm::mat4 cameraPuckXform = glm::dmat4(m_cameraTrackingPuckView->getDefaultComponentPose());
	drawTransformedAxes(cameraPuckXform, 0.1f);

	// Draw the mat puck transform
	const glm::mat4 matPuckXform = glm::dmat4(m_matTrackingPuckView->getDefaultComponentPose());
	drawTransformedAxes(matPuckXform, 0.1f);

	// Draw the most recently derived camera transform derived from the mat puck
	const float hfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.hfov);
	const float vfov_radians = degrees_to_radians(m_calibrationState->inputCameraIntrinsics.vfov);
	const float zNear= fmaxf(m_calibrationState->inputCameraIntrinsics.znear, 0.1f);
	const float zFar = fminf(m_calibrationState->inputCameraIntrinsics.zfar, 2.0f);
	drawTransformedFrustum(
		m_calibrationState->cameraXform_VRSpace,
		hfov_radians, vfov_radians,
		zNear, zFar,
		Colors::Yellow);
	drawTransformedAxes(m_calibrationState->cameraXform_VRSpace, 0.1f);
}