//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include "unit_test.h"

// JBUKernel.h directly includes <cuda.h> and is only compiled in (both here and in
// MikanARKitVideo.dll) when MIKAN_WITH_GSTREAMER is ON - JBU_KERNEL_PTX_PATH is
// only defined in that case too (see UnitTests/CMakeLists.txt), so it doubles as
// the compile-time guard for this whole file. When it's OFF, the stub below
// reports the module as skipped rather than failing to compile/link.
#if defined(JBU_KERNEL_PTX_PATH)

#include <assert.h>
#include <cmath>
#include <cstdint>
#include <vector>

#include <cuda.h>

#include "JBUKernel.h"

// Covers ticket D1/D2/D3's core correctness surface: JBUKernel's Driver-API-based
// upsample() against a plain CPU re-implementation of the exact same math (the
// same cross-check philosophy as arkit_rvl_swift_crosscheck_unit_tests.cpp - two
// independent implementations of one algorithm, compared against each other).
// Small/fast synthetic data, not the real 256x192 ARKit resolution or the
// CudaDepthUpsample prototype's actual PFM/PNG test assets - those were checked
// manually/exploratorily against this same kernel (see the ticket D1 summary) but
// aren't suitable for this permanent, deterministic, GPU-optional suite (bundling
// ~11MB of binary fixtures into this repo, and hard-failing on any machine without
// an NVIDIA GPU, isn't acceptable for a CI-run test).
//
// Ticket D4 (CUDA-GL interop): this file covers upsampleToSurface()'s device-side
// math (via a plain, non-GL-registered CUarray/CUsurfObject - surf2Dwrite behaves
// identically regardless of how the backing CUarray was created, so this is a
// faithful test of the kernel itself). CudaGLInterop.cpp's actual GL texture
// creation/cuGraphicsGLRegisterImage/map-unmap lifecycle is covered separately, in
// arkit_cuda_gl_interop_unit_tests.cpp, which creates its own minimal off-screen
// WGL context for exactly that purpose (this file's tests stay GL-free/headless).
namespace
{
// GST_MAP_CUDA/gst_cuda_load_library() aren't involved here - JBUKernel is
// deliberately GStreamer-agnostic (see its class comment), so this test creates
// its own throwaway CUDA context, exactly as any other standalone CUDA consumer
// would. This is fine for test-only usage; production code (Track D3/D4) must
// NOT create its own context - it reuses whatever GstCudaContext Track C already
// established, which is the entire reason JBUKernel never creates one itself.
bool cudaDeviceAvailable(CUdevice& outDevice)
{
	if (cuInit(0) != CUDA_SUCCESS)
		return false;

	int deviceCount= 0;
	if (cuDeviceGetCount(&deviceCount) != CUDA_SUCCESS || deviceCount <= 0)
		return false;

	return cuDeviceGet(&outDevice, 0) == CUDA_SUCCESS;
}

// Plain host re-implementation of JBUKernel.cu's jbu_upsample_u16_kernel, line for
// line, with CUDA device intrinsics swapped for their standard-library
// equivalents (__expf -> std::exp, etc.). This is the correctness oracle the GPU
// kernel's output is checked against below. `confidence` is optional (ticket D2),
// matching the kernel's own nullable-confidence semantics.
void cpuJbuUpsampleReference(const uint16_t* depthLow, int lowW, int lowH, const uint8_t* confidence,
							 const uint8_t* guideRGB, int guideW, int guideH, float* depthOut, int outW, int outH,
							 const JBUParams& params)
{
	const float scaleX= static_cast<float>(lowW) / static_cast<float>(outW);
	const float scaleY= static_cast<float>(lowH) / static_cast<float>(outH);
	const float invTwoSigmaSpatial2= 1.0f / (2.0f * params.sigmaSpatial * params.sigmaSpatial);
	const float invTwoSigmaColor2= 1.0f / (2.0f * params.sigmaColor * params.sigmaColor);
	const int radius= params.radius;

	for (int y= 0; y < outH; ++y)
	{
		for (int x= 0; x < outW; ++x)
		{
			const float fx= (x + 0.5f) * scaleX - 0.5f;
			const float fy= (y + 0.5f) * scaleY - 0.5f;
			const int lx= static_cast<int>(std::floor(fx));
			const int ly= static_cast<int>(std::floor(fy));

			const uint8_t* guideRow= guideRGB + static_cast<size_t>(y) * guideW * 3;
			const uint8_t gR= guideRow[3 * x + 0];
			const uint8_t gG= guideRow[3 * x + 1];
			const uint8_t gB= guideRow[3 * x + 2];

			float sumW= 0.0f;
			float sumD= 0.0f;

			const int winLowRadiusX= std::max(1, static_cast<int>(std::ceil(radius * scaleX)));
			const int winLowRadiusY= std::max(1, static_cast<int>(std::ceil(radius * scaleY)));

			const int x0= std::max(0, lx - winLowRadiusX);
			const int x1= std::min(lowW - 1, lx + winLowRadiusX);
			const int y0= std::max(0, ly - winLowRadiusY);
			const int y1= std::min(lowH - 1, ly + winLowRadiusY);

			for (int j= y0; j <= y1; ++j)
			{
				const uint16_t* depthRow= depthLow + static_cast<size_t>(j) * lowW;
				const float hy= (j + 0.5f) / scaleY - 0.5f;

				for (int i= x0; i <= x1; ++i)
				{
					const uint16_t dval= depthRow[i];
					if (dval == 0)
						continue;
					const float depthVal= static_cast<float>(dval);

					float wConfidence= 1.0f;
					if (confidence != nullptr)
					{
						const uint8_t c= confidence[static_cast<size_t>(j) * lowW + i];
						wConfidence= (c >= 2) ? 1.0f : (c == 1) ? params.confWeightMedium : params.confWeightLow;
						if (wConfidence <= 0.0f)
							continue;
					}

					const float hx= (i + 0.5f) / scaleX - 0.5f;
					const float dx= x - hx;
					const float dy= y - hy;
					const float spatial2= dx * dx + dy * dy;
					const float wSpatial= std::exp(-spatial2 * invTwoSigmaSpatial2);

					const int gx= std::min(guideW - 1, std::max(0, static_cast<int>(std::round(hx))));
					const int gy= std::min(guideH - 1, std::max(0, static_cast<int>(std::round(hy))));
					const uint8_t* guideRowLow= guideRGB + static_cast<size_t>(gy) * guideW * 3;
					const uint8_t r2= guideRowLow[3 * gx + 0];
					const uint8_t g2= guideRowLow[3 * gx + 1];
					const uint8_t b2= guideRowLow[3 * gx + 2];

					const float dr= static_cast<float>(gR) - static_cast<float>(r2);
					const float dg= static_cast<float>(gG) - static_cast<float>(g2);
					const float db= static_cast<float>(gB) - static_cast<float>(b2);
					const float color2= dr * dr + dg * dg + db * db;
					const float wColor= std::exp(-color2 * invTwoSigmaColor2);

					const float w= wSpatial * wColor * wConfidence;
					sumW+= w;
					sumD+= w * depthVal;
				}
			}

			depthOut[static_cast<size_t>(y) * outW + x]= (sumW > 0.0f) ? (sumD / sumW) : 0.0f;
		}
	}
}

// Deterministic (no RNG) synthetic test pattern - varying depth with some
// invalid (0) pixels sprinkled in, and a guide image with real color variation
// (a gradient), so both the spatial and color weighting terms are meaningfully
// exercised rather than trivially constant.
void generateSyntheticInputs(int lowW, int lowH, int guideW, int guideH, std::vector<uint16_t>& outDepth,
							 std::vector<uint8_t>& outGuide)
{
	outDepth.resize(static_cast<size_t>(lowW) * lowH);
	for (int y= 0; y < lowH; ++y)
	{
		for (int x= 0; x < lowW; ++x)
		{
			const int idx= y * lowW + x;
			const bool invalid= ((x * 7 + y * 3) % 11) == 0;
			outDepth[idx]= invalid ? 0 : static_cast<uint16_t>(500 + (x * 37 + y * 53) % 3000);
		}
	}

	outGuide.resize(static_cast<size_t>(guideW) * guideH * 3);
	for (int y= 0; y < guideH; ++y)
	{
		for (int x= 0; x < guideW; ++x)
		{
			const size_t idx= (static_cast<size_t>(y) * guideW + x) * 3;
			outGuide[idx + 0]= static_cast<uint8_t>((x * 3) % 256);
			outGuide[idx + 1]= static_cast<uint8_t>((y * 5) % 256);
			outGuide[idx + 2]= static_cast<uint8_t>(((x + y) * 2) % 256);
		}
	}
}

// Deterministic mixed 0/1/2 confidence pattern (ticket D2) - varies across both
// axes so the cross-check test exercises all three confidence tiers.
void generateSyntheticConfidence(int lowW, int lowH, std::vector<uint8_t>& outConfidence)
{
	outConfidence.resize(static_cast<size_t>(lowW) * lowH);
	for (int y= 0; y < lowH; ++y)
	{
		for (int x= 0; x < lowW; ++x)
		{
			outConfidence[y * lowW + x]= static_cast<uint8_t>((x + y * 2) % 3);
		}
	}
}
} // namespace

//-- private functions -----
static bool arkit_jbu_kernel_test_init_and_shutdown()
{
	UNIT_TEST_BEGIN("init() loads the PTX module, shutdown() is safe to call repeatedly")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		// No CUDA-capable GPU on whatever machine ran this - nothing further to
		// verify. Not a failure of this test.
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	{
		JBUKernel kernel;
		success= success && !kernel.isInitialized();
		assert(success);

		success= success && kernel.init(JBU_KERNEL_PTX_PATH);
		assert(success);
		success= success && kernel.isInitialized();
		assert(success);

		// Idempotent re-init
		success= success && kernel.init(JBU_KERNEL_PTX_PATH);
		assert(success);

		kernel.shutdown();
		success= success && !kernel.isInitialized();
		assert(success);

		// Safe to call more than once
		kernel.shutdown();
	}

	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_uniform_depth_propagates_unchanged()
{
	UNIT_TEST_BEGIN("a uniform valid depth plane upsamples to that same constant value everywhere")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 8, lowH= 6;
	const int guideW= 32, guideH= 24;
	const uint16_t kConstantDepthMM= 1234;

	std::vector<uint16_t> depthLow(static_cast<size_t>(lowW) * lowH, kConstantDepthMM);
	std::vector<uint8_t> guide;
	{
		std::vector<uint16_t> unusedDepth;
		generateSyntheticInputs(lowW, lowH, guideW, guideH, unusedDepth, guide);
	}

	CUdeviceptr d_depthLow= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(guideW) * guideH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	JBUParams params;
	success=
		success
		&& kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0, d_guide, guideW,
						   guideH, guideW * 3, d_out, guideW, guideH, guideW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	std::vector<float> out(static_cast<size_t>(guideW) * guideH);
	success= success && (cuMemcpyDtoH(out.data(), d_out, out.size() * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	bool allMatch= true;
	for (float v : out)
	{
		if (std::fabs(v - static_cast<float>(kConstantDepthMM)) > 0.01f)
		{
			allMatch= false;
			break;
		}
	}
	success= success && allMatch;
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_out != 0)
		cuMemFree(d_out);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_matches_cpu_reference()
{
	UNIT_TEST_BEGIN("GPU kernel output matches a plain CPU re-implementation of the same math")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);

	JBUParams params;
	params.radius= 8;
	params.sigmaSpatial= 6.0f;
	params.sigmaColor= 20.0f;

	std::vector<float> cpuOut(static_cast<size_t>(outW) * outH);
	cpuJbuUpsampleReference(depthLow.data(), lowW, lowH, nullptr, guide.data(), outW, outH, cpuOut.data(), outW, outH,
							params);

	CUdeviceptr d_depthLow= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success
			 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0, d_guide, outW,
								outH, outW * 3, d_out, outW, outH, outW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	std::vector<float> gpuOut(static_cast<size_t>(outW) * outH);
	success= success && (cuMemcpyDtoH(gpuOut.data(), d_out, gpuOut.size() * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	// __expf (device fast-math intrinsic) vs std::exp (full precision) means the
	// two implementations won't bit-match - a generous-but-meaningful absolute
	// tolerance in millimeters catches real algorithmic divergence while
	// tolerating that expected fast-math drift.
	float maxAbsDiff= 0.0f;
	for (size_t i= 0; i < gpuOut.size(); ++i)
	{
		maxAbsDiff= std::max(maxAbsDiff, std::fabs(gpuOut[i] - cpuOut[i]));
	}
	success= success && (maxAbsDiff < 1.0f);
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_out != 0)
		cuMemFree(d_out);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_high_confidence_matches_unweighted_baseline()
{
	UNIT_TEST_BEGIN("an all-high-confidence plane produces identical output to passing no confidence plane at all")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);
	std::vector<uint8_t> confidenceAllHigh(static_cast<size_t>(lowW) * lowH, 2);

	JBUParams params;
	params.radius= 8;

	CUdeviceptr d_depthLow= 0, d_confidence= 0, d_guide= 0, d_outNoConf= 0, d_outHighConf= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_confidence, confidenceAllHigh.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_outNoConf, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_outHighConf, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success=
		success && (cuMemcpyHtoD(d_confidence, confidenceAllHigh.data(), confidenceAllHigh.size()) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	success=
		success
		&& kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0, d_guide, outW, outH,
						   outW * 3, d_outNoConf, outW, outH, outW * static_cast<int>(sizeof(float)), params);
	success= success
			 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_confidence, lowW,
								d_guide, outW, outH, outW * 3, d_outHighConf, outW, outH,
								outW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	std::vector<float> outNoConf(static_cast<size_t>(outW) * outH);
	std::vector<float> outHighConf(static_cast<size_t>(outW) * outH);
	success= success && (cuMemcpyDtoH(outNoConf.data(), d_outNoConf, outNoConf.size() * sizeof(float)) == CUDA_SUCCESS);
	success= success
			 && (cuMemcpyDtoH(outHighConf.data(), d_outHighConf, outHighConf.size() * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	// Same math path either way (wConfidence is exactly 1.0f in both cases) -
	// should be bit-identical, not just close.
	bool identical= true;
	for (size_t i= 0; i < outNoConf.size(); ++i)
	{
		if (outNoConf[i] != outHighConf[i])
		{
			identical= false;
			break;
		}
	}
	success= success && identical;
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_confidence != 0)
		cuMemFree(d_confidence);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_outNoConf != 0)
		cuMemFree(d_outNoConf);
	if (d_outHighConf != 0)
		cuMemFree(d_outHighConf);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_all_low_confidence_degenerate_case_outputs_zero()
{
	UNIT_TEST_BEGIN("an all-low-confidence plane (default confWeightLow=0) excludes every sample -> output is 0.0 "
					"everywhere, not NaN/garbage")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);
	// All-invalid depth would trivially hit the same fallback for a different
	// reason (D1's own dval==0 skip) - use real, valid depth values here so this
	// test genuinely exercises the confidence-driven exclusion path, not the
	// pre-existing invalid-depth one.
	for (uint16_t& d : depthLow)
	{
		if (d == 0)
			d= 1000;
	}
	std::vector<uint8_t> confidenceAllLow(static_cast<size_t>(lowW) * lowH, 0);

	JBUParams params; // confWeightLow defaults to 0.0
	params.radius= 8;

	CUdeviceptr d_depthLow= 0, d_confidence= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_confidence, confidenceAllLow.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_confidence, confidenceAllLow.data(), confidenceAllLow.size()) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success
			 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_confidence, lowW,
								d_guide, outW, outH, outW * 3, d_out, outW, outH,
								outW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	std::vector<float> out(static_cast<size_t>(outW) * outH);
	success= success && (cuMemcpyDtoH(out.data(), d_out, out.size() * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	bool allZero= true;
	for (float v : out)
	{
		if (v != 0.0f)
		{
			allZero= false;
			break;
		}
	}
	success= success && allZero;
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_confidence != 0)
		cuMemFree(d_confidence);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_out != 0)
		cuMemFree(d_out);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_mixed_confidence_matches_cpu_reference()
{
	UNIT_TEST_BEGIN("GPU kernel with a mixed 0/1/2 confidence plane matches the CPU reference's confidence weighting")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);
	std::vector<uint8_t> confidence;
	generateSyntheticConfidence(lowW, lowH, confidence);

	JBUParams params;
	params.radius= 8;
	params.sigmaSpatial= 6.0f;
	params.sigmaColor= 20.0f;
	params.confWeightLow= 0.1f;
	params.confWeightMedium= 0.5f;

	std::vector<float> cpuOut(static_cast<size_t>(outW) * outH);
	cpuJbuUpsampleReference(depthLow.data(), lowW, lowH, confidence.data(), guide.data(), outW, outH, cpuOut.data(),
							outW, outH, params);

	CUdeviceptr d_depthLow= 0, d_confidence= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_confidence, confidence.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_confidence, confidence.data(), confidence.size()) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success
			 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_confidence, lowW,
								d_guide, outW, outH, outW * 3, d_out, outW, outH,
								outW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	std::vector<float> gpuOut(static_cast<size_t>(outW) * outH);
	success= success && (cuMemcpyDtoH(gpuOut.data(), d_out, gpuOut.size() * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	float maxAbsDiff= 0.0f;
	for (size_t i= 0; i < gpuOut.size(); ++i)
	{
		maxAbsDiff= std::max(maxAbsDiff, std::fabs(gpuOut[i] - cpuOut[i]));
	}
	success= success && (maxAbsDiff < 1.0f);
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_confidence != 0)
		cuMemFree(d_confidence);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_out != 0)
		cuMemFree(d_out);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_surface_kernel_matches_cpu_reference()
{
	UNIT_TEST_BEGIN(
		"upsampleToSurface() (ticket D4) matches the CPU reference, writing via a plain CUarray/CUsurfObject "
		"(no GL - see the file-level comment on what D4's GL-interop-specific code isn't covered by this suite)")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);
	std::vector<uint8_t> confidence;
	generateSyntheticConfidence(lowW, lowH, confidence);

	JBUParams params;
	params.radius= 8;
	params.sigmaSpatial= 6.0f;
	params.sigmaColor= 20.0f;
	params.confWeightLow= 0.1f;
	params.confWeightMedium= 0.5f;

	std::vector<float> cpuOut(static_cast<size_t>(outW) * outH);
	cpuJbuUpsampleReference(depthLow.data(), lowW, lowH, confidence.data(), guide.data(), outW, outH, cpuOut.data(),
							outW, outH, params);

	CUdeviceptr d_depthLow= 0, d_confidence= 0, d_guide= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_confidence, confidence.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_confidence, confidence.data(), confidence.size()) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	// A plain CUDA array (not GL-registered) - surf2Dwrite works identically on
	// any CUarray-backed surface object regardless of how the array was created,
	// so this exercises the exact same device-side write path
	// CudaGLInterop.cpp's GL-registered arrays would use, without needing a GL
	// context (unavailable in this headless console test executable).
	CUDA_ARRAY_DESCRIPTOR arrayDesc= {};
	arrayDesc.Width= static_cast<size_t>(outW);
	arrayDesc.Height= static_cast<size_t>(outH);
	arrayDesc.Format= CU_AD_FORMAT_FLOAT;
	arrayDesc.NumChannels= 1;

	CUarray outArray= nullptr;
	success= success && (cuArrayCreate(&outArray, &arrayDesc) == CUDA_SUCCESS);
	assert(success);

	CUsurfObject outSurface= 0;
	if (success)
	{
		CUDA_RESOURCE_DESC resourceDesc= {};
		resourceDesc.resType= CU_RESOURCE_TYPE_ARRAY;
		resourceDesc.res.array.hArray= outArray;
		success= success && (cuSurfObjectCreate(&outSurface, &resourceDesc) == CUDA_SUCCESS);
		assert(success);
	}

	success=
		success
		&& kernel.upsampleToSurface(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_confidence,
									lowW, d_guide, outW, outH, outW * 3, outSurface, outW, outH, params);
	assert(success);
	success= success && kernel.synchronize();
	assert(success);

	std::vector<float> gpuOut(static_cast<size_t>(outW) * outH);
	if (success)
	{
		CUDA_MEMCPY2D copyParams= {};
		copyParams.srcMemoryType= CU_MEMORYTYPE_ARRAY;
		copyParams.srcArray= outArray;
		copyParams.dstMemoryType= CU_MEMORYTYPE_HOST;
		copyParams.dstHost= gpuOut.data();
		copyParams.dstPitch= static_cast<size_t>(outW) * sizeof(float);
		copyParams.WidthInBytes= static_cast<size_t>(outW) * sizeof(float);
		copyParams.Height= static_cast<size_t>(outH);
		success= success && (cuMemcpy2D(&copyParams) == CUDA_SUCCESS);
		assert(success);
	}

	float maxAbsDiff= 0.0f;
	for (size_t i= 0; i < gpuOut.size(); ++i)
	{
		maxAbsDiff= std::max(maxAbsDiff, std::fabs(gpuOut[i] - cpuOut[i]));
	}
	success= success && (maxAbsDiff < 1.0f);
	assert(success);

	if (outSurface != 0)
		cuSurfObjectDestroy(outSurface);
	if (outArray != nullptr)
		cuArrayDestroy(outArray);
	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_confidence != 0)
		cuMemFree(d_confidence);
	if (d_guide != 0)
		cuMemFree(d_guide);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_timing_is_reported_and_reasonable()
{
	UNIT_TEST_BEGIN("getLastKernelMilliseconds() reports a real, sub-frame-budget elapsed time after synchronize()")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	// Not yet run - no timing available.
	success= success && (kernel.getLastKernelMilliseconds() < 0.0f);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);

	CUdeviceptr d_depthLow= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	JBUParams params;
	params.radius= 8;

	success= success
			 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0, d_guide, outW,
								outH, outW * 3, d_out, outW, outH, outW * static_cast<int>(sizeof(float)), params);
	assert(success);

	success= success && kernel.synchronize();
	assert(success);

	const float ms= kernel.getLastKernelMilliseconds();
	// A tiny 64x48 kernel launch should be well under a 30fps (33ms) frame
	// budget, let alone 16ms - a generous upper bound here just catches gross
	// regressions (e.g. accidentally measuring wall-clock host time instead of
	// GPU time), not a tight performance assertion.
	success= success && (ms >= 0.0f) && (ms < 16.0f);
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_out != 0)
		cuMemFree(d_out);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_bad_output_pointer_fails_at_synchronize_not_process_crash()
{
	UNIT_TEST_BEGIN(
		"a kernel fault from an invalid device pointer is caught at synchronize() (async CUDA error), not a crash")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);

	CUdeviceptr d_depthLow= 0, d_guide= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	// Deliberately invalid, unallocated device pointer (ticket D3's "inject a
	// deliberate CUDA error" verification) - every thread writes to depthOut, so
	// this reliably faults the kernel with an illegal address rather than merely
	// producing garbage output. cuLaunchKernel itself only enqueues work and is
	// expected to succeed; the fault is only observable asynchronously, at the
	// next synchronization point (see JBUKernel::synchronize's comment).
	const CUdeviceptr kBadDevicePtr= static_cast<CUdeviceptr>(0x1);

	JBUParams params;
	params.radius= 8;

	const bool launchEnqueued=
		kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), 0, 0, d_guide, outW, outH,
						outW * 3, kBadDevicePtr, outW, outH, outW * static_cast<int>(sizeof(float)), params);
	success= success && launchEnqueued;
	assert(success);

	// The fault must be reported here, as a normal false return - not a process
	// crash/abort. This is the whole point of ticket D3's async error handling:
	// the caller can log this and skip the frame's depth upsample rather than the
	// app dying.
	const bool synchronizeSucceeded= kernel.synchronize();
	success= success && !synchronizeSucceeded;
	assert(success);

	// Deliberately NOT calling cuMemFree/kernel.shutdown()/cuCtxDestroy here.
	// After an illegal-address fault, the CUDA context is left in a
	// driver-defined broken state where further Driver API calls against it are
	// not just error-returning but can themselves crash the host process
	// (confirmed empirically while writing this test: cuMemFree/cuCtxDestroy on
	// the post-fault context segfaulted this whole test binary, not just failed
	// gracefully) - so once the fault is confirmed via synchronize(), this
	// context and its allocations are simply abandoned/leaked for the remainder
	// of the process, exactly as real production code must also do (see
	// JBUKernel's class comment: on a real async fault, the right move is to
	// treat the whole context/session as unrecoverable, not attempt to keep
	// using it).

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_repeated_runs_on_identical_input_are_bit_identical()
{
	UNIT_TEST_BEGIN("running the same input through the kernel twice produces bit-identical output (ticket D5's "
					"no-flicker rationale: the algorithm itself is deterministic, no randomness anywhere)")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel;
	success= success && kernel.init(JBU_KERNEL_PTX_PATH);
	assert(success);

	const int lowW= 16, lowH= 12;
	const int outW= 64, outH= 48;

	std::vector<uint16_t> depthLow;
	std::vector<uint8_t> guide;
	generateSyntheticInputs(lowW, lowH, outW, outH, depthLow, guide);
	std::vector<uint8_t> confidence;
	generateSyntheticConfidence(lowW, lowH, confidence);

	// The D5-tuned defaults specifically (not a hand-picked test-only radius/sigma
	// like the other tests here) - this test is about confirming those defaults
	// don't introduce any nondeterminism, not about cross-checking the math.
	JBUParams params;

	CUdeviceptr d_depthLow= 0, d_confidence= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_confidence, confidence.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_confidence, confidence.data(), confidence.size()) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	std::vector<float> firstRun(static_cast<size_t>(outW) * outH);
	std::vector<float> secondRun(static_cast<size_t>(outW) * outH);

	for (std::vector<float>* out : {&firstRun, &secondRun})
	{
		success= success
				 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_confidence,
									lowW, d_guide, outW, outH, outW * 3, d_out, outW, outH,
									outW * static_cast<int>(sizeof(float)), params);
		success= success && kernel.synchronize();
		success= success && (cuMemcpyDtoH(out->data(), d_out, out->size() * sizeof(float)) == CUDA_SUCCESS);
		assert(success);
	}

	bool identical= true;
	for (size_t i= 0; i < firstRun.size(); ++i)
	{
		if (firstRun[i] != secondRun[i])
		{
			identical= false;
			break;
		}
	}
	success= success && identical;
	assert(success);

	if (d_depthLow != 0)
		cuMemFree(d_depthLow);
	if (d_confidence != 0)
		cuMemFree(d_confidence);
	if (d_guide != 0)
		cuMemFree(d_guide);
	if (d_out != 0)
		cuMemFree(d_out);
	kernel.shutdown();
	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

static bool arkit_jbu_kernel_test_upsample_without_init_fails_cleanly()
{
	UNIT_TEST_BEGIN("upsample() before init() fails cleanly instead of crashing")

	CUdevice device;
	if (!cudaDeviceAvailable(device))
	{
		UNIT_TEST_COMPLETE()
	}

	CUcontext context= nullptr;
	success= (cuCtxCreate(&context, nullptr, 0, device) == CUDA_SUCCESS);
	assert(success);

	JBUKernel kernel; // never initialized
	JBUParams params;
	success= success && !kernel.upsample(0, 1, 1, 2, 0, 2, 0, 1, 1, 3, 0, 1, 1, 4, params);
	assert(success);

	if (context != nullptr)
		cuCtxDestroy(context);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_jbu_kernel_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_jbu_kernel")
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_init_and_shutdown);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_uniform_depth_propagates_unchanged);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_matches_cpu_reference);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_high_confidence_matches_unweighted_baseline);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_all_low_confidence_degenerate_case_outputs_zero);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_mixed_confidence_matches_cpu_reference);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_surface_kernel_matches_cpu_reference);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_timing_is_reported_and_reasonable);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_repeated_runs_on_identical_input_are_bit_identical);
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_upsample_without_init_fails_cleanly);
	// Runs last - triggers a real GPU fault, which can leave its own throwaway
	// CUDA context in a driver-defined "unusable" state after the fact (expected,
	// see the test's own comment); harmless since every test here uses its own
	// fresh context, but keeping it last avoids any doubt.
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_bad_output_pointer_fails_at_synchronize_not_process_crash);
	UNIT_TEST_MODULE_END()
}

#else // !defined(JBU_KERNEL_PTX_PATH)

bool run_arkit_jbu_kernel_unit_tests()
{
	fprintf(stdout, "[arkit_jbu_kernel]\n");
	fprintf(stdout, "  skipped - built without MIKAN_WITH_GSTREAMER\n");
	return true;
}

#endif // defined(JBU_KERNEL_PTX_PATH)
