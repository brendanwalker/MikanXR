#pragma once

#include <string>

enum class eDepthMeshCaptureMenuState : int
{
	INVALID= -1, 

	inactive = 0,
	verifySetup = 1,
	capture = 2,
	testCalibration = 3,
	failedVideoStartStreamRequest = 4,

	COUNT
};
extern const std::string* k_DepthMeshCaptureMenuStateStrings;

enum class eDepthMeshCaptureViewpointMode : int
{
	INVALID= -1, 

	cameraViewpoint,
	vrViewpoint,
	mixedRealityViewpoint,

	COUNT
};
extern const std::string* k_DepthMeshCaptureViewpointModeStrings;

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 
