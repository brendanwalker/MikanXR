#include "Constants_VRTrackingRecenter.h"

const std::string g_VRTrackingRecenterMenuStateStrings[(int)eVRTrackingRecenterMenuState::COUNT] = {
	"inactive",
	"verifySetup",
	"capture",
	"testCalibration",
	"failedVideoStartStreamRequest"
};
const std::string* k_VRTrackingRecenterMenuStateStrings = g_VRTrackingRecenterMenuStateStrings;