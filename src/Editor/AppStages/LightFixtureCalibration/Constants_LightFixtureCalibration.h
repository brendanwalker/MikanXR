#pragma once

#include <string>

enum class eLightFixtureCalibrationMenuState : int
{
	INVALID = -1,

	inactive,
	pendingVideoStartStreamRequest,
	failedVideoStartStreamRequest,
	verifyInitialCameraSetup,
	capturePosition1,
	moveCamera,
	capturePosition2,
	verifyTriangulatedPosition,
	calibrationComplete,

	COUNT
};
extern const std::string* k_LightFixtureCalibrationMenuStateStrings;
