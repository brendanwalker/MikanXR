#pragma once

#include <vector>

class WMFDeviceList
{
public:
	WMFDeviceList() = default;

	size_t getDeviceCount() const { return m_deviceList.size(); }
	const struct WMFDeviceInfo* getDeviceByIndex(size_t index) const;

	bool rebuild();

protected:
	std::vector<struct WMFDeviceInfo> m_deviceList;
};
