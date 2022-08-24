#pragma once

// -- includes -----
#include "DeviceEnumerator.h"
#include <string>

// -- definitions -----
// Enumerates over valid cameras ports (up to 16) via the OpenCV API
class OpenCVCameraEnumerator : public DeviceEnumerator
{
public:

	OpenCVCameraEnumerator();
	virtual ~OpenCVCameraEnumerator();

	bool isValid() const override;
	bool next() override;
	int getUsbVendorId() const override { return -1; }
	int getUsbProductId() const override { return -1; }
	const char* getDevicePath() const override 
	{ return m_devicePath.c_str(); }
	eDeviceType getDeviceType() const override;

	inline int getDeviceIndex() const { return m_deviceIndex; }
	const class VideoCapabilitiesConfig* getVideoCapabilities() const
	{ return m_currentDeviceCapabilities; }

private:
	bool tryFetchDeviceCapabilities();

	class VideoCapabilitiesConfig* m_currentDeviceCapabilities;
	std::string m_devicePath;
	int m_deviceIndex;
};


