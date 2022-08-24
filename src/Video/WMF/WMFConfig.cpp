// -- includes -----
#include "WMFConfig.h"
#include "VideoCapabilitiesConfig.h"

// -- WMF Stereo Tracker Config
WMFVideoConfig::WMFVideoConfig(const std::string &fnamebase)
    : CommonVideoConfig(fnamebase)
{
};

const configuru::Config 
WMFVideoConfig::writeToJSON()
{
	configuru::Config pt = CommonVideoConfig::writeToJSON();

    return pt;
}

void 
WMFVideoConfig::readFromJSON(const configuru::Config &pt)
{
	CommonVideoConfig::readFromJSON(pt);
}