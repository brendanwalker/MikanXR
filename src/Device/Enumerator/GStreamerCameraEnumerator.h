#pragma once

// -- includes -----
#include "DeviceEnumerator.h"
#include "VideoFwd.h"

#include <string>

// -- definitions -----
// Enumerates over valid camera streams via the GStreamer API
class GStreamerCameraEnumerator : public DeviceEnumerator
{
public:

	GStreamerCameraEnumerator();

	bool isValid() const override;
	bool next() override;
	int getUsbVendorId() const override { return -1; }
	int getUsbProductId() const override { return -1; }
	const char* getDevicePath() const override 
	{ return m_devicePath.c_str(); }
	eDeviceType getDeviceType() const override;

	inline int getDeviceIndex() const { return m_deviceIndex; }
	VideoCapabilitiesConfigConstPtr getVideoCapabilities() const
	{ return m_currentDeviceCapabilities; }

private:
	bool tryFetchDeviceCapabilities();

	VideoCapabilitiesConfigPtr m_currentDeviceCapabilities;
	std::string m_devicePath;
	int m_deviceIndex;
};


