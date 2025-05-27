#pragma once

// -- includes -----
#include "VideoSourceInterface.h"
#include "CommonConfig.h"
#include "MikanMathTypes.h"
#include "MikanTypeFwd.h"

// -- definitions -----
class CommonVideoConfig : public CommonConfig
{
public:
    CommonVideoConfig(const std::string &fnamebase = "CommonVideoConfig");
    
    virtual configuru::Config writeToJSON();
    virtual void readFromJSON(const configuru::Config &pt);

	MikanVideoSourceID video_source_id;

    bool is_valid;
    long max_poll_failure_count;

	std::string current_mode;
	int video_properties[(int)VideoPropertyType::COUNT];

    MikanQuatd orientationOffset;
    MikanVector3d positionOffset;
};