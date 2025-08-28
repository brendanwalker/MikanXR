#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"

#include <map>
#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class VRDevicePoseView;
using VRDevicePoseViewPtr = std::shared_ptr<VRDevicePoseView>;

// -- constants -----
enum class eVRDevicePoseSpace : int
{
	INVALID,

	VRTrackingSystem,
	MikanScene,

	COUNT,
};

// -- VRDevicePoseView -----
class VRDevicePoseView
{
public:
	VRDevicePoseView();
	VRDevicePoseView(
		const VRDeviceComponent* deviceComponent,
		eVRDevicePoseSpace space,
		const std::string& socketName = "");

	static VRDevicePoseViewPtr makePoseView(
		const VRDeviceComponent* deviceComponent,
		eVRDevicePoseSpace space,
		const std::string& socketName = "");
	static VRDevicePoseViewPtr makeInvalidPoseView();

	inline eVRDevicePoseSpace getPoseSpace() const { return m_poseSpace; }

	const VRDeviceComponent* getDeviceComponent() const;
	bool getPose(CameraComponentConstPtr cameraContext, glm::mat4& outPoseInSpace) const;
	bool getPose(CameraComponentConstPtr cameraContext, glm::dmat4& outPoseInSpace) const;

private:
	VRDeviceComponentConstWeakPtr m_deviceComponent;
	eVRDevicePoseSpace m_poseSpace;
	std::string m_socketName;
};