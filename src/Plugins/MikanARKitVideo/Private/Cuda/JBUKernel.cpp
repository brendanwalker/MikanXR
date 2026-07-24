#include "JBUKernel.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

namespace
{
constexpr const char* kKernelFunctionName= "jbu_upsample_u16_kernel";

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

bool checkCudaResult(CUresult result, const char* what)
{
	if (result == CUDA_SUCCESS)
		return true;

	const char* errorName= nullptr;
	const char* errorString= nullptr;
	cuGetErrorName(result, &errorName);
	cuGetErrorString(result, &errorString);

	MIKAN_LOG_ERROR("JBUKernel") << what << " failed: " << (errorName != nullptr ? errorName : "?") << " - "
								 << (errorString != nullptr ? errorString : "unknown error");

	return false;
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

	return true;
}

void JBUKernel::shutdown()
{
	if (m_module != nullptr)
	{
		cuModuleUnload(m_module);
		m_module= nullptr;
		m_kernelFunc= nullptr;
	}
}

bool JBUKernel::isInitialized() const { return m_module != nullptr; }

bool JBUKernel::upsample(CUdeviceptr depthLow, int lowW, int lowH, int lowStrideBytes, CUdeviceptr confidence,
						 int confidenceStrideBytes, CUdeviceptr guideRGB, int guideW, int guideH, int guideStrideBytes,
						 CUdeviceptr depthOut, int outW, int outH, int outStrideBytes, const JBUParams& params,
						 CUstream stream)
{
	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("JBUKernel::upsample") << "Kernel module not initialized";
		return false;
	}

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

	return checkCudaResult(
		cuLaunchKernel(m_kernelFunc, gridX, gridY, 1, kBlockDim, kBlockDim, 1, 0, stream, args, nullptr),
		"cuLaunchKernel");
}
