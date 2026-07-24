#pragma once

#include <cuda.h>

#include <string>

// Spatial/range parameters for Joint Bilateral Upsampling. `radius` is a starting
// point, not a tuned value - see ticket D5, which owns final parameter tuning
// against the real 256x192 (ARKit LiDAR) -> target-resolution ratio; the
// CudaDepthUpsample prototype's demo default (radius=6) was tuned for a different,
// much smaller upsample ratio and produces a near-nearest-neighbor effective
// window at ARKit's actual ~7-8x ratio (see ARKitDepthReceiver.h's
// kARKitDepthWidth/kARKitDepthHeight vs typical iPhone video resolution).
struct JBUParams
{
	int radius= 16;
	float sigmaSpatial= 7.0f;
	float sigmaColor= 25.0f;

	// Confidence deweighting (ticket D2). ARKit confidence values: 0=low, 1=medium,
	// 2=high (see ARKitDepthReceiver.h). High confidence always contributes full
	// weight (not tunable); these two deweight low/medium confidence samples
	// instead of hard-skipping them, which degrades more gracefully than an
	// outright cutoff. Only used when a confidence plane is actually passed to
	// upsample() - ignored otherwise. Provisional defaults (D5 owns final tuning);
	// confWeightLow=0.0 reproduces a hard skip, matching D1's confidence-unaware
	// behavior for a sample that would otherwise be excluded entirely.
	float confWeightLow= 0.0f;
	float confWeightMedium= 0.5f;
};

// Host-side wrapper around JBUKernel.cu's PTX module (Joint Bilateral Upsampling of
// a low-res uint16 depth plane, against a full-res RGB guide image, into a
// full-res float depth plane), implemented purely against the CUDA Driver API
// (CUdeviceptr/CUmodule/CUfunction) rather than the Runtime API - see ticket D1's
// design note: this deliberately avoids CUDA Runtime API (<<<>>>, cudaMalloc, ...)
// so that upsample() can be launched against whatever CUDA context is already
// current on the calling thread (e.g. the GstCudaContext Track C's nvcodec
// pipeline already owns), rather than needing a second, independent context.
//
// This class never calls cuCtxCreate/cuCtxSetCurrent itself - the caller is
// responsible for having a valid CUDA context current on the calling thread
// before calling init() or upsample().
class JBUKernel
{
public:
	JBUKernel();
	~JBUKernel();

	JBUKernel(const JBUKernel&)= delete;
	JBUKernel& operator=(const JBUKernel&)= delete;

	// Loads and links the PTX module from `ptxFilePath`. Requires a CUDA context
	// to already be current on the calling thread (see class comment). Safe to
	// call more than once (a no-op if already initialized). Returns false (with a
	// logged reason) on failure.
	bool init(const std::string& ptxFilePath);
	void shutdown();
	bool isInitialized() const;

	// Launches the upsample kernel against raw CUDA device pointers. The caller
	// owns allocation/lifetime of depthLow/confidence/guideRGB/depthOut and must
	// ensure the context they were allocated under is current on the calling
	// thread. `confidence` is optional (ticket D2) - pass 0 to get D1's original,
	// confidence-unaware behavior; when non-zero it must be a lowW x lowH uint8
	// plane matching depthLow's dimensions (see JBUParams for the weighting this
	// applies). depthOut must already be allocated as outW*outH floats (row-major,
	// outStrideBytes per row, may equal outW*sizeof(float) if unpadded).
	// Asynchronous with respect to `stream` (pass nullptr for the default
	// stream) - the caller is responsible for synchronizing before reading
	// depthOut. Returns false on a CUDA launch error (logged).
	bool upsample(CUdeviceptr depthLow, int lowW, int lowH, int lowStrideBytes, CUdeviceptr confidence,
				  int confidenceStrideBytes, CUdeviceptr guideRGB, int guideW, int guideH, int guideStrideBytes,
				  CUdeviceptr depthOut, int outW, int outH, int outStrideBytes, const JBUParams& params,
				  CUstream stream= nullptr);

private:
	CUmodule m_module= nullptr;
	CUfunction m_kernelFunc= nullptr;
};
