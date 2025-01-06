#pragma once

// -- includes -----
#include "MikanVideoSourceTypes.h"
#include "CommonVideoConfig.h"

// -- definitions -----
enum class eGStreamerProtocol
{
	INVALID= -1,

	RTMP,
	RTSP,

	COUNT
};
extern const std::string* k_GStreamerProtocolStrings;

class GStreamerVideoConfig : public CommonVideoConfig
{
public:
    GStreamerVideoConfig(const std::string &fnamebase = "GStreamerCameraConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

	bool applyDevicePath(const std::string& devicePath);

	std::string getSourcePluginString() const;
	std::string getFullURIPath() const;
	std::string getURIProtocolString() const;

	eGStreamerProtocol protocol;
	std::string address;
	std::string path;
	int port;
	MikanMonoIntrinsics cameraIntrinsics;
};
