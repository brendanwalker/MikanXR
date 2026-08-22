#include "Constants_SceneLightingCapture.h"

static const std::string k_MenuStateStrings[(int)eSceneLightingCaptureMenuState::COUNT]= {
	"inactive",
	"pendingVideoStartStreamRequest",
	"failedVideoStartStreamRequest",
	"verifyCameraSetup",
	"runningInference",
	"failedInference",
	"verifyEstimate",
	"captureComplete",
};

const std::string* k_SceneLightingCaptureMenuStateStrings= k_MenuStateStrings;
