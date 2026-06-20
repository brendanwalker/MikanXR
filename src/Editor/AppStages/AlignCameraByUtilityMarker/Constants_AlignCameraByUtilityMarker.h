#pragma once

#include <string>

enum class eAlignCameraByUtilityMarkerMenuState : int
{
	INVALID= -1,

	inactive= 0,
	selectSourceCamera= 1,
	pendingVideoStart= 2,
	verifySetup= 3,
	capturing= 4,
	testCalibration= 5,
	failedVideoStart= 6,

	COUNT
};

#define ALIGN_CAMERA_BY_UTILITY_MARKER_SAMPLE_COUNT 12
