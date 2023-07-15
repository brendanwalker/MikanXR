#include "Constants_AlignmentCalibration.h"

const std::string g_alignmentCalibrationMenuStateStrings[(int)eAlignmentCalibrationMenuState::COUNT] = {
	"inactive",
	"verifySetup",
	"capture",
	"testCalibration",
	"failedVideoStartStreamRequest"
};
const std::string* k_alignmentCalibrationMenuStateStrings = g_alignmentCalibrationMenuStateStrings;

const std::string g_alignmentCalibrationViewpointModeStrings[(int)eAlignmentCalibrationViewpointMode::COUNT] = {
	"cameraViewpoint",
	"vrViewpoint",
	"mixedRealityViewpoint"
};
extern const std::string* k_alignmentCalibrationViewpointModeStrings= g_alignmentCalibrationViewpointModeStrings;