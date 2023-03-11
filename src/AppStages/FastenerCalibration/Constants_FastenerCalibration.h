#pragma once

#include <string>

enum class eFastenerCalibrationMenuState : int
{
	INVALID = -1,

	inactive = 0,
	verifySetup1 = 1,
	capture1 = 3,
	verifySetup2 = 4,
	capture2 = 5,
	testCalibration = 6,
	failedVideoStartStreamRequest = 7,

	COUNT
};
extern const std::string* k_fastenerCalibrationMenuStateStrings;

enum class eFastenerCalibrationViewpointMode : int
{
	INVALID = -1,

	cameraViewpoint,
	vrViewpoint,

	COUNT
};
extern const std::string* k_fastenerCalibrationViewpointModeStrings;