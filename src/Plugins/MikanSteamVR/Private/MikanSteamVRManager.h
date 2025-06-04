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

	virtual bool startup(class IMkWindow* ownerWindow) override;
	virtual void update(float deltaTime) override;
	virtual void shutdown()  override;

	virtual size_t getDeviceCount() const override;
	virtual class IVRDevice* getDeviceByIndex(size_t index) override;
	virtual class IVRDevice* getDeviceByPath(const char* devicePath) override;

	// MikanSteamVRManager
	SteamVRDeviceList getActiveDevices() const;
	SteamVRDeviceList getActiveDevicesOfType(eVRDeviceType deviceType) const;
	eVRDeviceType getDeviceType(vr::TrackedDeviceIndex_t steamVRDeviceId) const;
	const vr::TrackedDevicePose_t* getDevicePose(
		vr::TrackedDeviceIndex_t steamvrDeviceId,
		int vrFrameDelay = 0) const;
	int getDeviceVendorId(vr::TrackedDeviceIndex_t steamvrDeviceId) const;
	int getDeviceProductId(vr::TrackedDeviceIndex_t steamvrDeviceId) const;
	std::string getDevicePath(vr::TrackedDeviceIndex_t steamvrDeviceId) const;

protected:
	bool tryConnect();
	void disconnect();
	void addConnectedDeviceIdsOfClass(vr::ETrackedDeviceClass deviceClass);
	void handleTrackedDeviceActivated(vr::TrackedDeviceIndex_t deviceIndex);
	void handleTrackedDevicePropertyChanged(vr::TrackedDeviceIndex_t deviceIndex);
	void handleTrackedDeviceDeactivated(vr::TrackedDeviceIndex_t deviceIndex);
	void updateDevicePoses();

private:
	static const float k_reconnectTimeoutDuration;
	static const int k_maxReconnectAttempts;

	class IMkWindow* m_ownerWindow= nullptr;
	std::vector<IVRDeviceManagerListener*> m_listeners;
	float m_reconnectTimeout;
	int m_reconnectAttemptCount = 0;
	std::unique_ptr< class DeviceSetPoseHistory > m_devicePoseHistory;
	int64_t m_vrFrameCounter;
	SteamVRIdSet m_activeSteamVRDeviceIdSet;
	std::vector<MikanSteamVRDevicePtr> m_activeSteamVRDeviceList;
};