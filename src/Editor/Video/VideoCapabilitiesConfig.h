#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "DeviceInterface.h"
#include "MikanVideoSourceTypes.h"
#include "VideoFwd.h"

// -- constants -----
#define CAMERA_BUFFER_FORMAT_MJPG			"MJPG"
#define CAMERA_BUFFER_FORMAT_YUY2			"YUY2"
#define CAMERA_BUFFER_FORMAT_NV12			"NV12"
#define CAMERA_BUFFER_FORMAT_BEYER			"BEYER"

// -- definitions -----
struct VideoFrameSectionInfo
{
    const configuru::Config writeToJSON() const;
    void readFromJSON(const configuru::Config &pt);

	int x;
	int y;
};

struct VideoModeConfig
{
    const configuru::Config writeToJSON() const;
    void readFromJSON(const configuru::Config &pt);

	std::string modeName;

	float frameRate;
	bool isFrameMirrored;
	bool isBufferMirrored;
	int bufferPixelWidth;
	int bufferPixelHeight;
	std::string bufferFormat;
	std::vector<VideoFrameSectionInfo> frameSections;
	
	MikanVideoSourceIntrinsics intrinsics;
};
using VideoModeConfigPtr = std::shared_ptr<VideoModeConfig>;
using VideoModeConfigConstPtr = std::shared_ptr<const VideoModeConfig>;

class VideoCapabilitiesConfig : public CommonConfig
{
public:
    VideoCapabilitiesConfig(const std::string &fnamebase = "CommonTrackerConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

	int findVideoModeIndex(const std::string& mode_name) const;
	const VideoModeConfig* findVideoMode(const std::string &mode_name) const;
	const VideoModeConfig* findMostCompatibleVideoMode(
		int width, int height, float fps, const std::string& bufferFormat) const;
	void getAvailableVideoModes(std::vector<std::string> &out_mode_names) const;

	std::string friendlyName;
	int usbProductId;
	int usbVendorId;
	eDeviceType deviceType;
	std::vector<VideoModeConfig> supportedModes;
};

class VideoCapabilitiesSet
{
public:
	bool reloadSupportedVideoCapabilities();
	VideoCapabilitiesConfigConstPtr getVideoSourceCapabilities(unsigned short vendor_id, unsigned short product_id) const;

private:
	std::vector<VideoCapabilitiesConfigConstPtr> m_supportedTrackers;
};
