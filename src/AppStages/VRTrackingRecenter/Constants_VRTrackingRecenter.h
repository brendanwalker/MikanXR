#pragma once

#include <string>

enum class eVRTrackingRecenterMenuState : int
{
	INVALID= -1, 

	inactive = 0,
	verifySetup = 1,
	capture = 2,
	testCalibration = 3,
	failedVideoStartStreamRequest = 4,

	COUNT
};
extern const std::string* k_VRTrackingRecenterMenuStateStrings;

#define DESIRED_MARKER_SAMPLE_COUNT 10
