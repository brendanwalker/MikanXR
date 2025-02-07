#pragma once

// -- includes -----
#include "MikanRendererFwd.h"
#include "SteamVRDevice.h"
#include "VRDeviceInterface.h"

#include <map>
#include <vector>

namespace vr
{
	struct TrackedDevicePose_t;
	struct HmdMatrix34_t;
}

// -- definitions -----
class SteamVRDevice : public IVRDeviceInterface {
public:
	SteamVRDevice();
	virtual ~SteamVRDevice();

	bool open(); // Opens the first available tracker

	void setPoseMatrix(const glm::mat4& mat);
	inline const glm::mat4& getPoseMatrix() const { return m_poseMatrix; }

	class VRDeviceManager* getOwnerDeviceManager() const { return m_ownerDeviceManager; }
	class SteamVRDeviceProperties* getSteamVRDeviceProperties() const { return m_deviceProperties; }

	// -- IDeviceInterface
	bool matchesDeviceEnumerator(const class DeviceEnumerator* enumerator) const override;
	bool open(const class DeviceEnumerator* enumerator) override;
	bool getIsOpen() const override;
	void close() override;
	static eDeviceType getDeviceTypeStatic()
	{
		return eDeviceType::VRTracker;
	}
	eDeviceType getDeviceType() const override;

	// -- IVRDeviceInterface
	IVRDeviceInterface::eDriverType getDriverType() const override {
		return IVRDeviceInterface::eDriverType::SteamVR;
	}
	void updateProperties() override;
	void updatePose() override;
	void bindToScene(GlScenePtr scene) override;
	void removeFromBoundScene() override;
	std::string getDevicePath() const override;
	std::string getSerialNumber() const override;
	std::string getTrackerRole() const override;
	bool getIsPoseValid() const override;
	glm::quat getOrientation() const override;
	glm::vec3 getPosition() const override;
	const CommonVRDeviceState* getVRTrackerState() override;
	bool getComponentPoseByName(const std::string& componentName, glm::mat4& outPose) const override;
	void getComponentNames(std::vector<std::string>& outComponentName) const override;

private:
	void rebuildRenderComponents();
	void disposeRenderComponents();

	class VRDeviceManager* m_ownerDeviceManager= nullptr;
	class SteamVRDeviceProperties *m_deviceProperties;
	std::string m_devicePath;
	IVRDeviceInterface::eDriverType m_driverType;
	eDeviceType m_deviceType;

	GlSceneWeakPtr m_boundScene;
	std::map<std::string, class SteamVRRenderComponent*> m_renderComponents;
	glm::mat4 m_poseMatrix;
	bool m_isPoseValid= false;
	CommonVRDeviceState* m_vrTrackerState;
};

