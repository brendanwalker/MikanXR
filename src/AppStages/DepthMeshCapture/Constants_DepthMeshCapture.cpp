#include "Constants_DepthMeshCapture.h"

const std::string g_DepthMeshCaptureMenuStateStrings[(int)eDepthMeshCaptureMenuState::COUNT] = {
	"inactive",
	"verifySetup",
	"capture",
	"testCapture",
	"failedVideoStartStreamRequest"
};
const std::string* k_DepthMeshCaptureMenuStateStrings = g_DepthMeshCaptureMenuStateStrings;

const std::string g_DepthMeshCaptureViewpointModeStrings[(int)eDepthMeshCaptureViewpointMode::COUNT] = {
	"cameraViewpoint",
	"vrViewpoint",
	"mixedRealityViewpoint"
};
extern const std::string* k_DepthMeshCaptureViewpointModeStrings= g_DepthMeshCaptureViewpointModeStrings;