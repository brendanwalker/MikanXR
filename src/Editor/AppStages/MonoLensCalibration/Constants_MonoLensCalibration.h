#pragma once

enum class eMonoLensCalibrationMenuState : int
{
	INVALID = -1,

	inactive = 0,
	capture = 1,
	processingCalibration = 2,
	testCalibration = 3,
	failedCalibration = 4,
	failedVideoStartStreamRequest = 5,

	COUNT
};

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 
