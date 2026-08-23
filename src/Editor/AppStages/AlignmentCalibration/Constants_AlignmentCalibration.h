#pragma once

#include <string>

enum class eAlignmentCalibrationMenuState : int
{
	INVALID= -1,

	inactive= 0,
	pendingVideoStart= 1,
	verifySetup= 2,
	capture= 3,
	testCalibration= 4,
	failedVideoStartStreamRequest= 5,

	COUNT
};
extern const std::string* k_alignmentCalibrationMenuStateStrings;

enum class eAlignmentCalibrationViewpointMode : int
{
	INVALID= -1,

	calibration,
	stageView,
	xrView,

	COUNT
};
extern const std::string* k_alignmentCalibrationViewpointModeStrings;

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100
