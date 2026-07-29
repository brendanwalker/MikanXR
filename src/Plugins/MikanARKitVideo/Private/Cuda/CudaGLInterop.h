#pragma once

#include <cuda.h>

#include "MkRendererFwd.h"

// Zero-copy CUDA-GL interop for the decoded ARKit color video frame (ticket E3):
// owns an IMkTexture (MK_RGBA, 8-bit-per-channel) and registers its backing GL
// texture with CUDA, so NV12ConversionKernel can write the decoded/converted frame
// directly into the GL texture's storage with no host round-trip.
//
// THREADING: every method on this class must be called on the thread that owns
// the current GL context. Mikan is single-threaded for GL - there is no separate
// render thread; App::tickWindows() runs each window's update()/render() on the
// main thread, which is the only thread with a GL context ever current (see
// VideoFrameDistortionView::processVideoFrame() for the established pattern of
// marshaling GL work onto that thread from a producer/decode thread - this class
// does not do that marshaling itself, its caller must). This applies to
// init()/resize()/shutdown() (GL texture creation) and
// beginCudaAccess()/endCudaAccess() (CUDA-GL registration/mapping) alike -
// calling any of these off that thread is a documented source of silent
// driver-level failures. A CUDA context must also be current on that thread for
// the CUDA-side calls.
class CudaGLColorTexture
{
public:
	CudaGLColorTexture();
	~CudaGLColorTexture();

	CudaGLColorTexture(const CudaGLColorTexture&)= delete;
	CudaGLColorTexture& operator=(const CudaGLColorTexture&)= delete;

	bool init(int width, int height);
	void shutdown();
	bool isInitialized() const;

	int getWidth() const;
	int getHeight() const;
	IMkTexturePtr getTexture() const;

	bool resize(int width, int height);

	bool beginCudaAccess(CUsurfObject& outSurface);
	bool endCudaAccess(CUstream stream);

private:
	bool registerWithCuda();
	void unregisterFromCuda();

	IMkTexturePtr m_texture;
	CUgraphicsResource m_graphicsResource= nullptr;
	CUsurfObject m_currentSurface= 0;
	int m_width= 0;
	int m_height= 0;
};
