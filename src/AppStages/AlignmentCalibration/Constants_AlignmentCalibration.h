#pragma once

#include <string>

enum class eAlignmentCalibrationMenuState : int
{
	INVALID= -1, 

	inactive = 0,
	verifySetup = 1,
	capture = 2,
	testCalibration = 3,
	failedVideoStartStreamRequest = 4,

	COUNT
};
extern const std::string* k_alignmentCalibrationMenuStateStrings;

enum class eAlignmentCalibrationViewpointMode : int
{
	INVALID= -1, 

	cameraViewpoint,
	vrViewpoint,
	mixedRealityViewpoint,

	COUNT
};
extern const std::string* k_alignmentCalibrationViewpointModeStrings;

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 
