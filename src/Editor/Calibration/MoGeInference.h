#pragma once

//-- includes -----
// Result holds cv::Mat by value, so the forward declarations in OpenCVFwd.h
// are not enough here.
#include <opencv2/core.hpp>

#include <memory>
#include <string>

//-- types -----
/// Runs the MoGe-2 monocular geometry model in-process through ONNX Runtime.
///
/// One forward pass of a ViT yields a metric point map, directly-predicted
/// surface normals, and a validity mask. This mirrors
/// tools/moge2_onnx_pipeline.py step for step; that script is the executable
/// specification and was validated against the PyTorch reference (depth within
/// 0.04%, normals within 0.03 degrees).
///
/// The network's point map is affine - camera space up to an unknown Z shift
/// and a global scale. Recovery to metric camera space needs the CALIBRATED
/// horizontal FOV, which Mikan always has; the reference implementation's
/// focal-estimation path is deliberately not ported. The normals need no
/// recovery at all: they are independent of the shift/scale ambiguity
/// (measured 0.0 degrees of movement across a 45-70 degree FOV sweep).
///
/// See docs/reference/scene-lighting.md for the measurements behind this.
class MoGeInference
{
public:
	struct Config
	{
		/// Directory holding model.onnx (Ruicheng/moge-2-vitl-normal-onnx).
		/// Fetched by tools/fetch_moge2_onnx.py.
		std::string modelDirectory;

		/// Try DirectML first, falling back to CPU. CPU works but is ~7x slower.
		bool preferGpu= true;

		/// Number of base ViT tokens. More is finer detail and slower inference.
		/// The checkpoint supports 1200..3600.
		int numTokens= 3600;
	};

	struct Result
	{
		/// All maps are at the input image's resolution, in MIKAN camera space:
		/// +X right, +Y up, +Z toward the viewer - so visible geometry sits at
		/// negative Z. (The raw model output is OpenCV convention; the Y/Z
		/// negation happens here so nothing downstream has to remember it.)

		cv::Mat depth;   ///< CV_32FC1, metres along -Z, +inf where invalid
		cv::Mat points;  ///< CV_32FC3, metric camera-space positions
		cv::Mat normals; ///< CV_32FC3, unit length; ZERO where invalid, which the
						 ///< SH fit's existing unit-length rejection handles
		cv::Mat mask;    ///< CV_8UC1, 1 = valid geometry
	};

	MoGeInference();
	~MoGeInference();

	bool startup(const Config& config);
	void shutdown();

	bool getIsInitialized() const { return m_bIsInitialized; }

	/// "DirectML" or "CPU". Worth surfacing: OnnxSession falls back to CPU
	/// silently, and a ViT that quietly lands on CPU looks like a hang.
	const char* getActiveExecutionProvider() const;

	/// bgrImage is CV_8UC3 as it comes out of the video frame pipeline, and
	/// should already be undistorted. fovXDegrees is the calibrated horizontal
	/// FOV of that (undistorted) frame - metric scale rides directly on it.
	bool run(const cv::Mat& bgrImage, float fovXDegrees, Result& outResult);

	/// The 1-D least squares that recovers the Z shift making the affine point
	/// map reproject through a known focal:  min |focal * xy/(z+shift) - uv|.
	/// pointsCam is CV_32FC3 in the RAW OpenCV model convention, validMask is
	/// CV_8UC1, focal is relative to half the image diagonal. Public and static
	/// for the unit test; production code goes through run().
	static float solveDepthShift(const cv::Mat& pointsCam, const cv::Mat& validMask, float focal);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
	bool m_bIsInitialized= false;
};
