#include "MikanARKitVideoDeviceManager.h"
#include "MikanARKitVideoDevice.h"
#include "Logger.h"

#include <algorithm>
#include <cstring>

// -- MikanARKitVideoDeviceManager ------
void MikanARKitVideoDeviceManager::update(float deltaTime)
{
	for (auto& device : m_deviceList)
	{
		if (device && device->getVideoOpeningStatus() >= eVideoOpeningStatus::opening)
		{
			device->update(deltaTime);
		}
	}
}

void MikanARKitVideoDeviceManager::shutdown()
{
	for (auto& device : m_deviceList)
	{
		device->close();
	}

	m_deviceList.clear();
}

size_t MikanARKitVideoDeviceManager::getDeviceCount() const { return m_deviceList.size(); }

IARKitVideoDevicePtr MikanARKitVideoDeviceManager::getDeviceByIndex(size_t index)
{
	if (index < m_deviceList.size())
	{
		return m_deviceList[index];
	}

	return IARKitVideoDevicePtr();
}

IARKitVideoDevicePtr MikanARKitVideoDeviceManager::getDeviceByPath(const char* devicePath)
{
	const size_t devicePathLen= strlen(devicePath);

	auto it= std::find_if(m_deviceList.begin(), m_deviceList.end(),
						  [devicePath, devicePathLen](const IARKitVideoDevicePtr& entry)
						  { return strncmp(devicePath, entry->getDevicePath(), devicePathLen) == 0; });

	if (it != m_deviceList.end())
	{
		return *it;
	}

	return IARKitVideoDevicePtr();
}

IARKitVideoDevicePtr MikanARKitVideoDeviceManager::createVideoDevice(const ARKitVideoConnectionSettings& settings)
{
	IARKitVideoDevicePtr device= std::make_shared<MikanARKitVideoDevice>(this, settings);

	m_deviceList.push_back(device);

	return device;
}

void MikanARKitVideoDeviceManager::destroyVideoDevice(IARKitVideoDevicePtr device)
{
	auto it= std::find(m_deviceList.begin(), m_deviceList.end(), device);

	if (it != m_deviceList.end())
	{
		m_deviceList.erase(it);
	}
}
