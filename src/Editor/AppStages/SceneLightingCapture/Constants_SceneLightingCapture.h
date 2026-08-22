#pragma once

#include <string>

enum class eSceneLightingCaptureMenuState : int
{
	INVALID= -1,

	inactive,
	pendingVideoStartStreamRequest,
	failedVideoStartStreamRequest,
	// Live video, waiting for the operator to frame the shot and hit Capture.
	verifyCameraSetup,
	// Inference is running. It takes seconds, not milliseconds, so this state
	// exists to tell the operator the app has not hung.
	runningInference,
	failedInference,
	// Estimate recovered; showing the lit-sphere preview and the confidence
	// numbers so it can be judged before being committed to the probe.
	verifyEstimate,
	captureComplete,

	COUNT
};
extern const std::string* k_SceneLightingCaptureMenuStateStrings;
