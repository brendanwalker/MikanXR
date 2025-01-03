#pragma once

// -- includes -----
#include "MikanVideoSourceTypes.h"
#include "CommonVideoConfig.h"

// -- definitions -----
enum class GStreamerProtocol
{
	INVALID= -1,

	RTMP,
	RTSP,

	COUNT
};

enum class GStreamerCodec
{
	INVALID= -1,

	MJPEG,
	H264,
	H265,

	COUNT
};

class GStreamerVideoConfig : public CommonVideoConfig
{
public:
    GStreamerVideoConfig(const std::string &fnamebase = "GStreamerCameraConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

	std::string getSourcePluginString() const;
	std::string getFullURIPath() const;
	std::string getURIProtocolString() const;
	std::string getCodecString() const;

	GStreamerProtocol protocol;
	std::string address;
	std::string path;
	int port;
	GStreamerCodec codec;
	MikanMonoIntrinsics cameraIntrinsics;
};
