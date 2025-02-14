#pragma once

// -- includes -----
#include "DeviceEnumerator.h"
#include "VideoFwd.h"

#include <string>
#include <vector>

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
	const char* getDevicePath() const override;
	eDeviceType getDeviceType() const override;

	inline int getDeviceIndex() const { return m_cameraIndex; }

private:
	void rebuildCameraURIList();

	std::vector<std::string> m_cameraURIList;
	int m_cameraIndex;
};


