#pragma once

//-- includes -----
#include "DeviceInterface.h"
#include <chrono>

// -- declarations -----
class DeviceView
{
public:
	DeviceView(const int device_id);
	virtual ~DeviceView();

	virtual bool open(const class DeviceEnumerator* enumerator);
	virtual void close();

	bool matchesDeviceEnumerator(const class DeviceEnumerator* enumerator) const;

	// getters
	inline int getDeviceID() const
	{
		return m_deviceID;
	}

	virtual IDeviceInterface* getDevice() const = 0;

	// Used for when you have to get device specific data
	template <class t_device_subclass>
	inline const t_device_subclass* castCheckedConst() const
	{
		IDeviceInterface* device = getDevice();
		assert(device != nullptr);
		assert(device->getDeviceType() == t_device_subclass::getDeviceTypeStatic());
		const t_device_subclass* controller = static_cast<const t_device_subclass*>(device);

		return controller;
	}

	template <class t_device_subclass>
	inline t_device_subclass* castChecked()
	{
		IDeviceInterface* device = getDevice();
		assert(device != nullptr);
		assert(device->getDeviceType() == t_device_subclass::getDeviceTypeStatic());
		t_device_subclass* controller = static_cast<t_device_subclass*>(device);

		return controller;
	}

	// Returns true if device opened successfully
	bool getIsOpen() const;

	inline std::chrono::time_point<std::chrono::high_resolution_clock> getLastNewDataTimestamp() const
	{
		return m_lastNewDataTimestamp;
	}

protected:
	virtual bool allocateDeviceInterface(const class DeviceEnumerator* enumerator) = 0;
	virtual void freeDeviceInterface() = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastNewDataTimestamp;

private:
	int m_deviceID;
};
