#pragma once

#include <string>

enum class eAnchorTriangulationMenuState : int
{
	INVALID = -1,

	inactive,
	verifyInitialCameraSetup,
	captureOrigin1,
	captureXAxis1,
	captureYAxis1,
	verifyInitialPointCapture,
	moveCamera,
	captureOrigin2,
	captureXAxis2,
	captureYAxis2,
	verifyTriangulatedPoints,
	testCalibration,
	failedVideoStartStreamRequest,

	COUNT
};
extern const std::string* k_AnchorTriangulationMenuStateStrings;