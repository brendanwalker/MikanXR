//-- includes -----
#include "DeviceView.h"
#include "Logger.h"

//-- public implementation -----
DeviceView::DeviceView(
	const int device_id)
	: m_deviceID(device_id)
{
}

DeviceView::~DeviceView()
{
}

bool DeviceView::open(const DeviceEnumerator* enumerator)
{
	// Attempt to allocate the device 
	bool bSuccess = allocateDeviceInterface(enumerator);

	// Attempt to open the device
	if (bSuccess)
	{
		bSuccess = getDevice()->open(enumerator);
	}

	return bSuccess;
}

bool DeviceView::getIsOpen() const
{
	IDeviceInterface* device = getDevice();

	return (device != nullptr) ? device->getIsOpen() : false;
}

void DeviceView::close()
{
	if (getIsOpen())
	{
		getDevice()->close();
		freeDeviceInterface();
	}
}

bool DeviceView::matchesDeviceEnumerator(const DeviceEnumerator* enumerator) const
{
	return getIsOpen() && getDevice()->matchesDeviceEnumerator(enumerator);
}