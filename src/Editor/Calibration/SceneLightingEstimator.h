#pragma once

//-- includes -----
#include "MarigoldInference.h"
#include "SphericalHarmonics.h"

#include <glm/glm.hpp>

#include <string>

//-- types -----
/// Recovers the scene's low-frequency lighting from one captured frame.
///
/// Pipeline: undistorted frame -> Marigold (normals + diffuse shading) ->
/// per-pixel masking -> order-2 SH least squares -> rotate camera space into
/// Mikan world space using the tracked camera pose.
///
/// The rotation step is the reason this lives inside Mikan: the model returns
/// camera-space normals, so without the tracked pose the recovered environment
/// has no defined orientation relative to the Unreal scene.
///
/// See docs/reference/scene-lighting.md for what this can and cannot recover.
class SceneLightingEstimator
{
public:
	struct Config
	{
		std::string modelDirectory;
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

	bool startup(const Config& config);
	void shutdown();

	bool getIsInitialized() const { return m_inference.getIsInitialized(); }
	const char* getActiveExecutionProvider() const { return m_inference.getActiveExecutionProvider(); }

	/// bgrImage should already be undistorted (see VideoFrameDistortionView).
	/// cameraToWorldRotation is the tracked camera's orientation at capture.
	bool estimate(const cv::Mat& bgrImage, const glm::mat3& cameraToWorldRotation, Result& outResult);

	/// Fit only, for tests and for re-fitting cached model output without
	/// paying for inference again.
	bool fitFromModelOutputs(const MarigoldInference::Result& modelOutputs, const glm::mat3& cameraToWorldRotation,
							 Result& outResult) const;

private:
	Config m_config;
	MarigoldInference m_inference;
};
