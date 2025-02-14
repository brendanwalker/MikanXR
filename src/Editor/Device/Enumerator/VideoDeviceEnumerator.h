#pragma once

//-- includes -----
#include "DeviceEnumerator.h"

#include <string>
#include <vector>

//-- definitions -----
enum class eVideoDeviceApi : int
{
	INVALID = -1,

	OPENCV,
	WMF,
	GSTREAMER,
};

class VideoDeviceEnumerator : public DeviceEnumerator
{
public:
    VideoDeviceEnumerator();
	~VideoDeviceEnumerator();

    bool isValid() const override;
    bool next() override;
    const char *getDevicePath() const override;
	eDeviceType getDeviceType() const override;
    
    int getUsbVendorId() const override;
	int getUsbProductId() const override;
    inline int getCameraIndex() const { return m_cameraIndex; }
	eVideoDeviceApi getVideoApi() const;
#ifdef _WIN32
	const class WMFCameraEnumerator *getWMFCameraEnumerator() const;
#endif // _WIN32
	const class OpenCVCameraEnumerator* getOpenCVCameraEnumerator() const;
	const class GStreamerCameraEnumerator* getGStreamerCameraEnumerator() const;

private:
	struct EnumeratorEntry
	{
		eVideoDeviceApi api_type;
		DeviceEnumerator* enumerator;
	};

	void allocateChildEnumerator();

	std::vector<EnumeratorEntry> m_enumerators;
	int m_enumeratorIndex;
    int m_cameraIndex;
};