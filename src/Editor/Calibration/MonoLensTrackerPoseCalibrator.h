#pragma once

#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class MonoLensTrackerPoseCalibrator
{
public:
	MonoLensTrackerPoseCalibrator(
		CameraComponentPtr cameraComponent,
		VRDevicePoseViewPtr cameraPuckPoseView,
		VRDevicePoseViewPtr matTrackingPuckView,
		class VideoFrameDistortionView* distortionView,
		int desiredSampleCount);
	virtual ~MonoLensTrackerPoseCalibrator();

	inline class CalibrationPatternFinder* getPatternFinder() const { return m_patternFinder; }

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeCameraToPuckXform();
	bool hasValidCameraToPuckXform() const;
	bool getLastSceneSpaceAperturePose(VRDevicePoseViewPtr attachedVRDevicePtr, glm::mat4& outCameraPose) const;
	void sampleLastCameraToPuckXform();
	bool computeCalibratedCameraTrackerOffset(MikanQuatd& outRotationOffset, MikanVector3d& outTranslationOffset);

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:
	// Protected constructor for test subclasses (bypasses CameraComponent + VideoFrameDistortionView)
	MonoLensTrackerPoseCalibrator(
		class CalibrationPatternFinder* patternFinder,
		const struct MikanMonoIntrinsics& cameraIntrinsics,
		int desiredSampleCount);

	// Overridable input accessors — test subclasses return stored test values
	virtual bool fetchCameraPuckVRSpacePose(glm::dmat4& outPose) const;
	virtual bool fetchMatPuckVRSpacePose(glm::dmat4& outPose) const;
	virtual glm::dvec3 fetchMatPuckOffsetMM() const;

	float frameWidth;
	float frameHeight;

	// Internal Calibration State
	struct MonoLensTrackerCalibrationState* m_calibrationState;

	// Tracking used for calibration
	CameraComponentPtr m_cameraComponent;
	VRDevicePoseViewPtr m_cameraPuckPose_VRSystemSpace;
	VRDevicePoseViewPtr m_matPuckPose_VRSystemSpace;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Calibration pattern being used
	class CalibrationPatternFinder* m_patternFinder;
};