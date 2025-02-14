#include "Constants_StencilAlignment.h"



const std::string g_StencilAlignmentMenuStateStrings[(int)eStencilAlignmentMenuState::COUNT] = {
	"inactive",
	"verifyInitialCameraSetup",
	"captureOriginPixel",
	"captureOriginVertex",
	"captureXAxisPixel",
	"captureXAxisVertex",
	"captureYAxisPixel",
	"captureYAxisVertex",
	"captureZAxisPixel",
	"captureZAxisVertex",
	"verifyPointsCapture",
	"testCalibration",
	"failedVideoStartStreamRequest",
};
const std::string* k_StencilAlignmentMenuStateStrings = g_StencilAlignmentMenuStateStrings;