#pragma once

// -- includes -----
#include "DeviceInterface.h"

// -- definitions -----
class DeviceEnumerator
{
public:
    DeviceEnumerator() {}
    virtual ~DeviceEnumerator() {}

    virtual bool isValid() const = 0;
    virtual bool next() = 0;
	virtual int getUsbVendorId() const = 0;
	virtual int getUsbProductId() const = 0;
    virtual const char* getDevicePath() const = 0;   
    virtual eDeviceType getDeviceType() const = 0;
};