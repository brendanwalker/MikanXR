#include "DeviceHotplugNotifier.h"
#include "MikanWMFVideoDevice.h"
#include "MikanWMFVideoDeviceManager.h"
#include "Logger.h"
#include "WMFDeviceList.h"

// -- MikanWMFVideoDeviceManager ------
MikanWMFVideoDeviceManager::MikanWMFVideoDeviceManager()
	: m_wmfDeviceList(new WMFDeviceList())
	, m_deviceHotplugNotifier(new DeviceHotplugNotifier())
{}

MikanWMFVideoDeviceManager::~MikanWMFVideoDeviceManager()
{
	delete m_wmfDeviceList;
}

void MikanWMFVideoDeviceManager::addListener(IUsbVideoDeviceManagerListener* eventListener)
{
	auto it = std::find(m_eventListeners.begin(), m_eventListeners.end(), eventListener);
	if (it == m_eventListeners.end())
	{
		m_eventListeners.push_back(eventListener);
	}
}

void MikanWMFVideoDeviceManager::removeListener(IUsbVideoDeviceManagerListener* eventListener)
{
	auto it = std::find(m_eventListeners.begin(), m_eventListeners.end(), eventListener);
	if (it != m_eventListeners.end())
	{
		m_eventListeners.erase(it);
	}
}

bool MikanWMFVideoDeviceManager::startup()
{
	if (!m_deviceHotplugNotifier->startup(this))
	{
		MIKAN_LOG_INFO("MikanWMFVideoDeviceManager::startup") << "Failed to start hotplug listener";
		return false;
	}

	rebuildDeviceList();

	return true;
}

void MikanWMFVideoDeviceManager::update(float deltaTime)
{
	m_deviceHotplugNotifier->update();
}

void MikanWMFVideoDeviceManager::shutdown()
{
	m_deviceHotplugNotifier->shutdown();
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

void MikanWMFVideoDeviceManager::onDeviceConnected(const std::string& path)
{
	MIKAN_LOG_INFO("MikanWMFVideoDeviceManager") << "Device connected: " << path;
	rebuildDeviceList();
}

void MikanWMFVideoDeviceManager::onDeviceDisconnected(const std::string& path)
{
	MIKAN_LOG_INFO("MikanWMFVideoDeviceManager") << "Device disconnected: " << path;
	rebuildDeviceList();
}

void MikanWMFVideoDeviceManager::rebuildDeviceList()
{
	m_wmfDeviceList->rebuild();

	for (IUsbVideoDeviceManagerListener* listener : m_eventListeners)
	{
		listener->onConnectedDeviceListChanged();
	}
}