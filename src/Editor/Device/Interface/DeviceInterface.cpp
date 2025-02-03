#include "DeviceInterface.h"

const std::string g_deviceTypeStrings[(int)eDeviceType::COUNT] = {
	"INVALID",
	"HMD",
	"VRTracker",
	"Controller",
	"MonoVideoSource",
	"StereoVideoSource" 
};
const std::string* k_deviceTypeStrings= g_deviceTypeStrings;