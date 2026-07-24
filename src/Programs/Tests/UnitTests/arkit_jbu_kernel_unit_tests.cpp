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

// Covers ticket D1's core correctness surface: JBUKernel's Driver-API-based
// upsample() against a plain CPU re-implementation of the exact same math (the
// same cross-check philosophy as arkit_rvl_swift_crosscheck_unit_tests.cpp - two
// independent implementations of one algorithm, compared against each other).
// Small/fast synthetic data, not the real 256x192 ARKit resolution or the
// CudaDepthUpsample prototype's actual PFM/PNG test assets - those were checked
// manually/exploratorily against this same kernel (see the ticket D1 summary) but
// aren't suitable for this permanent, deterministic, GPU-optional suite (bundling
// ~11MB of binary fixtures into this repo, and hard-failing on any machine without
// an NVIDIA GPU, isn't acceptable for a CI-run test).
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
// kernel's output is checked against below.
void cpuJbuUpsampleReference(const uint16_t* depthLow, int lowW, int lowH, const uint8_t* guideRGB, int guideW,
							 int guideH, float* depthOut, int outW, int outH, const JBUParams& params)
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

					const float w= wSpatial * wColor;
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
		&& kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_guide, guideW, guideH,
						   guideW * 3, d_out, guideW, guideH, guideW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && (cuCtxSynchronize() == CUDA_SUCCESS);
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
	cpuJbuUpsampleReference(depthLow.data(), lowW, lowH, guide.data(), outW, outH, cpuOut.data(), outW, outH, params);

	CUdeviceptr d_depthLow= 0, d_guide= 0, d_out= 0;
	success= success && (cuMemAlloc(&d_depthLow, depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_guide, guide.size()) == CUDA_SUCCESS);
	success= success && (cuMemAlloc(&d_out, static_cast<size_t>(outW) * outH * sizeof(float)) == CUDA_SUCCESS);
	assert(success);

	success= success && (cuMemcpyHtoD(d_depthLow, depthLow.data(), depthLow.size() * sizeof(uint16_t)) == CUDA_SUCCESS);
	success= success && (cuMemcpyHtoD(d_guide, guide.data(), guide.size()) == CUDA_SUCCESS);
	assert(success);

	success= success
			 && kernel.upsample(d_depthLow, lowW, lowH, lowW * static_cast<int>(sizeof(uint16_t)), d_guide, outW, outH,
								outW * 3, d_out, outW, outH, outW * static_cast<int>(sizeof(float)), params);
	assert(success);
	success= success && (cuCtxSynchronize() == CUDA_SUCCESS);
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
	success= success && !kernel.upsample(0, 1, 1, 2, 0, 1, 1, 3, 0, 1, 1, 4, params);
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
	UNIT_TEST_MODULE_CALL_TEST(arkit_jbu_kernel_test_upsample_without_init_fails_cleanly);
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
