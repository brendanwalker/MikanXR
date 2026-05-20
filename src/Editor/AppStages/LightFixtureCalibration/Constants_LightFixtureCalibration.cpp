#include "Constants_LightFixtureCalibration.h"

static const std::string k_MenuStateStrings[(int)eLightFixtureCalibrationMenuState::COUNT] = {
	"inactive",
	"pendingVideoStartStreamRequest",
	"failedVideoStartStreamRequest",
	"verifyInitialCameraSetup",
	"capturePosition1",
	"moveCamera",
	"capturePosition2",
	"verifyTriangulatedPosition",
	"calibrationComplete",
};

const std::string* k_LightFixtureCalibrationMenuStateStrings = k_MenuStateStrings;
