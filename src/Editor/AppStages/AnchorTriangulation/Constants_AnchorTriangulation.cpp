#include "Constants_AnchorTriangulation.h"



const std::string g_AnchorTriangulationMenuStateStrings[(int)eAnchorTriangulationMenuState::COUNT] = {
	"inactive",
	"verifyInitialCameraSetup",
	"captureOrigin1",
	"captureXAxis1",
	"captureYAxis1",
	"verifyInitialPointCapture",
	"moveCamera",
	"captureOrigin2",
	"captureXAxis2",
	"captureYAxis2",
	"verifyTriangulatedPoints",
	"testCalibration",
	"failedVideoStartStreamRequest",
};
const std::string* k_AnchorTriangulationMenuStateStrings = g_AnchorTriangulationMenuStateStrings;