#pragma once

#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"
#include <memory>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class ArucoMarkerPoseSampler
{
public:
	ArucoMarkerPoseSampler(
		CameraComponentPtr cameraComponent,
		class VideoFrameDistortionView* distortionView,
		int desiredSampleCount);
	virtual ~ArucoMarkerPoseSampler();

	inline class CalibrationPatternFinder_Aruco* getPatternFinder() const { return m_markerFinder; }

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeVRSpaceMarkerXform();
	bool hasValidVRSpaceMarkerXform() const;
	void sampleLastVRSpaceMarkerXform();
	bool computeCalibratedMarkerPose(MikanQuatd& outRotation, MikanVector3d& outTranslation);

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Calibration State
	struct ArucoMarkerPoseSamplerState* m_calibrationState;

	// Tracked camera used for calibration
	CameraComponentPtr m_calibrationCamera;

	// VR System-space pose of the camera puck
	VRDevicePoseViewPtr m_cameraPuckPose_VRSystemSpace;

	// Calibration pattern being used
	class CalibrationPatternFinder_Aruco* m_markerFinder;
};