#pragma once

// -- includes -----
#include "DeviceEnumerator.h"

#include <vector>
#include <string>
#include <map>

//-- constants -----
#define INVALID_DEVICE_FORMAT_INDEX			-1
#define UNSPECIFIED_CAMERA_WIDTH			0xFFFFFFFF
#define UNSPECIFIED_CAMERA_HEIGHT			0xFFFFFFFF
#define UNSPECIFIED_CAMERA_FPS				0xFFFFFFFF

// -- definitions -----
struct WMFDeviceFormatInfo
{
	int device_format_index;

	unsigned int frame_size;
	unsigned int height;
	unsigned int width;

	unsigned int yuv_matrix; // MFVideoTransferMatrix Enum
	unsigned int video_lighting; // MFVideoLighting Enum
	int default_stride; // Stride is positive for top-down images, and negative for bottom-up images.
	unsigned int video_chroma_siting; //  MFVideoChromaSubsampling Enum
	
	unsigned int fixed_size_samples;
	unsigned int video_nominal_range;
	unsigned int frame_rate_numerator;
	unsigned int frame_rate_denominator;
	unsigned int pixel_aspect_ratio_horizontal;
	unsigned int pixel_aspect_ratio_vertical;
	unsigned int all_samples_independant;
	unsigned int frame_rate_range_min_numerator;
	unsigned int frame_rate_range_min_denominator;
	unsigned int sample_size;
	unsigned int video_primaries;
	unsigned int interlace_mode;
	unsigned int frame_rate_range_max_numerator;
	unsigned int frame_rate_range_max_denominator;
	
	std::wstring am_format_type_name;
	std::wstring major_type_name;
	std::wstring sub_type_name;
};

struct WMFDeviceInfo
{
	int wmfDeviceIndex;
	std::string deviceFriendlyName; // utf-8 string
	std::string deviceSymbolicLink; // can be passed in as the value of the DevicePath argument of the SetupDiOpenDeviceInterface function.
	std::string uniqueIdentifier;
	unsigned int usbVendorId;
	unsigned int usbProductId;
	std::vector<WMFDeviceFormatInfo> deviceAvailableFormats;

	WMFDeviceInfo()
		: wmfDeviceIndex(-1)
		, deviceFriendlyName()
		, deviceSymbolicLink()
		, uniqueIdentifier()
		, usbVendorId(0)
		, usbProductId(0)
		, deviceAvailableFormats()
	{ }

	int findBestDeviceFormatIndex(
		unsigned int width, unsigned int height,
		unsigned int frameRate, const char *buffer_format) const;
};

// Enumerates over valid cameras accessible via the Windows Media Foundation API
class WMFCameraEnumerator : public DeviceEnumerator
{
public:
	static class VideoCapabilitiesSet *s_supportedTrackers;	
	
	WMFCameraEnumerator();
	virtual ~WMFCameraEnumerator();

    bool isValid() const override;
    bool next() override;
	int getUsbVendorId() const override;
	int getUsbProductId() const override;
    const char* getDevicePath() const override;
	eDeviceType getDeviceType() const override;

	const char* getUniqueIdentifier() const;
	const WMFDeviceInfo *getDeviceInfo() const;
	const class VideoCapabilitiesConfig *getVideoCapabilities() const;

private:
	struct WMFDeviceList *m_wmfDeviceList;

	std::string m_currentDeviceIdentifier;
    int m_deviceEnumeratorIndex; // Index into our m_wmfDeviceList which is not the same as the WMF API's device index
};

