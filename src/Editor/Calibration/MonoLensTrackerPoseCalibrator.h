#pragma once

#include "MikanMathTypes.h"
#include "DeviceViewFwd.h"
#include "ObjectSystemConfigFwd.h"

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class MonoLensTrackerPoseCalibrator
{
public:
	MonoLensTrackerPoseCalibrator(
		ProfileConfigConstPtr profileConfig,
		VRDevicePoseViewPtr cameraTrackingPuckView,
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
	bool getLastCameraPose(VRDevicePoseViewPtr attachedVRDevicePtr, glm::mat4& outCameraPose) const;
	void sampleLastCameraToPuckXform();
	bool computeCalibratedCameraTrackerOffset(MikanQuatd& outRotationOffset, MikanVector3d& outTranslationOffset);

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Calibration State
	struct MonoLensTrackerCalibrationState* m_calibrationState;

	// Tracking pucks used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;
	VRDevicePoseViewPtr m_matTrackingPuckPoseView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Calibration pattern being used
	class CalibrationPatternFinder* m_patternFinder;
};