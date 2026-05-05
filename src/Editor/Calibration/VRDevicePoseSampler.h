#pragma once

#include "ComponentFwd.h"
#include "MikanMathTypes.h"
#include "VRDevicePoseView.h"

#include <memory>

class VRDevicePoseSampler
{
public:
	VRDevicePoseSampler(
		VRDevicePoseViewPtr poseView,
		CameraComponentConstPtr cameraContext,
		int desiredSampleCount);
	virtual ~VRDevicePoseSampler();

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeVRDeviceXform();
	bool hasValidVRDeviceXform() const;
	bool sampleLastVRDeviceXform();

	// Average all accumulated samples into a single pose
	bool computeCalibratedDevicePose(MikanQuatd& outRotation, MikanVector3d& outTranslation);

private:
	struct VRDevicePoseSamplerState* m_state;
	VRDevicePoseViewPtr m_poseView;
	CameraComponentConstWeakPtr m_cameraContext;
};
