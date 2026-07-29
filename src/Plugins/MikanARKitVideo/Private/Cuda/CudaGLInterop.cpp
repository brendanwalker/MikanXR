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

// -- CudaGLColorTexture -----
CudaGLColorTexture::CudaGLColorTexture()= default;

CudaGLColorTexture::~CudaGLColorTexture() { shutdown(); }

bool CudaGLColorTexture::init(int width, int height)
{
	if (isInitialized())
	{
		if (width == m_width && height == m_height)
			return true; // already initialized at this size

		MIKAN_LOG_ERROR("CudaGLColorTexture::init")
			<< "Already initialized at a different size - call resize() instead";
		return false;
	}

	if (width <= 0 || height <= 0)
	{
		MIKAN_LOG_ERROR("CudaGLColorTexture::init") << "Invalid size " << width << "x" << height;
		return false;
	}

	m_texture= CreateMkTexture(static_cast<uint16_t>(width), static_cast<uint16_t>(height), nullptr, MK_RGBA, MK_RGBA);
	if (m_texture == nullptr || !m_texture->createTexture())
	{
		MIKAN_LOG_ERROR("CudaGLColorTexture::init") << "Failed to create MK_RGBA texture";
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

bool CudaGLColorTexture::registerWithCuda()
{
	// WRITE_DISCARD | SURFACE_LDST: NV12ConversionKernel overwrites the entire
	// texture every frame via surf2Dwrite, never reads back prior contents.
	const unsigned int registerFlags=
		CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD | CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST;

	return checkCudaResult(
		cuGraphicsGLRegisterImage(&m_graphicsResource, m_texture->getGlTextureId(), GL_TEXTURE_2D, registerFlags),
		"cuGraphicsGLRegisterImage");
}

void CudaGLColorTexture::unregisterFromCuda()
{
	if (m_graphicsResource != nullptr)
	{
		checkCudaResult(cuGraphicsUnregisterResource(m_graphicsResource), "cuGraphicsUnregisterResource");
		m_graphicsResource= nullptr;
	}
}

void CudaGLColorTexture::shutdown()
{
	if (m_currentSurface != 0)
	{
		MIKAN_LOG_WARNING("CudaGLColorTexture::shutdown")
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

bool CudaGLColorTexture::isInitialized() const { return m_texture != nullptr && m_graphicsResource != nullptr; }

int CudaGLColorTexture::getWidth() const { return m_width; }

int CudaGLColorTexture::getHeight() const { return m_height; }

IMkTexturePtr CudaGLColorTexture::getTexture() const { return m_texture; }

bool CudaGLColorTexture::resize(int width, int height)
{
	if (width == m_width && height == m_height && isInitialized())
		return true;

	shutdown();
	return init(width, height);
}

bool CudaGLColorTexture::beginCudaAccess(CUsurfObject& outSurface)
{
	outSurface= 0;

	if (!isInitialized())
	{
		MIKAN_LOG_ERROR("CudaGLColorTexture::beginCudaAccess") << "Not initialized";
		return false;
	}

	if (m_currentSurface != 0)
	{
		MIKAN_LOG_ERROR("CudaGLColorTexture::beginCudaAccess") << "Already mapped - missing endCudaAccess() call";
		return false;
	}

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

bool CudaGLColorTexture::endCudaAccess(CUstream stream)
{
	if (m_currentSurface == 0)
	{
		MIKAN_LOG_ERROR("CudaGLColorTexture::endCudaAccess") << "Not currently mapped - missing beginCudaAccess() call";
		return false;
	}

	const bool destroyOk= checkCudaResult(cuSurfObjectDestroy(m_currentSurface), "cuSurfObjectDestroy");
	m_currentSurface= 0;

	const bool unmapOk=
		checkCudaResult(cuGraphicsUnmapResources(1, &m_graphicsResource, stream), "cuGraphicsUnmapResources");

	return destroyOk && unmapOk;
}
