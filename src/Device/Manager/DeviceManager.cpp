//-- includes -----
#include "DeviceManager.h"
#include "DeviceEnumerator.h"
#include "DeviceView.h"
#include "Logger.h"

#include <assert.h>

//-- methods -----
DeviceManager::DeviceManager()
	: m_deviceViews(nullptr)
{
}

DeviceManager::~DeviceManager()
{
	assert(m_deviceViews == nullptr);
}

/// Override if the device type needs to initialize any services (e.g., hid_init)
bool DeviceManager::startup(class IGlWindow* ownerWindow)
{
	assert(m_deviceViews == nullptr);

	m_ownerWindow = ownerWindow;

	const int maxDeviceCount = getMaxDevices();
	m_deviceViews = new DeviceViewPtr[maxDeviceCount];

	// Allocate all of the device views
	for (int device_id = 0; device_id < maxDeviceCount; ++device_id)
	{
		DeviceViewPtr deviceView = DeviceViewPtr(allocateDeviceView(device_id));

		m_deviceViews[device_id] = deviceView;
	}

	return true;
}

/// Override if the device type needs to teardown any services (e.g., hid_init)
void DeviceManager::shutdown()
{
	if (m_deviceViews != nullptr)
	{
		// Close any controllers that were opened
		for (int device_id = 0; device_id < getMaxDevices(); ++device_id)
		{
			DeviceViewPtr device = m_deviceViews[device_id];

			if (device->getIsOpen())
			{
				device->close();
			}

			m_deviceViews[device_id] = DeviceViewPtr();
		}

		// Free the device view pointer list
		delete[] m_deviceViews;
		m_deviceViews = nullptr;
	}
}

void DeviceManager::update(float deltaTime)
{
}

void DeviceManager::updateConnectedDeviceViews()
{
	const int maxDeviceCount = getMaxDevices();
	bool exists_in_enumerator[64];
	bool bSendControllerUpdatedNotification = false;

	// Initialize temp table used to keep track of open devices
	// still found in the enumerator
	assert(maxDeviceCount <= 64);
	memset(exists_in_enumerator, 0, sizeof(exists_in_enumerator));

	// Step 1
	// Mark any open devices that still show up in the enumerator.
	// Open devices shown in the enumerator that we haven't open yet.
	{
		DeviceEnumerator* enumerator = allocateDeviceEnumerator();

		while (enumerator->isValid())
		{
			// Find device index for the device with the matching device path
			int device_id = findOpenDeviceId(enumerator);

			// Existing device case (Most common)
			if (device_id != -1)
			{
				// Mark the device as having showed up in the enumerator
				exists_in_enumerator[device_id] = true;
			}
			// New controller connected case
			else
			{
				int device_id_ = findFirstClosedDeviceId();

				if (device_id_ != -1)
				{
					// Fetch the controller from it's existing controller slot
					DeviceViewPtr availableDeviceView = getDeviceViewPtr(device_id_);

					// Attempt to open the device
					if (availableDeviceView->open(enumerator))
					{
						MIKAN_LOG_INFO("DeviceTypeManager::update_connected_devices") <<
							"Device device_id " << device_id_ << " (" << enumerator->getDevicePath() << ") opened";

						// Mark the device as having showed up in the enumerator
						exists_in_enumerator[device_id_] = true;

						// Send notification to clients that a new device was added
						bSendControllerUpdatedNotification = true;
					}
					else
					{
						MIKAN_LOG_ERROR("DeviceTypeManager::update_connected_devices") <<
							"Device device_id " << device_id_ << " (" << enumerator->getDevicePath() << ") failed to open!";
					}
				}
				else
				{
					MIKAN_LOG_ERROR("DeviceTypeManager::update_connected_devices") <<
						"Can't connect any more new devices. Too many open device.";
					break;
				}
			}

			enumerator->next();
		}

		freeDeviceEnumerator(enumerator);
	}

	// Step 2
	// Close any device that is open and wasn't found in the enumerator
	for (int device_id = 0; device_id < maxDeviceCount; ++device_id)
	{
		DeviceViewPtr existingDevice = getDeviceViewPtr(device_id);

		// This probably shouldn't happen very often (at all?) as polling should catch
		// disconnected devices first.
		if (existingDevice->getIsOpen() && !exists_in_enumerator[device_id])
		{
			MIKAN_LOG_WARNING("DeviceTypeManager::update_connected_devices") << "Closing device "
				<< device_id << " since it's no longer in the device list.";
			existingDevice->close();
			bSendControllerUpdatedNotification = true;
		}
	}
}

int DeviceManager::findOpenDeviceId(const DeviceEnumerator* enumerator)
{
	int result_device_id = -1;

	for (int device_id = 0; device_id < getMaxDevices(); ++device_id)
	{
		DeviceViewPtr device = getDeviceViewPtr(device_id);

		if (device && device->matchesDeviceEnumerator(enumerator))
		{
			result_device_id = device_id;
			break;
		}
	}

	return result_device_id;
}

int DeviceManager::findFirstClosedDeviceId()
{
	int result_device_id = -1;
	for (int device_id = 0; device_id < getMaxDevices(); ++device_id)
	{
		DeviceViewPtr device = getDeviceViewPtr(device_id);

		if (device && !device->getIsOpen())
		{
			result_device_id = device_id;
			break;
		}
	}
	return result_device_id;
}

DeviceViewPtr DeviceManager::getDeviceViewPtr(int device_id)
{
	assert(m_deviceViews != nullptr);

	return m_deviceViews[device_id];
}