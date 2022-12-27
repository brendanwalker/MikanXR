#pragma once

enum class eAlignmentCalibrationMenuState : int
{
	inactive = 0,
	verifySetup = 1,
	capture = 2,
	testCalibration = 3,
	failedVideoStartStreamRequest = 4,
};

enum class eAlignmentCalibrationViewpointMode : int
{
	cameraViewpoint,
	vrViewpoint,
	mixedRealityViewpoint,

	COUNT
};

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 
