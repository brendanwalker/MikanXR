#pragma once

// -- includes -----
#include "MikanClientTypes.h"
#include "CommonVideoConfig.h"

// -- definitions -----
class OpenCVVideoConfig : public CommonVideoConfig
{
public:
    OpenCVVideoConfig(const std::string &fnamebase = "OpenCVCameraConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

	inline int getExposure() const { return video_properties[(int)VideoPropertyType::Exposure]; }
	inline int getGain() const { return video_properties[(int)VideoPropertyType::Gain]; }

	inline void setExposure(int exposure) { video_properties[(int)VideoPropertyType::Exposure]= exposure; }
	inline void setGain(int gain) { video_properties[(int)VideoPropertyType::Gain]= gain; }

	bool flip_horizontal;
	bool flip_vertical;
	MikanMonoIntrinsics cameraIntrinsics;
};
