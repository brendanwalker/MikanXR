#pragma once

#include <string>

enum class eFastenerCalibrationMenuState : int
{
	INVALID = -1,

	inactive,
	verifySetup1,
	capture1,
	verifySetup2,
	capture2,
	testCalibration,
	failedVideoStartStreamRequest,

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