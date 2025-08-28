#pragma once

#include "INetworkVideoDevice.h"
#include "INetworkVideoDeviceManager.h"

#include <memory>
#include <string>
#include <vector>

class MikanGStreamerVideoDeviceManager : public INetworkVideoDeviceManager
{
public:
	MikanGStreamerVideoDeviceManager() = default;
	virtual ~MikanGStreamerVideoDeviceManager() {}

	virtual bool startup() override { return true; }
	virtual void update(float deltaTime) override;
	virtual void shutdown() override;

	virtual size_t getDeviceCount() const override;
	virtual INetworkVideoDevicePtr getDeviceByIndex(size_t index) override;
	virtual INetworkVideoDevicePtr getDeviceByPath(const char* devicePath) override;

	virtual INetworkVideoDevicePtr createVideoDevice(
		const NetworkVideoConnectionSettings& settings) override;
	virtual void destroyVideoDevice(INetworkVideoDevicePtr device) override;

private:
	std::vector<INetworkVideoDevicePtr> m_deviceList;
};