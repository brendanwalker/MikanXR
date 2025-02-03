#include "Constants_CTOffsetCalibration.h"

const std::string g_CTOffsetCalibrationMenuStateStrings[(int)eCTOffsetCalibrationMenuState::COUNT] = {
	"inactive",
	"verifySetup",
	"capture",
	"reposition",
	"testCalibration",
	"failedVideoStartStreamRequest"
};
const std::string* k_CTOffsetCalibrationMenuStateStrings = g_CTOffsetCalibrationMenuStateStrings;

const std::string g_CTOffsetCalibrationViewpointModeStrings[(int)eCTOffsetCalibrationViewpointMode::COUNT] = {
	"cameraViewpoint",
	"vrViewpoint",
	"mixedRealityViewpoint"
};
extern const std::string* k_CTOffsetCalibrationViewpointModeStrings= g_CTOffsetCalibrationViewpointModeStrings;