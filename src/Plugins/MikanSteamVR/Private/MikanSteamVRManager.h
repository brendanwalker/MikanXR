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

	virtual bool startup(class IMkWindow* ownerWindow) override;
	virtual void update(float deltaTime) override;
	virtual void shutdown()  override;

	virtual size_t getDeviceCount() const override;
	virtual class IVRDevice* getDeviceByIndex(size_t index) override;
	virtual class IVRDevice* getDeviceByPath(const char* devicePath) override;

private:
	class IMkWindow* m_ownerWindow= nullptr;
	std::vector<IVRDeviceManagerListener*> m_listeners;
};