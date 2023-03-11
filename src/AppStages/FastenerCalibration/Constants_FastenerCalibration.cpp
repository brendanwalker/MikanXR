#include "Constants_FastenerCalibration.h"

const std::string g_fastenerCalibrationMenuStateStrings[(int)eFastenerCalibrationMenuState::COUNT] = {
	"inactive",
	"verifySetup1",
	"capture1",
	"verifySetup2",
	"capture2",
	"testCalibration",
	"failedVideoStartStreamRequest"
};
const std::string* k_fastenerCalibrationMenuStateStrings = g_fastenerCalibrationMenuStateStrings;

const std::string g_fastenerCalibrationViewpointModeStrings[(int)eFastenerCalibrationViewpointMode::COUNT] = {
	"cameraViewpoint",
	"vrViewpoint"
};
extern const std::string* k_fastenerCalibrationViewpointModeStrings = g_fastenerCalibrationViewpointModeStrings;