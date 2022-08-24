#pragma once

//-- includes -----
#include "VRDeviceInterface.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

//-- typedefs -----
typedef std::set<int> SteamVRIdSet;
typedef std::set<int>::iterator SteamVRIdSetIter;

typedef std::vector<int> SteamVRIdList;
typedef std::vector<int>::iterator SteamVRIdListIter;

namespace vr
{
	typedef uint32_t TrackedDeviceIndex_t;
	enum ETrackedDeviceClass;
	struct TrackedDevicePose_t;
	class IVRSystem;
};

//-- definitions -----
class SteamVRManager
{
public:
	SteamVRManager(IVRSystemEventListener* listener);
	virtual ~SteamVRManager();

	bool startup();
	void update(float deltaTime);
	void shutdown();

	inline const SteamVRIdSet& getActiveDeviceIdSet() const { return m_activeSteamVRDeviceIdSet; }
	SteamVRIdList getActiveDevices() const;
	SteamVRIdList getActiveDevicesOfType(eDeviceType deviceType) const;
	eDeviceType getDeviceType(vr::TrackedDeviceIndex_t steamvrDeviceId) const;
	const struct vr::TrackedDevicePose_t* getDevicePose(vr::TrackedDeviceIndex_t steamvrDeviceId) const;
	int getDeviceVendorId(vr::TrackedDeviceIndex_t steamvrDeviceId) const;
	int getDeviceProductId(vr::TrackedDeviceIndex_t steamvrDeviceId) const;
	std::string getDevicePath(vr::TrackedDeviceIndex_t steamvrDeviceId) const;

	std::unique_ptr<class SteamVRResourceManager>& getResourceManager() { return m_resourceManager; }

protected:
	bool tryConnect();
	void disconnect();
	void addConnectedDeviceIdsOfClass(vr::ETrackedDeviceClass deviceClass);
	void handleTrackedDeviceActivated(vr::TrackedDeviceIndex_t deviceIndex);
	void handleTrackedDevicePropertyChanged(vr::TrackedDeviceIndex_t deviceIndex);
	void handleTrackedDeviceDeactivated(vr::TrackedDeviceIndex_t deviceIndex);
	void updateDevicePoses();

	static const float k_reconnectTimeoutDuration;
	float m_reconnectTimeout;
	std::unique_ptr< class DeviceSetPoseHistory > m_devicePoseHistory;
	const struct DeviceSetPoseSample* m_currentPoseSet;
	uint64_t m_vrFrameCounter;
	std::unique_ptr< class SteamVRResourceManager > m_resourceManager;
	IVRSystemEventListener* m_eventListener= nullptr;
	SteamVRIdSet m_activeSteamVRDeviceIdSet;
};