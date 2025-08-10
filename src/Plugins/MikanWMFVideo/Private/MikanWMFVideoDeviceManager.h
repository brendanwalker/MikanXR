#pragma once

#include "IUsbVideoDevice.h"
#include "IUsbVideoDeviceManager.h"

class MikanWMFVideoDeviceManager : public IUsbVideoDeviceManager
{
public:
	MikanWMFVideoDeviceManager();
	virtual ~MikanWMFVideoDeviceManager();

	// IUsbVideoDeviceManager
	virtual void addListener(IUsbVideoDeviceManagerListener* eventListener) override;
	virtual void removeListener(IUsbVideoDeviceManagerListener* eventListener) override;

	virtual bool startup() override;
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;

	virtual size_t getDeviceCount() const override;
	virtual class IUsbVideoDevice* getDeviceByIndex(size_t index) override;
	virtual class IUsbVideoDevice* getDeviceByPath(const char* devicePath) override;

private:
};