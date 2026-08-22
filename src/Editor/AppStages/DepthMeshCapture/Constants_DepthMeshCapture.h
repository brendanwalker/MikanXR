#pragma once

#include <string>

enum class eDepthMeshCaptureMenuState : int
{
	INVALID= -1,

	inactive,
	pendingVideoStartStreamRequest,
	failedVideoStartStreamRequest,
	// Live video, waiting for the operator to frame the shot and hit Capture.
	verifyCameraSetup,
	// Inference is running. About a second on GPU, but still long enough that
	// this state exists to tell the operator the app has not hung.
	runningInference,
	failedInference,
	// Mesh generated; showing the depth overlay and the mesh statistics so it
	// can be judged before a stencil is created from it.
	verifyMesh,
	captureComplete,

	COUNT
};
extern const std::string* k_DepthMeshCaptureMenuStateStrings;
