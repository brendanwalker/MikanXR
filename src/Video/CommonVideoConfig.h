#pragma once

// -- includes -----
#include "VideoSourceInterface.h"
#include "CommonConfig.h"
#include "MikanClientTypes.h"

// -- definitions -----
class CommonVideoConfig : public CommonConfig
{
public:
    CommonVideoConfig(const std::string &fnamebase = "CommonVideoConfig");
    
    virtual const configuru::Config writeToJSON();
    virtual void readFromJSON(const configuru::Config &pt);

    bool is_valid;
    long max_poll_failure_count;

	std::string current_mode;
	int video_properties[(int)VideoPropertyType::COUNT];

    MikanQuatd orientationOffset;
    MikanVector3d positionOffset;
};