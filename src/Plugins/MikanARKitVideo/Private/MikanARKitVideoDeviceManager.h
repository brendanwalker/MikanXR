#pragma once

#include "IARKitVideoDevice.h"
#include "IARKitVideoDeviceManager.h"

#include <memory>
#include <string>
#include <vector>

class MikanARKitVideoDeviceManager : public IARKitVideoDeviceManager
{
public:
	MikanARKitVideoDeviceManager()= default;
	virtual ~MikanARKitVideoDeviceManager() {}

	virtual bool startup() override { return true; }
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;

	virtual size_t getDeviceCount() const override;
	virtual IARKitVideoDevicePtr getDeviceByIndex(size_t index) override;
	virtual IARKitVideoDevicePtr getDeviceByPath(const char* devicePath) override;

	virtual IARKitVideoDevicePtr createVideoDevice(const ARKitVideoConnectionSettings& settings) override;
	virtual void destroyVideoDevice(IARKitVideoDevicePtr device) override;

private:
	std::vector<IARKitVideoDevicePtr> m_deviceList;
};
