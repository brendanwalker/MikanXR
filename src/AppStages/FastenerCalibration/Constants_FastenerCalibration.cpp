#include "Constants_FastenerCalibration.h"



const std::string g_fastenerCalibrationMenuStateStrings[(int)eFastenerCalibrationMenuState::COUNT] = {
	"inactive",
	"verifyInitialCameraSetup",
	"captureInitialPoints",
	"verifyInitialPointCapture",
	"moveCamera",
	"captureTriangulatedPoints",
	"verifyTriangulatedPoints",
	"testCalibration",
	"failedVideoStartStreamRequest",
};
const std::string* k_fastenerCalibrationMenuStateStrings = g_fastenerCalibrationMenuStateStrings;

const std::string g_fastenerCalibrationViewpointModeStrings[(int)eFastenerCalibrationViewpointMode::COUNT] = {
	"mixedRealityViewpoint",
	"vrViewpoint"
};
extern const std::string* k_fastenerCalibrationViewpointModeStrings = g_fastenerCalibrationViewpointModeStrings;