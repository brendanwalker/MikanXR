#pragma once

#include <cuda.h>

#include <string>

// Host-side wrapper around NV12ConversionKernel.cu's PTX module (CUDA Driver API
// only, owns a dedicated non-blocking stream, never creates/sets a CUDA context
// itself - the caller must have one current).
class NV12ConversionKernel
{
public:
	NV12ConversionKernel();
	~NV12ConversionKernel();

	NV12ConversionKernel(const NV12ConversionKernel&)= delete;
	NV12ConversionKernel& operator=(const NV12ConversionKernel&)= delete;

	bool init(const std::string& ptxFilePath);
	void shutdown();
	bool isInitialized() const;

	// Converts one NV12 frame to RGBA (written into rgbaSurface, e.g. a
	// CudaGLColorTexture's mapped surface) and to packed RGB (written into
	// guideRGBOut, a plain device buffer at least height*guideStrideBytes bytes -
	// a mandatory output parameter of this kernel, though nothing currently reads
	// it). Asynchronous - enqueues on this instance's own stream and returns
	// immediately; call synchronize() before consuming either output or reading
	// getLastKernelMilliseconds().
	bool convert(CUdeviceptr yPlane, int yStrideBytes, CUdeviceptr uvPlane, int uvStrideBytes, int width, int height,
				 CUsurfObject rgbaSurface, CUdeviceptr guideRGBOut, int guideStrideBytes);

	bool synchronize();
	float getLastKernelMilliseconds() const;

private:
	CUmodule m_module= nullptr;
	CUfunction m_kernelFunc= nullptr;
	CUstream m_stream= nullptr;
	CUevent m_startEvent= nullptr;
	CUevent m_stopEvent= nullptr;
	bool m_hasTiming= false;
};
