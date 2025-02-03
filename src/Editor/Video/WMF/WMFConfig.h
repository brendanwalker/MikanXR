#pragma once

// -- includes -----
#include "CommonVideoConfig.h"

// -- definitions -----
class WMFVideoConfig : public CommonVideoConfig
{
public:
    WMFVideoConfig(const std::string &fnamebase = "WMFCommonVideoConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

	int wmfVideoFormatIndex;
};