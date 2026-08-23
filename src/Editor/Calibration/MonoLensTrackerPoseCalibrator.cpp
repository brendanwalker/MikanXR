#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder_Charuco.h"
#include "CameraComponent.h"
#include "CameraMath.h"
#include "Colors.h"
#include "IEditorWindow.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MathGLM.h"
#include "MonoLensTrackerPoseCalibrator.h"
#include "MarkerObjectSystem.h"
#include "MathTypeConversion.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "ProjectManager.h"
#include "StageComponent.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"
#include "VRDeviceComponent.h"
#include "VRTrackingVolumeComponent.h"

#include <algorithm>
#include <atomic>
#include <thread>

struct MonoLensTrackerCalibrationState
{
	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;
	int desiredSampleCount;

	// Computed every frame
	CameraPuckToApertureResults cameraPuckToApertureResults;

	// Sample State
	int capturedSampleCount;
	std::vector<cv::Quatd> cv_apertureOffsetQuats;
	std::vector<cv::Vec3d> cv_apertureOffsetPositions; // meters

	// Result
	glm::dquat rotationOffset;
	glm::dvec3 translationOffset;

	void init(CameraComponentPtr cameraComponent, int patternCount)
	{
		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		cameraComponent->getApertureIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS);

		inputCameraIntrinsics= cameraIntrinsics.getMonoIntrinsics();
		desiredSampleCount= patternCount;

		resetCalibration();
	}

	void init(const MikanMonoIntrinsics& intrinsics, int patternCount)
	{
		inputCameraIntrinsics= intrinsics;
		desiredSampleCount= patternCount;

		resetCalibration();
	}

	void resetCalibration()
	{
		// Reset the capture state
		cameraPuckToApertureResults.reset();
		capturedSampleCount= 0;
		cv_apertureOffsetQuats.clear();
		cv_apertureOffsetPositions.clear();

		capturedSampleCount= 0;
		cv_apertureOffsetQuats.clear();
		cv_apertureOffsetPositions.clear();

		// Reset the output
		rotationOffset= glm::dquat();
		translationOffset= glm::dvec3(0.0);
	}
};

//-- MonoDistortionCalibrator ----
MonoLensTrackerPoseCalibrator::MonoLensTrackerPoseCalibrator(CameraComponentPtr cameraComponent,
															 VRDevicePoseViewPtr cameraPuckPoseView,
															 VRDevicePoseViewPtr matTrackingPuckPoseView,
															 VideoFrameDistortionView* distortionView,
															 int desiredSampleCount)
	: m_calibrationState(new MonoLensTrackerCalibrationState)
	, m_cameraComponent(cameraComponent)
	, m_cameraPuckPose_VRSystemSpace(cameraPuckPoseView)
	, m_matPuckPose_VRSystemSpace(matTrackingPuckPoseView)
	, m_distortionView(distortionView)
{
	StageComponentConstPtr ownerStage= cameraComponent->getOwnerStageComponent();
	assert(ownerStage != nullptr);
	IEditorWindow* ownerWindow= ownerStage->getOwnerEditorWindow();
	assert(ownerWindow != nullptr);
	MarkerObjectSystemPtr markerSystem= ownerWindow->getProjectManager()->getSystemOfType<MarkerObjectSystem>();

	auto markerConfig= markerSystem->getTypedDefinitionConst();
	m_patternFinder= new CalibrationPatternFinder_Charuco(
		distortionView, markerConfig->getCharucoRows(), markerConfig->getCharucoCols(),
		markerConfig->getCharucoSquareLengthMM(), markerConfig->getCharucoMarkerLengthMM(),
		markerConfig->getCharucoDictionaryType());

	frameWidth= distortionView->getFrameWidth();
	frameHeight= distortionView->getFrameHeight();

	// Private calibration state
	m_calibrationState->init(cameraComponent, desiredSampleCount);
}

MonoLensTrackerPoseCalibrator::MonoLensTrackerPoseCalibrator(CalibrationPatternFinder* patternFinder,
															 const MikanMonoIntrinsics& cameraIntrinsics,
															 int desiredSampleCount)
	: m_calibrationState(new MonoLensTrackerCalibrationState)
	, m_cameraComponent(nullptr)
	, m_cameraPuckPose_VRSystemSpace(nullptr)
	, m_matPuckPose_VRSystemSpace(nullptr)
	, m_distortionView(nullptr)
	, m_patternFinder(patternFinder)
{
	frameWidth= patternFinder->getFrameWidth();
	frameHeight= patternFinder->getFrameHeight();

	m_calibrationState->init(cameraIntrinsics, desiredSampleCount);
}

bool MonoLensTrackerPoseCalibrator::fetchCameraPuckVRSpacePose(glm::dmat4& outPose) const
{
	assert(m_cameraPuckPose_VRSystemSpace->getPoseSpace() == eVRDevicePoseSpace::VRTrackingSystemPose);
	return m_cameraPuckPose_VRSystemSpace->getPose(m_cameraComponent, outPose);
}

bool MonoLensTrackerPoseCalibrator::fetchMatPuckVRSpacePose(glm::dmat4& outPose) const
{
	assert(m_matPuckPose_VRSystemSpace->getPoseSpace() == eVRDevicePoseSpace::VRTrackingSystemPose);
	return m_matPuckPose_VRSystemSpace->getPose(m_cameraComponent, outPose);
}

glm::dvec3 MonoLensTrackerPoseCalibrator::fetchMatPuckOffsetMM() const
{
	VRTrackingVolumeDefinitionConstPtr vrTrackingConfig= m_cameraComponent->getVRTrackingVolumeDefinition();
	const MikanVector3f off= vrTrackingConfig->getCharucoMountOffsetMM();
	return glm::dvec3(off.x, off.y, off.z);
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
	const float samplePercentage=
		(float)m_calibrationState->capturedSampleCount / (float)m_calibrationState->desiredSampleCount;

	return samplePercentage;
}

void MonoLensTrackerPoseCalibrator::resetCalibrationState() { m_calibrationState->resetCalibration(); }

bool MonoLensTrackerPoseCalibrator::computeCalibrationSample()
{
	// Mark the last capture as invalid
	m_calibrationState->cameraPuckToApertureResults.reset();

	// Fetch the calibration poses from the devices
	glm::dmat4 cameraPuckXform_VRSpace;
	if (!fetchCameraPuckVRSpacePose(cameraPuckXform_VRSpace))
	{
		return false;
	}
	glm::dmat4 matPuckXform_VRSpace;
	if (!fetchMatPuckVRSpacePose(matPuckXform_VRSpace))
	{
		return false;
	}

	// Look for the calibration pattern in the latest video frame
	glm::dmat4 apertureToPatternXform;
	if (!m_patternFinder->estimateNewCalibrationPatternPose(apertureToPatternXform))
	{
		return false;
	}

	// Get the physical offset from the mat puck to the calibration pattern center (in mm)
	const glm::dvec3 puckOffsetMM= fetchMatPuckOffsetMM();

	// Compute the camera-puck-to-aperture offset
	computeCameraPuckToApertureXform(cameraPuckXform_VRSpace, matPuckXform_VRSpace, apertureToPatternXform,
									 puckOffsetMM, m_calibrationState->cameraPuckToApertureResults);

	return true;
}

bool MonoLensTrackerPoseCalibrator::hasValidCameraPuckToApertureXform() const
{
	return m_calibrationState->cameraPuckToApertureResults.bIsValid;
}

bool MonoLensTrackerPoseCalibrator::getLastCameraPuckToApertureXform(glm::mat4& outCameraToCameraPuckXform) const
{
	if (m_calibrationState->cameraPuckToApertureResults.bIsValid)
	{
		outCameraToCameraPuckXform=
			glm::mat4(m_calibrationState->cameraPuckToApertureResults.cameraPuckToApertureXform);
		return true;
	}

	return false;
}

bool MonoLensTrackerPoseCalibrator::getLastCameraPuckToApertureResults(CameraPuckToApertureResults& outResults) const
{
	if (m_calibrationState->cameraPuckToApertureResults.bIsValid)
	{
		outResults= m_calibrationState->cameraPuckToApertureResults;
		return true;
	}
	return false;
}

void MonoLensTrackerPoseCalibrator::recordCalibrationSample()
{
	const CameraPuckToApertureResults& results= m_calibrationState->cameraPuckToApertureResults;

	if (!results.bIsValid)
		return;

	// Extract the rotation and translation offsets from the cameraPuckToApertureXform transform
	glm::dvec3 glm_translationOffset= results.cameraPuckToApertureXform[3];
	glm::dquat glm_rotationOffset= glm::quat_cast(results.cameraPuckToApertureXform);

	// Need to store this as OpenCV types for averaging operation in computeAverageCameraPuckToApertureOffset
	cv::Vec3d cv_translationOffset= glm_dvec3_to_cv_vec3d(glm_translationOffset);
	cv::Quatd cv_rotationOffset= glm_dquat_to_cv_quatd(glm_rotationOffset);

	m_calibrationState->cv_apertureOffsetQuats.push_back(cv_rotationOffset);
	m_calibrationState->cv_apertureOffsetPositions.push_back(cv_translationOffset);
	m_calibrationState->capturedSampleCount++;
}

bool MonoLensTrackerPoseCalibrator::computeAverageCameraPuckToApertureOffset(MikanQuatd& outRotationOffset,
																			 MikanVector3d& outTranslationOffset)
{
	cv::Vec3d cv_cameraOffsetPosition;
	cv::Quatd cv_cameraOffsetQuat;

	if (hasFinishedSampling()
		&& opencv_quaternion_compute_average(m_calibrationState->cv_apertureOffsetQuats, cv_cameraOffsetQuat)
		&& opencv_vec3d_compute_average(m_calibrationState->cv_apertureOffsetPositions, cv_cameraOffsetPosition))
	{
		outRotationOffset= cv_quatd_to_MikanQuatd(cv_cameraOffsetQuat);
		outTranslationOffset= cv_vec3d_to_MikanVector3d(cv_cameraOffsetPosition);

		return true;
	}

	return false;
}

void MonoLensTrackerPoseCalibrator::renderApertureSpaceCalibrationState()
{
	const CameraPuckToApertureResults& results= m_calibrationState->cameraPuckToApertureResults;
	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();

	// Draw the most recently capture chessboard in camera space
	m_patternFinder->renderCalibrationPattern2D();

	// Draw the camera relative transforms of the pattern (computed from solvePnP)
	// and the mat puck location offset from the pattern origin
	if (results.bIsValid)
	{
		const glm::mat4 apertureToPatternXform= glm::mat4(results.apertureToPatternXform_CameraSpace);
		const glm::mat4 apertureToMatPuckXform= glm::mat4(results.apertureToMatPuckXform_CameraSpace);

		m_patternFinder->renderSolvePnPPattern3D(apertureToPatternXform);

		drawTransformedAxes(graphicsContext, apertureToPatternXform, 0.1f);
		drawTransformedAxes(graphicsContext, apertureToMatPuckXform, 0.1f);

		TextStyle style= getDefaultTextStyle();
		drawTextAtWorldPosition(graphicsContext, style, glm_mat4_get_position(apertureToPatternXform),
								L"Pattern (Aperture Space)");
		drawTextAtWorldPosition(graphicsContext, style, glm_mat4_get_position(apertureToMatPuckXform),
								L"Mat Puck (Aperture Space)");
	}
}

void MonoLensTrackerPoseCalibrator::renderVRSpaceCalibrationState()
{
	const CameraPuckToApertureResults& results= m_calibrationState->cameraPuckToApertureResults;
	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();
	TextStyle style= getDefaultTextStyle();

	// Draw the most recently captured chessboard projected into VR
	m_patternFinder->renderSolvePnPPattern3D(results.patternXform_VRSpace);

	// Draw the camera puck transform
	glm::mat4 cameraPuckXform_VRSpace;
	if (m_cameraPuckPose_VRSystemSpace->getPose(m_cameraComponent, cameraPuckXform_VRSpace))
	{
		drawTransformedAxes(graphicsContext, cameraPuckXform_VRSpace, 0.1f);
		drawTextAtWorldPosition(graphicsContext, style, glm_mat4_get_position(cameraPuckXform_VRSpace),
								L"Camera Puck (VR Space)");
	}

	// Draw the mat puck transform
	glm::mat4 matPuckXform_VRSpace;
	if (m_matPuckPose_VRSystemSpace->getPose(m_cameraComponent, matPuckXform_VRSpace))
	{
		drawTransformedAxes(graphicsContext, matPuckXform_VRSpace, 0.1f);
		drawTextAtWorldPosition(graphicsContext, style, glm_mat4_get_position(matPuckXform_VRSpace),
								L"Mat Puck (VR Space)");
	}

	// Draw the most recently derived camera transform derived from the mat puck
	const float hfov_radians= degrees_to_radians(m_calibrationState->inputCameraIntrinsics.hfov);
	const float vfov_radians= degrees_to_radians(m_calibrationState->inputCameraIntrinsics.vfov);
	const float zNear= fmaxf(m_calibrationState->inputCameraIntrinsics.znear, 0.1f);
	const float zFar= fminf(m_calibrationState->inputCameraIntrinsics.zfar, 2.0f);
	drawTransformedFrustum(graphicsContext, results.apertureXform_VRSpace, hfov_radians, vfov_radians, zNear, zFar,
						   Colors::Yellow);
	drawTransformedAxes(graphicsContext, results.apertureXform_VRSpace, 0.1f);
	drawTextAtWorldPosition(graphicsContext, style, glm_mat4_get_position(results.apertureXform_VRSpace),
							L"Aperture (VR Space)");
}