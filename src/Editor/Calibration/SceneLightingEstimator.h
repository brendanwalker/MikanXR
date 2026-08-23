#pragma once

//-- includes -----
#include "MarigoldInference.h"
#include "MoGeInference.h"
#include "SphericalHarmonics.h"

#include <glm/glm.hpp>

#include <functional>
#include <string>

/// Stage of an in-flight estimate, in execution order. Only the diffusion
/// decomposition reports sub-steps; the rest are single opaque calls.
enum class eSceneLightingEstimatePhase : int
{
	idle,
	loadingModels,      ///< several GB of ONNX across the two models
	decomposingShading, ///< Marigold IID: VAE encode, the DDIM loop, the decodes
	estimatingGeometry, ///< MoGe-2 forward pass for the surface normals
	fittingLighting,    ///< the order-2 SH least squares
	complete,
};

/// Number of steps shown in the estimate progress readout.
static constexpr int k_sceneLightingEstimateStepCount= 4;

//-- types -----
/// Recovers the scene's low-frequency lighting from one captured frame.
///
/// Pipeline: undistorted frame -> Marigold IID (diffuse shading) + MoGe-2
/// (surface normals) -> per-pixel masking -> order-2 SH least squares ->
/// rotate camera space into Mikan world space using the tracked camera pose.
///
/// The normals moved from Marigold's 3.4GB normals UNet to MoGe-2's
/// directly-predicted normal head after the swap was measured to preserve the
/// recovered lighting within Marigold's own seed-to-seed spread; Marigold
/// remains the source of the diffuse shading, which has no feed-forward
/// replacement. See docs/reference/scene-lighting.md.
///
/// The rotation step is the reason this lives inside Mikan: the models return
/// camera-space normals, so without the tracked pose the recovered environment
/// has no defined orientation relative to the Unreal scene.
class SceneLightingEstimator
{
public:
	struct Config
	{
		/// Marigold model directory (vae/unet_iid/scheduler artifacts).
		std::string modelDirectory;

		/// MoGe-2 model directory (model.onnx from tools/fetch_moge2_onnx.py).
		std::string mogeModelDirectory;

		int processingResolution= 768;
		bool preferGpu= true;

		/// Seed for the initial latent noise. The denoise loop starts from
		/// noise, so this changes the result; fixed by default so a capture is
		/// reproducible.
		unsigned int seed= 1234u;

		/// Penalty on the l=2 SH band. See SHLightingSolver::solve.
		float bandRidge= 0.1f;

		/// Pixels at or above this shading value are treated as clipped and
		/// dropped - a saturated pixel says "at least this bright" rather than
		/// "this bright", which biases the fit.
		float saturationThreshold= 0.99f;

		/// Pixels at or below this are fully occluded and carry no lighting
		/// information, only visibility.
		float blackThreshold= 1e-4f;
	};

	struct Result
	{
		/// The recovered environment, in Mikan world space.
		SHLightingEnvironment environment;

		/// The same environment before the camera-pose rotation, kept for
		/// diagnostics and for the in-editor preview.
		SHLightingEnvironment cameraSpaceEnvironment;

		/// l=1 over l=0 band energy. Low means the scene is essentially
		/// ambient and the dominant direction is not meaningful. Surface this;
		/// do not silently present a flat estimate as a confident one.
		float directionality= 0.f;

		/// Suggested key light direction in world space. Only trustworthy when
		/// directionality is high.
		glm::vec3 keyLightDirection= glm::vec3(0.f, 0.f, 1.f);

		/// Fraction of the sphere where reconstructed radiance is negative.
		/// Non-zero is expected for directional scenes - order-2 SH cannot
		/// represent a sharp light. Diagnostic only.
		float negativeSolidAngleFraction= 0.f;

		int sampleCount= 0;
		int rejectedPixelCount= 0;

		/// Raw model outputs, retained so the editor can show them.
		MarigoldInference::Result modelOutputs;
	};

	/// Optional hooks so a UI can follow a long estimate and abandon it.
	struct Progress
	{
		/// Called on the estimating thread as work completes. completedUnits /
		/// totalUnits are only meaningful within the reported phase, and only
		/// the diffusion phase reports more than one unit.
		std::function<void(eSceneLightingEstimatePhase phase, int completedUnits, int totalUnits)> onProgress;

		/// Polled at each checkpoint; returning true abandons the estimate,
		/// which then fails like any other error. Cancelling is the caller's
		/// own flag to distinguish - see AppStage_SceneLightingCapture.
		std::function<bool()> isCancelled;
	};

	bool startup(const Config& config);
	void shutdown();

	/// Asks an in-flight estimate() to give up as soon as ONNX Runtime notices.
	/// Safe to call from another thread.
	void requestCancel();

	bool getIsInitialized() const { return m_inference.getIsInitialized() && m_geometryInference.getIsInitialized(); }
	/// The IID diffusion pipeline dominates the cost, so its execution provider
	/// is the one worth warning about.
	const char* getActiveExecutionProvider() const { return m_inference.getActiveExecutionProvider(); }

	/// bgrImage should already be undistorted (see VideoFrameDistortionView).
	/// cameraToWorldRotation is the tracked camera's orientation at capture.
	/// fovXDegrees is the calibrated horizontal FOV of the undistorted frame;
	/// the normals the fit consumes are FOV-independent (measured 0.0 degrees
	/// across a 45-70 sweep), but MoGe-2's metric depth recovery needs it, so
	/// pass the real value where one exists.
	bool estimate(const cv::Mat& bgrImage, const glm::mat3& cameraToWorldRotation, float fovXDegrees, Result& outResult,
				  const Progress& progress= {});

	/// What renderReconstructionImage should draw.
	enum class eReconstructionView : int
	{
		lighting,     ///< the recovered environment at the model's own normals
		relit,        ///< the same, modulated by the recovered albedo
		modelShading, ///< the shading the fit solved against, for an A/B
	};

	/// Builds a display-encoded BGR image (CV_8UC3) from a completed estimate,
	/// so what the recovered environment does and does not explain can be seen
	/// against the plate. Returns empty if the result carries no model outputs.
	///
	/// Note what this cannot show: an environment probe has no visibility term,
	/// so cast shadows are absent from the reconstruction by construction. The
	/// difference against modelShading is dominated by exactly those regions -
	/// that is expected, not a bad estimate. See docs/reference/scene-lighting.md.
	static cv::Mat renderReconstructionImage(const Result& result, eReconstructionView view);

	/// Fit only, for tests and for re-fitting cached model output without
	/// paying for inference again.
	bool fitFromModelOutputs(const MarigoldInference::Result& modelOutputs, const glm::mat3& cameraToWorldRotation,
							 Result& outResult) const;

private:
	Config m_config;
	MarigoldInference m_inference;
	MoGeInference m_geometryInference;
};
