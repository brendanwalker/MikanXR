#pragma once

#include <cuda.h>

#include <string>

// Spatial/range parameters for Joint Bilateral Upsampling. Defaults tuned in
// ticket D5 against the CudaDepthUpsample prototype's real depth_low.pfm (256x192,
// exactly ARKit LiDAR's resolution - see ARKitDepthReceiver.h's
// kARKitDepthWidth/kARKitDepthHeight) / rgb_full.png (1920x1080, a ~7.5x/5.6x
// upsample ratio matching a real iPhone video feed) test assets - not synthetic
// data. Method: swept radius in {6 (the prototype's own demo default), 16, 24, 32,
// 48} x sigmaSpatial roughly proportional to radius x sigmaColor in {15, 25, 30},
// visually comparing edge fidelity (object silhouette crispness, background detail
// resolved) against a nearest-neighbor-upsample baseline via
// src/Programs/Tests/UnitTests's own JBUKernel test harness (not committed - a
// throwaway sweep tool loading the prototype's real assets, one-off per this
// ticket's own "no new files" scope).
//
// Findings:
//  - The prototype's own demo defaults (radius=6, sigmaColor=25) noticeably
//    under-resolve at ARKit's real ~7-8x ratio (confirmed the concern already
//    flagged by earlier tickets) - only marginally better than nearest-neighbor.
//  - sigmaColor=15 (tighter than the prototype's 25) consistently produced
//    visibly crisper edges than 25-30 at every radius tested - a tighter color
//    threshold prevents blending across true depth discontinuities even with a
//    wide spatial window, which matters more than sigmaSpatial for perceived
//    quality here.
//  - Larger radius (32-48, well beyond the prototype's demo scale) resolved
//    meaningfully more real detail (individual background shelf items, a
//    foot/shoe silhouette that radius=16 blurred away) with no over-smoothing
//    artifacts, because the tight sigmaColor keeps it edge-aware even over a wide
//    window - larger radius was a net win here, not a tradeoff. radius=48 was the
//    single best-looking result of the sweep; chosen default is radius=32 (with
//    sigmaSpatial scaled proportionally) as a slightly more conservative point
//    that already captures nearly all of that improvement, since 48 was only
//    tested at this one scene/ratio and hasn't been stress-tested against
//    degenerate inputs (e.g. very sparse valid low-res regions) - revisit toward
//    48 once more real scenes are available to confirm it holds up.
//  - Cost is a non-issue anywhere in the tested range: even radius=48 measured
//    2.21ms via JBUKernel::getLastKernelMilliseconds() on this dev machine's GPU
//    (RTX 3090) at 1920x1080 output - far under a 30fps (33ms) budget, let alone
//    16ms. radius=6 measured 0.16ms; radius=32 (the chosen default) measured
//    2.41ms. Performance was never the deciding factor in this tradeoff.
//  - Flicker/stability (the original motivation for choosing lossless RVL over a
//    lossy depth codec): the algorithm itself is a deterministic weighted average
//    with no randomness (confirmed bit-identical output re-running the same
//    input twice - see arkit_jbu_kernel_unit_tests.cpp), so it cannot introduce
//    flicker on its own; a tighter sigmaColor also means less cross-frame jitter
//    gets pulled in from spatially-adjacent-but-unrelated surfaces than a looser
//    threshold would. This is an analytical argument, not a verified-against-
//    real-repeated-captures one - no live ARKit device was available in this
//    environment to capture the same static scene twice and compare (see ticket
//    D5's own summary for this disclosed gap).
struct JBUParams
{
	int radius= 32;
	float sigmaSpatial= 16.0f;
	float sigmaColor= 15.0f;

	// Confidence deweighting (ticket D2). ARKit confidence values: 0=low, 1=medium,
	// 2=high (see ARKitDepthReceiver.h). High confidence always contributes full
	// weight (not tunable); these two deweight low/medium confidence samples
	// instead of hard-skipping them, which degrades more gracefully than an
	// outright cutoff. Only used when a confidence plane is actually passed to
	// upsample() - ignored otherwise. Not part of D5's visual sweep (no confidence
	// data accompanies the prototype's test assets) - confWeightLow=0.0 reproduces
	// a hard skip, matching D1's confidence-unaware behavior for a sample that
	// would otherwise be excluded entirely; revisit empirically once real ARKit
	// confidence data is available (Track E).
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
//
// Ticket D3: upsample() is fully asynchronous - it enqueues work on a dedicated,
// non-default CUDA stream this class owns (CU_STREAM_NON_BLOCKING, so it never
// implicitly serializes against the legacy default stream, e.g. whatever stream
// Track C's nvh264dec decode happens to use) and returns immediately, without
// blocking on cuCtxSynchronize/cuStreamSynchronize. Callers that need the result -
// or an accurate kernel timing via getLastKernelMilliseconds() - must call
// synchronize() first. A CUDA error surfacing asynchronously (e.g. a kernel fault
// from a bad input pointer) is only ever observed at that synchronize() call, per
// normal CUDA semantics; JBUKernel never crashes the process on such an error,
// only logs and returns false, so the caller can skip that frame's depth upsample
// and continue running (see the class's error-handling design note in
// CudaErrorHandling.h).
class JBUKernel
{
public:
	JBUKernel();
	~JBUKernel();

	JBUKernel(const JBUKernel&)= delete;
	JBUKernel& operator=(const JBUKernel&)= delete;

	// Loads and links the PTX module and creates this instance's dedicated stream
	// and timing events. Requires a CUDA context to already be current on the
	// calling thread (see class comment). Safe to call more than once (a no-op if
	// already initialized). Returns false (with a logged reason) on failure.
	bool init(const std::string& ptxFilePath);
	void shutdown();
	bool isInitialized() const;

	// Launches the upsample kernel against raw CUDA device pointers, on this
	// instance's own dedicated stream (see class comment - NOT a blocking call).
	// The caller owns allocation/lifetime of depthLow/confidence/guideRGB/depthOut
	// and must ensure the context they were allocated under is current on the
	// calling thread. `confidence` is optional (ticket D2) - pass 0 to get D1's
	// original, confidence-unaware behavior; when non-zero it must be a lowW x
	// lowH uint8 plane matching depthLow's dimensions (see JBUParams for the
	// weighting this applies). depthOut must already be allocated as outW*outH
	// floats (row-major, outStrideBytes per row, may equal outW*sizeof(float) if
	// unpadded). Returns false if the launch itself could not be enqueued (logged)
	// - this does NOT guarantee the kernel ran successfully, only that it was
	// submitted; see synchronize().
	bool upsample(CUdeviceptr depthLow, int lowW, int lowH, int lowStrideBytes, CUdeviceptr confidence,
				  int confidenceStrideBytes, CUdeviceptr guideRGB, int guideW, int guideH, int guideStrideBytes,
				  CUdeviceptr depthOut, int outW, int outH, int outStrideBytes, const JBUParams& params);

	// Ticket D4: identical to upsample() above, but writes via a CUsurfObject
	// instead of a linear/pitched CUdeviceptr - required when the destination is a
	// CUDA-GL-interop-mapped CUarray (see CudaGLInterop.h), which is opaque,
	// texture-optimized memory that can't be addressed via a raw pointer at all.
	// depthOutSurface must wrap a single-channel 32-bit float (R32F) CUarray sized
	// outW x outH. Same asynchronous/threading/error-handling contract as
	// upsample() - see synchronize().
	bool upsampleToSurface(CUdeviceptr depthLow, int lowW, int lowH, int lowStrideBytes, CUdeviceptr confidence,
						   int confidenceStrideBytes, CUdeviceptr guideRGB, int guideW, int guideH,
						   int guideStrideBytes, CUsurfObject depthOutSurface, int outW, int outH,
						   const JBUParams& params);

	// Blocks the calling thread until all work enqueued on this instance's stream
	// (i.e. the most recent upsample() call) has completed. This is where an
	// asynchronous kernel-execution error (e.g. an illegal memory access from a
	// bad input pointer) actually surfaces, per normal CUDA semantics - returns
	// false (logged) in that case rather than crashing. Only meaningful to call
	// after upsample(); required before reading depthOut or calling
	// getLastKernelMilliseconds().
	bool synchronize();

	// Elapsed GPU time (milliseconds) of the most recent upsample() call, measured
	// via CUDA events recorded immediately before/after the kernel launch. Only
	// valid after a successful upsample() followed by a successful synchronize()
	// (or other confirmation that the stream has drained) - returns -1.0f
	// otherwise (e.g. not yet run, or the timing query itself failed).
	float getLastKernelMilliseconds() const;

private:
	CUmodule m_module= nullptr;
	CUfunction m_kernelFunc= nullptr;
	CUfunction m_surfaceKernelFunc= nullptr;
	CUstream m_stream= nullptr;
	CUevent m_startEvent= nullptr;
	CUevent m_stopEvent= nullptr;
	bool m_hasTiming= false;
};
