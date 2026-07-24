#include "CudaGLInterop.h"
#include "CudaErrorHandling.h"
#include "Logger.h"

// GLEW must be included before any header that pulls in a system GL header
// (cudaGL.h does, transitively) - see MikanARKitVideoDevice.cpp's equivalent note
// for the windows.h/GL.h ordering issue this avoids. GLEW itself is already
// initialized process-wide by the time any video-source update() tick runs (see
// GlGraphicsContext::startup()), so this translation unit doesn't call
// glewInit() - it only needs GLEW's type/macro definitions (GLuint, GL_TEXTURE_2D).
#include <GL/glew.h>

#include <cudaGL.h>

#include "IMkTexture.h"

CudaGLDepthTexture::CudaGLDepthTexture()= default;

CudaGLDepthTexture::~CudaGLDepthTexture() { shutdown(); }

bool CudaGLDepthTexture::init(int width, int height)
{
	if (isInitialized())
	{
		if (width == m_width && height == m_height)
			return true; // already initialized at this size

		MIKAN_LOG_ERROR("CudaGLDepthTexture::init")
			<< "Already initialized at a different size - call resize() instead";
		return false;
	}

	if (width <= 0 || height <= 0)
	{
		MIKAN_LOG_ERROR("CudaGLDepthTexture::init") << "Invalid size " << width << "x" << height;
		return false;
	}

	m_texture= CreateMkTexture(static_cast<uint16_t>(width), static_cast<uint16_t>(height), nullptr, MK_R32F, MK_RED);
	if (m_texture == nullptr || !m_texture->createTexture())
	{
		MIKAN_LOG_ERROR("CudaGLDepthTexture::init") << "Failed to create MK_R32F texture";
		m_texture.reset();
		return false;
	}

	m_width= width;
	m_height= height;

	if (!registerWithCuda())
	{
		m_texture->disposeTexture();
		m_texture.reset();
		m_width= 0;
		m_height= 0;
		return false;
	}

	return true;
}

bool CudaGLDepthTexture::registerWithCuda()
{
	// WRITE_DISCARD: the JBU kernel overwrites the entire texture every frame, so
	// CUDA never needs to preserve/read back prior contents. SURFACE_LDST: this
	// texture is written via surf2Dwrite (JBUKernel::upsampleToSurface), not
	// sampled as a CUDA texture reference - both hints are required together for
	// a surface-object-writable registration (see cudaGL.h's
	// cuGraphicsGLRegisterImage documentation).
	const unsigned int registerFlags=
		CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD | CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST;

	return checkCudaResult(
		cuGraphicsGLRegisterImage(&m_graphicsResource, m_texture->getGlTextureId(), GL_TEXTURE_2D, registerFlags),
		"cuGraphicsGLRegisterImage");
}

void CudaGLDepthTexture::unregisterFromCuda()
{
	if (m_graphicsResource != nullptr)
	{
		checkCudaResult(cuGraphicsUnregisterResource(m_graphicsResource), "cuGraphicsUnregisterResource");
		m_graphicsResource= nullptr;
	}
}

void CudaGLDepthTexture::shutdown()
{
	// A leftover mapped surface here would mean beginCudaAccess() was called
	// without a matching endCudaAccess() - defensively tear it down rather than
	// leaking the CUDA-side registration in an inconsistent state.
	if (m_currentSurface != 0)
	{
		MIKAN_LOG_WARNING("CudaGLDepthTexture::shutdown")
			<< "Shutting down with an in-flight CUDA mapping (missing endCudaAccess() call) - cleaning up anyway";
		cuSurfObjectDestroy(m_currentSurface);
		m_currentSurface= 0;
		if (m_graphicsResource != nullptr)
			cuGraphicsUnmapResources(1, &m_graphicsResource, nullptr);
	}

	unregisterFromCuda();

	if (m_texture != nullptr)
	{
		m_texture->disposeTexture();
		m_texture.reset();
	}

	m_width= 0;
	m_height= 0;
}

bool CudaGLDepthTexture::isInitialized() const { return m_texture != nullptr && m_graphicsResource != nullptr; }

int CudaGLDepthTexture::getWidth() const { return m_width; }

int CudaGLDepthTexture::getHeight() const { return m_height; }

IMkTexturePtr CudaGLDepthTexture::getTexture() const { return m_texture; }

bool CudaGLDepthTexture::resize(int width, int height)
{
	if (width == m_width && height == m_height && isInitialized())
		return true;

	// No partial-resize path in CUDA-GL interop - the registration is tied to a
	// specific GL texture object, so the only option is a full teardown/rebuild
	// (see class comment's edge-case note).
	shutdown();
	return init(width, height);
}

bool CudaGLDepthTexture::beginCudaAccess(CUsurfObject& outSurface)
{
	outSurface= 0;

	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("CudaGLDepthTexture::beginCudaAccess") << "Not initialized";
		return false;
	}

	if (m_currentSurface != 0)
	{
		MIKAN_LOG_ERROR("CudaGLDepthTexture::beginCudaAccess") << "Already mapped - missing endCudaAccess() call";
		return false;
	}

	// The map itself is enqueued on the default stream (nullptr) - it's a
	// synchronization point with GL's own usage of the texture, not part of the
	// JBU kernel's own async pipeline, so there's no benefit to using JBUKernel's
	// dedicated stream here.
	if (!checkCudaResult(cuGraphicsMapResources(1, &m_graphicsResource, nullptr), "cuGraphicsMapResources"))
		return false;

	CUarray mappedArray= nullptr;
	if (!checkCudaResult(cuGraphicsSubResourceGetMappedArray(&mappedArray, m_graphicsResource, 0, 0),
						 "cuGraphicsSubResourceGetMappedArray"))
	{
		cuGraphicsUnmapResources(1, &m_graphicsResource, nullptr);
		return false;
	}

	CUDA_RESOURCE_DESC resourceDesc= {};
	resourceDesc.resType= CU_RESOURCE_TYPE_ARRAY;
	resourceDesc.res.array.hArray= mappedArray;

	if (!checkCudaResult(cuSurfObjectCreate(&m_currentSurface, &resourceDesc), "cuSurfObjectCreate"))
	{
		m_currentSurface= 0;
		cuGraphicsUnmapResources(1, &m_graphicsResource, nullptr);
		return false;
	}

	outSurface= m_currentSurface;
	return true;
}

bool CudaGLDepthTexture::endCudaAccess(CUstream stream)
{
	if (m_currentSurface == 0)
	{
		MIKAN_LOG_ERROR("CudaGLDepthTexture::endCudaAccess") << "Not currently mapped - missing beginCudaAccess() call";
		return false;
	}

	// Destroying the surface object first is required - it must not outlive the
	// mapping it was created from.
	const bool destroyOk= checkCudaResult(cuSurfObjectDestroy(m_currentSurface), "cuSurfObjectDestroy");
	m_currentSurface= 0;

	const bool unmapOk=
		checkCudaResult(cuGraphicsUnmapResources(1, &m_graphicsResource, stream), "cuGraphicsUnmapResources");

	return destroyOk && unmapOk;
}
