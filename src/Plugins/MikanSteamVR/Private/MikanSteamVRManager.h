#pragma once

#include "IVRDevice.h"
#include "IVRDeviceManager.h"
#include "MikanSteamVRDeviceFwd.h"

#include <set>
#include <string>
#include <vector>

using SteamVRIdSet = std::set<int>;
using SteamVRIdSetIter = std::set<int>::iterator;

using SteamVRDeviceList = std::vector<IVRDevice*>;
using SteamVRDeviceListIter = std::vector<IVRDevicePtr*>::iterator;

class MikanSteamVRManager : public IVRDeviceManager
{
public:
	MikanSteamVRManager();
	virtual ~MikanSteamVRManager();

	// IVRDeviceManager
	virtual void addListener(IVRDeviceManagerListener* eventListener) override;
	virtual void removeListener(IVRDeviceManagerListener* eventListener) override;

	virtual bool startup(class IMkGraphicsContext* graphicsContext) override;
	virtual void update(float deltaTime) override;
	virtual void shutdown()  override;

	virtual size_t getDeviceCount() const override;
	virtual class IVRDevice* getDeviceByIndex(size_t index) override;
	virtual class IVRDevice* getDeviceByPath(const char* devicePath) override;

	// MikanSteamVRManager
	inline class IMkGraphicsContext* getOwnerContext() const { return m_ownerContext; }
	inline std::unique_ptr<class SteamVRResourceManager>& getResourceManager() { return m_resourceManager; }
	SteamVRDeviceList getActiveDevices() const;
	SteamVRDeviceList getActiveDevicesOfType(eVRDeviceType deviceType) const;
	const vr::TrackedDevicePose_t* getDevicePose(
		vr::TrackedDeviceIndex_t steamvrDeviceId,
		int vrFrameDelay) const;

protected:
	bool tryConnect();
	void disconnect();
	void addConnectedDeviceIdsOfClass(int deviceClassEnumValue);
	void handleTrackedDeviceActivated(vr::TrackedDeviceIndex_t deviceIndex);
	void handleTrackedDevicePropertyChanged(vr::TrackedDeviceIndex_t deviceIndex);
	void handleTrackedDeviceDeactivated(vr::TrackedDeviceIndex_t deviceIndex);
	void updateDevicePoses();

private:
	static const float k_reconnectTimeoutDuration;
	static const int k_maxReconnectAttempts;

	class IMkGraphicsContext* m_ownerContext;
	std::vector<IVRDeviceManagerListener*> m_listeners;
	float m_reconnectTimeout;
	int m_reconnectAttemptCount;
	std::unique_ptr< class DeviceSetPoseHistory > m_devicePoseHistory;
	int64_t m_vrFrameCounter;
	std::unique_ptr< class SteamVRResourceManager > m_resourceManager;
	SteamVRIdSet m_activeSteamVRDeviceIdSet;
	std::vector<MikanSteamVRDevicePtr> m_activeSteamVRDeviceList;
};