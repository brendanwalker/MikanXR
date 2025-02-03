#pragma once

#include "MikanMathTypes.h"
#include "DeviceViewFwd.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class CameraTrackerOffsetCalibrator
{
public:
	CameraTrackerOffsetCalibrator(
		ProfileConfigConstPtr profileConfig,
		VRDevicePoseViewPtr cameraTrackingPuckPoseView,
		class VideoFrameDistortionView* distortionView,
		int desiredSampleCount);
	virtual ~CameraTrackerOffsetCalibrator();

	inline class CalibrationPatternFinder* getPatternFinder() const { return m_patternFinder; }

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeCameraToPuckXform();
	bool getLastCameraPose(VRDevicePoseViewPtr attachedVRDevicePtr, glm::mat4& outCameraPose) const;
	void sampleLastCameraToPuckXform();
	bool computeCalibratedCameraTrackerOffset(MikanQuatd& outRotationOffset, MikanVector3d& outTranslationOffset);

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Calibration State
	struct CameraTrackerOffsetCalibrationState* m_calibrationState;

	// Tracking pucks used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Calibration pattern being used
	class CalibrationPatternFinder* m_patternFinder;
};