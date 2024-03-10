#include "Constants_DepthMeshCapture.h"

const std::string g_DepthMeshCaptureMenuStateStrings[(int)eDepthMeshCaptureMenuState::COUNT] = {
	"inactive",
	"verifySetup",
	"capture",
	"captureFailed",
	"testCapture",
	"failedToStart"
};
const std::string* k_DepthMeshCaptureMenuStateStrings = g_DepthMeshCaptureMenuStateStrings;

const std::string g_DepthMeshCaptureViewpointModeStrings[(int)eDepthMeshCaptureViewpointMode::COUNT] = {
	"videoSourceViewpoint",
	"vrViewpoint"
};
extern const std::string* k_DepthMeshCaptureViewpointModeStrings= g_DepthMeshCaptureViewpointModeStrings;

