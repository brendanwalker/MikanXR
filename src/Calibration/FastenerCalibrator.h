#pragma once

#include <memory>
#include "MikanClientTypes.h"
#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class FastenerCalibrator
{
public:
	FastenerCalibrator(
		const class ProfileConfig* profileConfig,
		VRDeviceViewPtr cameraTrackingPuckView,
		class VideoFrameDistortionView* distortionView);
	virtual ~FastenerCalibrator();

	bool hasFinishedSampling() const;
	void resetCalibrationState();

	void sampleMouseScreenPosition();
	void sampleCameraPose();
	bool computeFastenerPoints(MikanSpatialFastenerInfo* fastener);

	void renderCameraSpaceCalibrationState(const int cameraPoseIndex);
	void renderVRSpacePreCalibrationState(const int cameraPoseIndex);
	void renderVRSpacePostCalibrationState();

protected:
	float m_frameWidth;
	float m_frameHeight;
	const class ProfileConfig* m_profileConfig;

	// Internal Calibration State
	struct FastenerCalibrationState* m_calibrationState;

	// Tracking puck used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;
};