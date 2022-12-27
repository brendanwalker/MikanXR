#pragma once

#include <memory>
#include "MikanMathTypes.h"
#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class MonoLensTrackerPoseCalibrator
{
public:
	MonoLensTrackerPoseCalibrator(
		const class ProfileConfig* profileConfig,
		VRDeviceViewPtr cameraTrackingPuckView,
		VRDeviceViewPtr matTrackingPuckView,
		class VideoFrameDistortionView* distortionView,
		int desiredSampleCount);
	virtual ~MonoLensTrackerPoseCalibrator();

	inline class CalibrationPatternFinder* getPatternFinder() const { return m_patternFinder; }

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeCameraToPuckXform();
	glm::mat4 getLastCameraPose(VRDeviceViewPtr attachedVRDevicePtr) const;
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
	VRDeviceViewPtr m_cameraTrackingPuckView;
	VRDeviceViewPtr m_matTrackingPuckView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Calibration pattern being used
	class CalibrationPatternFinder* m_patternFinder;
};