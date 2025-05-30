#pragma once

#include <memory>
#include <stdint.h>

class IVRDeviceManagerListener
{
public:
	virtual ~IVRDeviceManagerListener() {};

	virtual void onActiveDeviceListChanged() = 0;
	virtual void onDevicePropertyChanged(int deviceId) = 0;
	virtual void onDevicePosesChanged(int64_t newFrameId) = 0;
};

class IVRDeviceManager
{
public:
	IVRDeviceManager() = default;
	virtual ~IVRDeviceManager() {}

	virtual void addListener(IVRDeviceManagerListener* eventListener) = 0;
	virtual void removeListener(IVRDeviceManagerListener* eventListener) = 0;

	virtual bool startup() = 0;
	virtual void update(float deltaTime) = 0;
	virtual void shutdown() = 0;

	virtual size_t getDeviceCount() const = 0;
	virtual class IVRDevice* getDeviceByIndex(size_t index) = 0;
	virtual class IVRDevice* getDeviceByPath(const char* devicePath) = 0;
};
using IVRDeviceManagerPtr = std::shared_ptr<IVRDeviceManager>;