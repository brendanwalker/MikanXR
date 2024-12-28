#pragma once

#include <string>

enum class eDepthMeshCaptureMenuState : int
{
	INVALID= -1, 

	inactive,
	verifySetup,
	capture,
	captureFailed,
	testCapture,
	failedToStart,

	COUNT
};
extern const std::string* k_DepthMeshCaptureMenuStateStrings;

enum class eDepthMeshCaptureViewpointMode : int
{
	INVALID= -1, 

	videoSourceViewpoint,
	vrViewpoint,

	COUNT
};
extern const std::string* k_DepthMeshCaptureViewpointModeStrings;
