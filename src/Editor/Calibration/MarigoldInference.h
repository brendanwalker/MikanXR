#pragma once

//-- includes -----
// Result holds cv::Mat by value, so the forward declarations in OpenCVFwd.h
// are not enough here.
#include <opencv2/core.hpp>

#include <memory>
#include <string>
#include <vector>

//-- constants -----
/// The exported UNet graphs bake in their downsample/upsample chain, so the
/// latent grid must be divisible by 8 and therefore the padded image must be
/// divisible by 64. Marigold's own preprocessing only aligns to 8, which is a
/// live trap: a latent of 60 fails inside the graph at up_blocks.1/Concat with
/// "mismatched dimensions of 15 and 16". See docs/reference/scene-lighting.md.
static constexpr int k_marigoldImageAlignment= 64;

/// SD2 latent scaling factor.
static constexpr float k_marigoldVaeScale= 0.18215f;

//-- types -----
/// Runs the two Marigold checkpoints in-process through ONNX Runtime.
///
/// This mirrors tools/marigold_onnx_pipeline.py step for step; that script is
/// the executable specification and was validated against the PyTorch
/// reference (recovered light direction agreed to 1.8 degrees).
class MarigoldInference
{
public:
	struct Config
	{
		/// Directory holding vae_encoder.onnx, vae_decoder.onnx,
		/// unet_iid_lighting.onnx, unet_normals.onnx, empty_text_embedding.bin
		/// and scheduler.json. Produced by tools/export_marigold_onnx.py.
		std::string modelDirectory;

		/// Long edge the frame is resized to before inference. The checkpoints
		/// are trained for 768 and drift badly away from it.
		int processingResolution= 768;

		/// Try DirectML first, falling back to CPU. CPU works but is far slower.
		bool preferGpu= true;

		/// Seed for the initial latent noise. Fixed by default so a capture is
		/// reproducible; the denoise loop starts from noise, so an unseeded RNG
		/// would make two runs on the same frame disagree.
		unsigned int seed= 1234u;
	};

	struct Result
	{
		cv::Mat albedo;   ///< CV_32FC3, linear space, [0,1]
		cv::Mat shading;  ///< CV_32FC3, linear space, up to a global scale
		cv::Mat residual; ///< CV_32FC3, non-diffuse remainder
		cv::Mat normals;  ///< CV_32FC3, unit length, CAMERA space
	};

	MarigoldInference();
	~MarigoldInference();

	bool startup(const Config& config);
	void shutdown();

	bool getIsInitialized() const { return m_bIsInitialized; }

	/// "DirectML" or "CPU". Worth surfacing: a silent CPU fallback turns a few
	/// seconds of inference into minutes.
	const char* getActiveExecutionProvider() const;

	/// bgrImage is CV_8UC3 as it comes out of the video frame pipeline.
	bool run(const cv::Mat& bgrImage, Result& outResult);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
	bool m_bIsInitialized= false;
};
