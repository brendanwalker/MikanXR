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

/// What the verification overlay draws over the captured frame once an
/// estimate lands. The first three are full-frame images built from the model
/// outputs, so they can be flipped between to compare directly.
enum class eLightingPreviewMode : int
{
	/// The recovered environment evaluated at the scene's own normals - what
	/// the estimate says the incident lighting across this shot is.
	recoveredLighting,
	/// The same, modulated by the recovered albedo: the plate re-rendered lit
	/// only by the recovered environment.
	relitScene,
	/// Marigold's diffuse shading, which is the target the fit solved against.
	/// The A/B partner for recoveredLighting - what the fit could not explain
	/// shows up as the difference.
	modelShading,
	/// A sphere lit by the recovered environment, in camera space.
	litSphere,

	COUNT
};
