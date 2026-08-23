#include "Constants_DepthMeshCapture.h"

// Not named k_MenuStateStrings: the unity build concatenates this file with
// the other stages' Constants_*.cpp, so file-statics still collide by name.
static const std::string k_DepthMeshMenuStateStrings[(int)eDepthMeshCaptureMenuState::COUNT]= {
	"inactive",
	"pendingVideoStartStreamRequest",
	"failedVideoStartStreamRequest",
	"verifyCameraSetup",
	"runningInference",
	"failedInference",
	"verifyMesh",
	"captureComplete",
};

const std::string* k_DepthMeshCaptureMenuStateStrings= k_DepthMeshMenuStateStrings;
