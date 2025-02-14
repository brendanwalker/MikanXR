#pragma once

//-- includes -----
#include <memory>
#include <deque>
#include <vector>

#include "CommonConfig.h"
#include "DeviceViewFwd.h"
#include "DeviceManager.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"
#include "MulticastDelegate.h"
#include "SteamVRManager.h"
#include "stdint.h"

#include <glm/ext/matrix_float4x4.hpp>

//-- typedefs -----
using VRDeviceList = std::vector<VRDeviceViewPtr>;

//-- definitions -----
class VRDeviceManager : public DeviceManager, public IVRSystemEventListener
{
public:
	VRDeviceManager();
	virtual ~VRDeviceManager();

	inline static VRDeviceManager* getInstance() { return m_instance; }

	bool startup(class IMkWindow *ownerWindow) override;
	void update(float deltaTime) override;
	void shutdown() override;

	void closeAllVRTrackers();

	static const int k_max_devices = 8;
	int getMaxDevices() const override
	{
		return VRDeviceManager::k_max_devices;
	}

	int64_t getLastVRFrameIndex() const { return m_lastVRFrameIndex; }

	inline class SteamVRManager* getSteamVRManager() const { return m_steamVRManager; }

	VRDeviceViewPtr getVRDeviceViewById(int device_id) const;
	VRDeviceViewPtr getVRDeviceViewByPath( const std::string& devicePath) const;
	VRDeviceList getVRDeviceList() const;
	VRDeviceList getFilteredVRDeviceList(eDeviceType deviceType) const;
	const glm::mat4& getVRDevicePoseOffset() const { return m_vrDevicePoseOffset; }

	//-- IVRSystemEventListener ----
	void onActiveDeviceListChanged() override;
	void onDevicePropertyChanged(int device_id)  override;
	void onDevicePosesChanged(int64_t newFrameIndex) override;

	MulticastDelegate<void()> OnDeviceListChanged;
	MulticastDelegate<void(int64_t newVRFrameIndex)> OnDevicePosesChanged;

protected:
	static VRDeviceManager* m_instance;

	class SteamVRManager *m_steamVRManager= nullptr;
	int64_t m_lastVRFrameIndex= 0;
	glm::mat4 m_vrDevicePoseOffset= glm::mat4(1.0f);

	DeviceEnumerator* allocateDeviceEnumerator() override;
	void freeDeviceEnumerator(DeviceEnumerator*) override;
	DeviceView* allocateDeviceView(int device_id) override;

	// -- ProfileConfig Events --
	void onProfileConfigMarkedDirty(
		CommonConfigPtr configPtr,
		const ConfigPropertyChangeSet& changedPropertySet);
	void onVRTrackingOffsetChanged(ProfileConfigPtr config);
};

class VRDeviceListIterator
{
public:
	VRDeviceListIterator() = default;
	VRDeviceListIterator(eDeviceType deviceType);

	inline bool hasVRDevices() const { return m_vrDeviceList.size() > 0; }
	inline int getCurrentIndex() const { return m_listIndex; }
	VRDeviceViewPtr getCurrent() const;
	bool goPrevious();
	bool goNext();

private:
	VRDeviceList m_vrDeviceList;
	int m_listIndex = 0;
};