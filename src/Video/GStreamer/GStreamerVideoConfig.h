#pragma once

// -- includes -----
#include "MikanGStreamerConstants.h"
#include "MikanVideoSourceTypes.h"
#include "CommonVideoConfig.h"

// -- definitions -----

class GStreamerVideoConfig : public CommonVideoConfig
{
public:
    GStreamerVideoConfig(const std::string &fnamebase = "GStreamerCameraConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

	bool applyDevicePath(const std::string& devicePath);

	eGStreamerProtocol protocol;
	std::string address;
	std::string path;
	int port;
	MikanMonoIntrinsics cameraIntrinsics;
};
