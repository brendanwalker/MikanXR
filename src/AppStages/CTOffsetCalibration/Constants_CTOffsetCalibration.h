#pragma once

#include <string>

enum class eCTOffsetCalibrationMenuState : int
{
	INVALID= -1, 

	inactive = 0,
	verifySetup = 1,
	capture = 2,
	reposition = 3,
	testCalibration = 4,
	failedVideoStartStreamRequest = 5,

	COUNT
};
extern const std::string* k_CTOffsetCalibrationMenuStateStrings;

enum class eCTOffsetCalibrationViewpointMode : int
{
	INVALID= -1, 

	cameraViewpoint,
	vrViewpoint,
	mixedRealityViewpoint,

	COUNT
};
extern const std::string* k_CTOffsetCalibrationViewpointModeStrings;

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 
