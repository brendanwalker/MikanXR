#include "MikanSteamVRManager.h"
#include "IMkWindow.h"
#include "IVRDevice.h"

MikanSteamVRManager::~MikanSteamVRManager()
{
}

void MikanSteamVRManager::addListener(IVRDeviceManagerListener* eventListener)
{
	auto it = std::find(m_listeners.begin(), m_listeners.end(), eventListener);
	if (it == m_listeners.end())
	{
		m_listeners.push_back(eventListener);
	}
}

void MikanSteamVRManager::removeListener(IVRDeviceManagerListener* eventListener)
{
	auto it = std::find(m_listeners.begin(), m_listeners.end(), eventListener);
	if (it != m_listeners.end())
	{
		m_listeners.erase(it);
	}
}

bool MikanSteamVRManager::startup(IMkWindow* ownerWindow)
{
	m_ownerWindow= ownerWindow;

	return true;
}

void MikanSteamVRManager::update(float deltaTime)
{
}

void MikanSteamVRManager::shutdown()
{
}

size_t MikanSteamVRManager::getDeviceCount() const
{
	return 0;
}

IVRDevice* MikanSteamVRManager::getDeviceByIndex(size_t index)
{
	return nullptr;
}

IVRDevice* MikanSteamVRManager::getDeviceByPath(const char* devicePath)
{
	return nullptr;
}
