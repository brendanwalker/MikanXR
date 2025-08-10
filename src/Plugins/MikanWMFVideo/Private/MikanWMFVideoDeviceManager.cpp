#include "MikanWMFVideoDevice.h"
#include "MikanWMFVideoDeviceManager.h"
#include "Logger.h"

// -- MikanWMFVideoDeviceManager ------
MikanWMFVideoDeviceManager::MikanWMFVideoDeviceManager()
{}

MikanWMFVideoDeviceManager::~MikanWMFVideoDeviceManager()
{
}

void MikanWMFVideoDeviceManager::addListener(IUsbVideoDeviceManagerListener* eventListener)
{
	//auto it = std::find(m_listeners.begin(), m_listeners.end(), eventListener);
	//if (it == m_listeners.end())
	//{
	//	m_listeners.push_back(eventListener);
	//}
}

void MikanWMFVideoDeviceManager::removeListener(IUsbVideoDeviceManagerListener* eventListener)
{
	//auto it = std::find(m_listeners.begin(), m_listeners.end(), eventListener);
	//if (it != m_listeners.end())
	//{
	//	m_listeners.erase(it);
	//}
}

bool MikanWMFVideoDeviceManager::startup()
{
	return true;
}

void MikanWMFVideoDeviceManager::update(float deltaTime)
{
}

void MikanWMFVideoDeviceManager::shutdown()
{
}

size_t MikanWMFVideoDeviceManager::getDeviceCount() const
{
	return 0;
}

IUsbVideoDevice* MikanWMFVideoDeviceManager::getDeviceByIndex(size_t index)
{
	return nullptr;
}

IUsbVideoDevice* MikanWMFVideoDeviceManager::getDeviceByPath(const char* devicePath)
{
	return nullptr;
}