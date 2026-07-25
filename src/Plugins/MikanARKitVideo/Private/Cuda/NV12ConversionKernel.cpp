#include "NV12ConversionKernel.h"
#include "CudaErrorHandling.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

namespace
{
constexpr const char* kNV12KernelFunctionName= "nv12_to_rgb_kernel";

bool nv12ReadFileToString(const std::string& path, std::string& outContents)
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

NV12ConversionKernel::NV12ConversionKernel()= default;

NV12ConversionKernel::~NV12ConversionKernel() { shutdown(); }

bool NV12ConversionKernel::init(const std::string& ptxFilePath)
{
	if (m_module != nullptr)
		return true; // already initialized

	if (!checkCudaResult(cuInit(0), "cuInit"))
		return false;

	std::string ptxSource;
	if (!nv12ReadFileToString(ptxFilePath, ptxSource))
	{
		MIKAN_LOG_ERROR("NV12ConversionKernel::init") << "Failed to read PTX file: " << ptxFilePath;
		return false;
	}

	if (!checkCudaResult(cuModuleLoadData(&m_module, ptxSource.c_str()), "cuModuleLoadData"))
		return false;

	if (!checkCudaResult(cuModuleGetFunction(&m_kernelFunc, m_module, kNV12KernelFunctionName), "cuModuleGetFunction"))
	{
		cuModuleUnload(m_module);
		m_module= nullptr;
		return false;
	}

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

void NV12ConversionKernel::shutdown()
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
	}

	m_hasTiming= false;
}

bool NV12ConversionKernel::isInitialized() const { return m_module != nullptr; }

bool NV12ConversionKernel::convert(CUdeviceptr yPlane, int yStrideBytes, CUdeviceptr uvPlane, int uvStrideBytes,
								   int width, int height, CUsurfObject rgbaSurface, CUdeviceptr guideRGBOut,
								   int guideStrideBytes)
{
	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("NV12ConversionKernel::convert") << "Kernel module not initialized";
		return false;
	}

	m_hasTiming= false;

	void* args[]= {&yPlane, &yStrideBytes, &uvPlane,     &uvStrideBytes,   &width,
				   &height, &rgbaSurface,  &guideRGBOut, &guideStrideBytes};

	constexpr unsigned int kBlockDim= 16;
	const unsigned int gridX= (static_cast<unsigned int>(width) + kBlockDim - 1) / kBlockDim;
	const unsigned int gridY= (static_cast<unsigned int>(height) + kBlockDim - 1) / kBlockDim;

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

bool NV12ConversionKernel::synchronize()
{
	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("NV12ConversionKernel::synchronize") << "Kernel module not initialized";
		return false;
	}

	return checkCudaResult(cuStreamSynchronize(m_stream), "cuStreamSynchronize");
}

float NV12ConversionKernel::getLastKernelMilliseconds() const
{
	if (!m_hasTiming)
		return -1.0f;

	float milliseconds= 0.0f;
	const CUresult result= cuEventElapsedTime(&milliseconds, m_startEvent, m_stopEvent);
	if (result == CUDA_SUCCESS)
		return milliseconds;

	if (result != CUDA_ERROR_NOT_READY)
		checkCudaResult(result, "cuEventElapsedTime");

	return -1.0f;
}
