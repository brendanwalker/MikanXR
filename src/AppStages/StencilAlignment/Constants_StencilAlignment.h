#pragma once

#include <string>

enum class eStencilAlignmentMenuState : int
{
	INVALID = -1,

	inactive,
	verifyInitialCameraSetup,
	captureOriginPixel,
	captureOriginVertex,
	captureXAxisPixel,
	captureXAxisVertex,
	captureYAxisPixel,
	captureYAxisVertex,
	captureZAxisPixel,
	captureZAxisVertex,
	verifyPointsCapture,
	testCalibration,
	failedVideoStartStreamRequest,

	COUNT
};
extern const std::string* k_StencilAlignmentMenuStateStrings;