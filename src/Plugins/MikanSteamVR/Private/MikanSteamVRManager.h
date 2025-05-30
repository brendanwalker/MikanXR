#pragma once

#include "IVRDeviceManager.h"
#include <vector>

class MikanSteamVRManager : public IVRDeviceManager
{
public:
	MikanSteamVRManager() = default;
	virtual ~MikanSteamVRManager();

	virtual void addListener(IVRDeviceManagerListener* eventListener) override;
	virtual void removeListener(IVRDeviceManagerListener* eventListener) override;

	virtual bool startup() override;
	virtual void update(float deltaTime) override;
	virtual void shutdown()  override;

	virtual size_t getDeviceCount() const override;
	virtual class IVRDevice* getDeviceByIndex(size_t index) override;
	virtual class IVRDevice* getDeviceByPath(const char* devicePath) override;

private: 
	std::vector<IVRDeviceManagerListener*> m_listeners;
};