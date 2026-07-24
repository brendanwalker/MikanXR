#pragma once

#include <cuda.h>

#include "MkRendererFwd.h"

// Zero-copy CUDA-GL interop for the JBU depth output (ticket D4): owns an
// IMkTexture (MK_R32F, single-channel float) and registers its backing GL texture
// with CUDA, so JBUKernel::upsampleToSurface() can write directly into the GL
// texture's storage with no host round-trip (see JBUKernel.h's
// upsampleToSurface()).
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
// the CUDA-side calls, same requirement as JBUKernel itself.
class CudaGLDepthTexture
{
public:
	CudaGLDepthTexture();
	~CudaGLDepthTexture();

	CudaGLDepthTexture(const CudaGLDepthTexture&)= delete;
	CudaGLDepthTexture& operator=(const CudaGLDepthTexture&)= delete;

	// Creates the IMkTexture (MK_R32F, width x height) and registers its GL
	// texture with CUDA for surface load/store access (see class comment for
	// threading requirements). Safe to call more than once (a no-op if already
	// initialized at this size - use resize() to change dimensions). Returns
	// false (logged) on failure.
	bool init(int width, int height);
	void shutdown();
	bool isInitialized() const;

	int getWidth() const;
	int getHeight() const;
	IMkTexturePtr getTexture() const;

	// Unregisters + destroys + recreates + reregisters the texture at the new
	// size (ticket D4's edge case: CUDA-GL interop has no resize-in-place - the
	// registration is tied to a specific GL texture object/size). A no-op
	// (returns true) if width/height match the current size. Same threading
	// requirements as init().
	bool resize(int width, int height);

	// Maps the registered texture for CUDA access this frame and returns a
	// surface object wrapping it, suitable for JBUKernel::upsampleToSurface().
	// Must be paired with a matching endCudaAccess() call before the texture is
	// used by GL again (e.g. rendered/sampled/resized) - the mapping is
	// exclusive between GL and CUDA. Returns false (logged) on failure, in which
	// case outSurface is left at 0 and endCudaAccess() must not be called.
	bool beginCudaAccess(CUsurfObject& outSurface);

	// Unmaps the texture, returning ownership to GL. `stream` is the stream any
	// CUDA work between beginCudaAccess()/endCudaAccess() was enqueued on
	// (typically JBUKernel's own stream) - cuGraphicsUnmapResources synchronizes
	// with it internally, so the caller does not need to have already called
	// JBUKernel::synchronize() first. Returns false (logged) on failure.
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
