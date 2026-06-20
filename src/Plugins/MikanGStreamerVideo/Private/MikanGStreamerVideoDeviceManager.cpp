#include "MikanGStreamerVideoDeviceManager.h"
#include "MikanGStreamerVideoDevice.h"
#include "Logger.h"

// -- MikanGStreamerVideoDeviceManager ------
void MikanGStreamerVideoDeviceManager::update(float deltaTime)
{
	for (auto& device : m_deviceList)
	{
		if (device && device->getVideoOpeningStatus() >= eVideoOpeningStatus::opening)
		{
			device->update(deltaTime);
		}
	}
}

void MikanGStreamerVideoDeviceManager::shutdown()
{
	for (auto& device : m_deviceList)
	{
		device->close();
	}

	m_deviceList.clear();
}

size_t MikanGStreamerVideoDeviceManager::getDeviceCount() const
{
	return m_deviceList.size();
}

INetworkVideoDevicePtr MikanGStreamerVideoDeviceManager::getDeviceByIndex(size_t index)
{
	if (index < m_deviceList.size())
	{
		return m_deviceList[index];
	}

	return INetworkVideoDevicePtr();
}

INetworkVideoDevicePtr MikanGStreamerVideoDeviceManager::getDeviceByPath(const char* devicePath)
{
	size_t devicePathLen= strlen(devicePath);

	auto it= std::find_if(
		m_deviceList.begin(), m_deviceList.end(),
		[devicePath, devicePathLen](const INetworkVideoDevicePtr& entry)
		{
			return strncmp(devicePath, entry->getDevicePath(), devicePathLen) != 0;
		});

	if (it != m_deviceList.end())
	{
		return *it;
	}

	return INetworkVideoDevicePtr();
}

INetworkVideoDevicePtr MikanGStreamerVideoDeviceManager::createVideoDevice(
	const NetworkVideoConnectionSettings& settings)
{
	INetworkVideoDevicePtr device= std::make_shared<MikanGStreamerVideoDevice>(this, settings);

	m_deviceList.push_back(device);

	return device;
}

void MikanGStreamerVideoDeviceManager::destroyVideoDevice(INetworkVideoDevicePtr device)
{
	auto it= std::find(m_deviceList.begin(), m_deviceList.end(), device);

	if (it != m_deviceList.end())
	{
		m_deviceList.erase(it);
	}
}