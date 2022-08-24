#pragma once

// -- includes -----
#include "DeviceEnumerator.h"
#include <string>
#include <vector>

// -- definitions -----
// Enumerates over valid SteamVR trackers (up to 16) via the SteamVR API
class SteamVRDeviceEnumerator : public DeviceEnumerator
{
public:

	SteamVRDeviceEnumerator();

	bool isValid() const override;
	bool next() override;
	int getUsbVendorId() const override { return m_vendorId; }
	int getUsbProductId() const override { return m_productId; }
	const char* getDevicePath() const override
	{
		return m_devicePath.c_str();
	}
	eDeviceType getDeviceType() const override { return m_deviceType; }

	int getSteamVRDeviceID() const;

private:
	bool tryFetchVRTrackerProperties();

	std::vector<int> m_steamVRDeviceIds; 
	std::string m_devicePath;
	eDeviceType m_deviceType;
	int m_vendorId;
	int m_productId;
	int m_enumeratorIndex;
};



