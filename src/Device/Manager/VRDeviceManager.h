#pragma once

//-- includes -----
#include <memory>
#include <deque>
#include <vector>

#include "DeviceManager.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"
#include "MulticastDelegate.h"
#include "SteamVRManager.h"
#include "stdint.h"

//-- typedefs -----
class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;
typedef std::vector<VRDeviceViewPtr> VRDeviceList;

//-- definitions -----
class VRDeviceManager : public DeviceManager, public IVRSystemEventListener
{
public:
	VRDeviceManager();
	virtual ~VRDeviceManager();

	inline static VRDeviceManager* getInstance() { return m_instance; }

	bool startup() override;
	void update(float deltaTime) override;
	void shutdown() override;

	void closeAllVRTrackers();

	static const int k_max_devices = 8;
	int getMaxDevices() const override
	{
		return VRDeviceManager::k_max_devices;
	}

	uint64_t getLastVRFrameIndex() const { return m_lastVRFrameIndex; }

	inline class SteamVRManager* getSteamVRManager() const { return m_steamVRManager; }

	VRDeviceViewPtr getVRDeviceViewPtr(int device_id) const;
	VRDeviceList getVRDeviceList() const;
	VRDeviceList getFilteredVRDeviceList(eDeviceType deviceType) const;

	//-- IVRSystemEventListener ----
	void onActiveDeviceListChanged() override;
	void onDevicePropertyChanged(int device_id)  override;
	void onDevicePosesChanged(uint64_t newFrameIndex) override;

	MulticastDelegate<void()> OnDeviceListChanged;
	MulticastDelegate<void(uint64_t newVRFrameIndex)> OnDevicePosesChanged;

protected:
	static VRDeviceManager* m_instance;
	class SteamVRManager *m_steamVRManager= nullptr;
	uint64_t m_lastVRFrameIndex= 0;

	DeviceEnumerator* allocateDeviceEnumerator() override;
	void freeDeviceEnumerator(DeviceEnumerator*) override;
	DeviceView* allocateDeviceView(int device_id) override;
};

class VRDeviceListIterator
{
public:
	VRDeviceListIterator() = default;
	VRDeviceListIterator(eDeviceType deviceType, const std::string& devicePath);

	inline bool hasVRDevices() const { return m_vrDeviceList.size() > 0; }
	inline int getCurrentIndex() const { return m_listIndex; }
	VRDeviceViewPtr getCurrent() const;
	bool goPrevious();
	bool goNext();

private:
	VRDeviceList m_vrDeviceList;
	int m_listIndex = 0;
};