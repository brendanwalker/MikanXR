#pragma once

//-- includes -----
#include "DeviceEnumerator.h"

#include <string>
#include <vector>

//-- definitions -----
enum class eVRTrackerDeviceApi : int
{
	INVALID = -1,
	STEAMVR
};

class VRDeviceEnumerator : public DeviceEnumerator
{
public:
	VRDeviceEnumerator();
	~VRDeviceEnumerator();

	bool isValid() const override;
	bool next() override;
	const char* getDevicePath() const override;
	eDeviceType getDeviceType() const override;

	int getUsbVendorId() const override;
	int getUsbProductId() const override;
	eVRTrackerDeviceApi getVRTrackerApi() const;
	const class SteamVRDeviceEnumerator* getSteamVRTrackerEnumerator() const;

private:
	struct EnumeratorEntry
	{
		eVRTrackerDeviceApi api_type;
		DeviceEnumerator* enumerator;
	};

	void allocateChildEnumerator();

	std::vector<EnumeratorEntry> m_enumerators;
	int m_enumeratorIndex;
};