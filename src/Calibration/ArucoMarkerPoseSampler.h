#pragma once

#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class ArucoMarkerPoseSampler
{
public:
	ArucoMarkerPoseSampler(
		ProfileConfigConstPtr profileConfig,
		VRDeviceViewPtr cameraTrackingPuckView,
		class VideoFrameDistortionView* distortionView,
		int desiredSampleCount,
		bool bApplyVRDeviceOffset);
	virtual ~ArucoMarkerPoseSampler();

	inline class CalibrationPatternFinder_Aruco* getPatternFinder() const { return m_markerFinder; }

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeVRSpaceMarkerXform();
	void sampleLastVRSpaceMarkerXform();
	bool computeCalibratedMarkerPose(MikanQuatd& outRotation, MikanVector3d& outTranslation);

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Calibration State
	struct ArucoMarkerPoseSamplerState* m_calibrationState;

	// Tracking pucks used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Calibration pattern being used
	class CalibrationPatternFinder_Aruco* m_markerFinder;
};