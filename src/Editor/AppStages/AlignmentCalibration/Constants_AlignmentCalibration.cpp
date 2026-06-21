#include "Constants_AlignmentCalibration.h"

const std::string g_alignmentCalibrationMenuStateStrings[(int)eAlignmentCalibrationMenuState::COUNT]= {
	"inactive", "pendingVideoStart", "verifySetup", "capture", "testCalibration", "failedVideoStartStreamRequest"};
const std::string* k_alignmentCalibrationMenuStateStrings= g_alignmentCalibrationMenuStateStrings;

const std::string g_alignmentCalibrationViewpointModeStrings[(int)eAlignmentCalibrationViewpointMode::COUNT]= {
	"Calibration", "Stage View", "XR View"};
const std::string* k_alignmentCalibrationViewpointModeStrings= g_alignmentCalibrationViewpointModeStrings;