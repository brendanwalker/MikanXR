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

/// Stage of an in-flight capture, reported by the capture worker so the panel
/// can show which step is running. Ordered by execution order; the values feed
/// a progress fraction, so keep them in that order.
enum class eDepthMeshCapturePhase : int
{
	idle,
	loadingModel,     ///< first capture only - over a gigabyte of ONNX model
	runningInference, ///< the single forward pass, and usually the longest step
	calibratingScale, ///< marker ratio + applying the correction
	generatingMesh,   ///< grid walk and discontinuity culling
	complete,
};

/// Number of steps shown in the capture progress readout.
static constexpr int k_depthMeshCaptureStepCount= 4;

/// Where the metric scale correction applied to a capture came from.
/// MoGe-2's scale head guesses scale from image appearance, so an unusual lens
/// can be off by an integer factor; an ArUco marker of known size in the
/// captured frame gives ground truth to correct against, and the factor is
/// persisted on the camera for marker-less captures.
enum class eDepthScaleCorrectionSource : int
{
	none,           ///< no marker visible and nothing stored - depth is the model's raw guess
	storedOnCamera, ///< reusing the factor from a previous marker calibration
	arucoMarker,    ///< computed from a marker in this capture; persisted on Create Stencil
};
