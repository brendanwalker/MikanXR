#pragma once

#include <string>

enum class eAlignCameraByOriginMarkerMenuState : int
{
	INVALID = -1,

	inactive = 0,
	pendingVideoStart = 1,
	verifySetup = 2,
	capturing = 3,
	testCalibration = 4,
	failedVideoStart = 5,

	COUNT
};

#define ALIGN_CAMERA_BY_ORIGIN_MARKER_SAMPLE_COUNT 12
