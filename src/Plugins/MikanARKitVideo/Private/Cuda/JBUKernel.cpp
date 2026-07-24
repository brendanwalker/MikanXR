#include "JBUKernel.h"
#include "CudaErrorHandling.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

namespace
{
constexpr const char* kKernelFunctionName= "jbu_upsample_u16_kernel";
constexpr const char* kSurfaceKernelFunctionName= "jbu_upsample_u16_surface_kernel";

bool readFileToString(const std::string& path, std::string& outContents)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return false;

	std::ostringstream ss;
	ss << file.rdbuf();
	outContents= ss.str();

	return !outContents.empty();
}
} // namespace

JBUKernel::JBUKernel()= default;

JBUKernel::~JBUKernel() { shutdown(); }

bool JBUKernel::init(const std::string& ptxFilePath)
{
	if (m_module != nullptr)
		return true; // already initialized

	// Harmless/idempotent if the driver (or GStreamer's own cuInit-equivalent
	// bootstrapping) already initialized it - cuInit() is documented as safe to
	// call more than once. This is the one Driver API call this class makes that
	// isn't scoped to "requires a context already current", since cuInit() itself
	// has no context notion.
	if (!checkCudaResult(cuInit(0), "cuInit"))
		return false;

	std::string ptxSource;
	if (!readFileToString(ptxFilePath, ptxSource))
	{
		MIKAN_LOG_ERROR("JBUKernel::init") << "Failed to read PTX file: " << ptxFilePath;
		return false;
	}

	if (!checkCudaResult(cuModuleLoadData(&m_module, ptxSource.c_str()), "cuModuleLoadData"))
		return false;

	if (!checkCudaResult(cuModuleGetFunction(&m_kernelFunc, m_module, kKernelFunctionName), "cuModuleGetFunction"))
	{
		cuModuleUnload(m_module);
		m_module= nullptr;
		return false;
	}

	if (!checkCudaResult(cuModuleGetFunction(&m_surfaceKernelFunc, m_module, kSurfaceKernelFunctionName),
						 "cuModuleGetFunction(surface)"))
	{
		cuModuleUnload(m_module);
		m_module= nullptr;
		m_kernelFunc= nullptr;
		return false;
	}

	// Non-default, non-blocking stream (ticket D3) - never implicitly serializes
	// against the legacy default stream, so upsample() doesn't unnecessarily block
	// on (or block) whatever stream Track C's nvh264dec decode is using.
	if (!checkCudaResult(cuStreamCreate(&m_stream, CU_STREAM_NON_BLOCKING), "cuStreamCreate"))
	{
		shutdown();
		return false;
	}

	if (!checkCudaResult(cuEventCreate(&m_startEvent, CU_EVENT_DEFAULT), "cuEventCreate(start)")
		|| !checkCudaResult(cuEventCreate(&m_stopEvent, CU_EVENT_DEFAULT), "cuEventCreate(stop)"))
	{
		shutdown();
		return false;
	}

	return true;
}

void JBUKernel::shutdown()
{
	if (m_startEvent != nullptr)
	{
		cuEventDestroy(m_startEvent);
		m_startEvent= nullptr;
	}

	if (m_stopEvent != nullptr)
	{
		cuEventDestroy(m_stopEvent);
		m_stopEvent= nullptr;
	}

	if (m_stream != nullptr)
	{
		cuStreamDestroy(m_stream);
		m_stream= nullptr;
	}

	if (m_module != nullptr)
	{
		cuModuleUnload(m_module);
		m_module= nullptr;
		m_kernelFunc= nullptr;
		m_surfaceKernelFunc= nullptr;
	}

	m_hasTiming= false;
}

bool JBUKernel::isInitialized() const { return m_module != nullptr; }

bool JBUKernel::upsample(CUdeviceptr depthLow, int lowW, int lowH, int lowStrideBytes, CUdeviceptr confidence,
						 int confidenceStrideBytes, CUdeviceptr guideRGB, int guideW, int guideH, int guideStrideBytes,
						 CUdeviceptr depthOut, int outW, int outH, int outStrideBytes, const JBUParams& params)
{
	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("JBUKernel::upsample") << "Kernel module not initialized";
		return false;
	}

	m_hasTiming= false;

	// Non-const: cuLaunchKernel's kernelParams is a void* const[] of pointers to
	// each argument's storage, and a const T* can't implicitly convert to void*.
	float scaleX= static_cast<float>(lowW) / static_cast<float>(outW);
	float scaleY= static_cast<float>(lowH) / static_cast<float>(outH);
	float invTwoSigmaSpatial2= 1.0f / (2.0f * params.sigmaSpatial * params.sigmaSpatial);
	float invTwoSigmaColor2= 1.0f / (2.0f * params.sigmaColor * params.sigmaColor);
	int radius= params.radius;
	float confWeightLow= params.confWeightLow;
	float confWeightMedium= params.confWeightMedium;

	// Addresses of each argument, in exact kernel-parameter order/type - the
	// Driver API launch path does no type checking, it just copies these bytes
	// into the kernel's parameter space, so this must match
	// jbu_upsample_u16_kernel's signature in JBUKernel.cu exactly.
	void* args[]= {&depthLow,
				   &lowW,
				   &lowH,
				   &lowStrideBytes,
				   &confidence,
				   &confidenceStrideBytes,
				   &confWeightLow,
				   &confWeightMedium,
				   &guideRGB,
				   &guideW,
				   &guideH,
				   &guideStrideBytes,
				   &depthOut,
				   &outW,
				   &outH,
				   &outStrideBytes,
				   &radius,
				   &invTwoSigmaSpatial2,
				   &invTwoSigmaColor2,
				   &scaleX,
				   &scaleY};

	constexpr unsigned int kBlockDim= 16;
	const unsigned int gridX= (static_cast<unsigned int>(outW) + kBlockDim - 1) / kBlockDim;
	const unsigned int gridY= (static_cast<unsigned int>(outH) + kBlockDim - 1) / kBlockDim;

	// Events bracket just the kernel launch itself (not the whole enqueue path),
	// all on this instance's dedicated stream, so getLastKernelMilliseconds()
	// reflects actual GPU execution time.
	if (!checkCudaResult(cuEventRecord(m_startEvent, m_stream), "cuEventRecord(start)"))
		return false;

	if (!checkCudaResult(
			cuLaunchKernel(m_kernelFunc, gridX, gridY, 1, kBlockDim, kBlockDim, 1, 0, m_stream, args, nullptr),
			"cuLaunchKernel"))
		return false;

	if (!checkCudaResult(cuEventRecord(m_stopEvent, m_stream), "cuEventRecord(stop)"))
		return false;

	m_hasTiming= true;
	return true;
}

bool JBUKernel::upsampleToSurface(CUdeviceptr depthLow, int lowW, int lowH, int lowStrideBytes, CUdeviceptr confidence,
								  int confidenceStrideBytes, CUdeviceptr guideRGB, int guideW, int guideH,
								  int guideStrideBytes, CUsurfObject depthOutSurface, int outW, int outH,
								  const JBUParams& params)
{
	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("JBUKernel::upsampleToSurface") << "Kernel module not initialized";
		return false;
	}

	m_hasTiming= false;

	float scaleX= static_cast<float>(lowW) / static_cast<float>(outW);
	float scaleY= static_cast<float>(lowH) / static_cast<float>(outH);
	float invTwoSigmaSpatial2= 1.0f / (2.0f * params.sigmaSpatial * params.sigmaSpatial);
	float invTwoSigmaColor2= 1.0f / (2.0f * params.sigmaColor * params.sigmaColor);
	int radius= params.radius;
	float confWeightLow= params.confWeightLow;
	float confWeightMedium= params.confWeightMedium;

	// Must match jbu_upsample_u16_surface_kernel's signature in JBUKernel.cu
	// exactly (see the identical note on upsample()'s args array).
	void* args[]= {&depthLow,
				   &lowW,
				   &lowH,
				   &lowStrideBytes,
				   &confidence,
				   &confidenceStrideBytes,
				   &confWeightLow,
				   &confWeightMedium,
				   &guideRGB,
				   &guideW,
				   &guideH,
				   &guideStrideBytes,
				   &depthOutSurface,
				   &outW,
				   &outH,
				   &radius,
				   &invTwoSigmaSpatial2,
				   &invTwoSigmaColor2,
				   &scaleX,
				   &scaleY};

	constexpr unsigned int kBlockDim= 16;
	const unsigned int gridX= (static_cast<unsigned int>(outW) + kBlockDim - 1) / kBlockDim;
	const unsigned int gridY= (static_cast<unsigned int>(outH) + kBlockDim - 1) / kBlockDim;

	if (!checkCudaResult(cuEventRecord(m_startEvent, m_stream), "cuEventRecord(start)"))
		return false;

	if (!checkCudaResult(
			cuLaunchKernel(m_surfaceKernelFunc, gridX, gridY, 1, kBlockDim, kBlockDim, 1, 0, m_stream, args, nullptr),
			"cuLaunchKernel(surface)"))
		return false;

	if (!checkCudaResult(cuEventRecord(m_stopEvent, m_stream), "cuEventRecord(stop)"))
		return false;

	m_hasTiming= true;
	return true;
}

bool JBUKernel::synchronize()
{
	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("JBUKernel::synchronize") << "Kernel module not initialized";
		return false;
	}

	// A kernel-execution error (e.g. an illegal memory access from a bad input
	// pointer passed to upsample()) surfaces here, asynchronously, per normal CUDA
	// semantics - not at the upsample() call that enqueued it.
	return checkCudaResult(cuStreamSynchronize(m_stream), "cuStreamSynchronize");
}

float JBUKernel::getLastKernelMilliseconds() const
{
	if (!m_hasTiming)
		return -1.0f;

	float milliseconds= 0.0f;
	// CUDA_ERROR_NOT_READY (the stop event hasn't completed yet, i.e. the caller
	// hasn't synchronized) is expected and just means "not available yet" - only
	// log genuinely unexpected failures.
	const CUresult result= cuEventElapsedTime(&milliseconds, m_startEvent, m_stopEvent);
	if (result == CUDA_SUCCESS)
		return milliseconds;

	if (result != CUDA_ERROR_NOT_READY)
		checkCudaResult(result, "cuEventElapsedTime");

	return -1.0f;
}
