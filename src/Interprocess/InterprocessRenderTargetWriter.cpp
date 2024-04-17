#include "InterprocessRenderTargetWriter.h"
#include "Logger.h"
#include "MikanCoreTypes.h"
#ifdef ENABLE_SPOUT_DX
#include "SpoutDX.h"
#endif // ENABLE_SPOUT_DX
#include "SpoutLibrary.h"

// Define DXGI_FORMAT enum values ourselves if we don't use SPOUTDX API
#ifndef ENABLE_SPOUT_DX
enum DXGI_FORMAT
{
	DXGI_FORMAT_R8G8B8A8_UNORM                          = 28,
    DXGI_FORMAT_B8G8R8A8_UNORM                          = 87,
};
#endif // !ENABLE_SPOUT_DX

class SpoutOpenGLTextureWriter
{
public:
	SpoutOpenGLTextureWriter(const std::string& clientName)
		: m_colorFrameSenderName(clientName+"_color")
		, m_depthFrameSenderName(clientName+"_depth")
		, m_spoutColorFrame(nullptr)
		, m_spoutDepthFrame(nullptr)
	{
		memset(&m_descriptor, 0, sizeof(MikanRenderTargetDescriptor));
	}

	virtual ~SpoutOpenGLTextureWriter()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor, bool bEnableFrameCounter)
	{
		bool bSuccess = true;

		dispose();

		m_spoutColorFrame = GetSpout();
		if (m_spoutColorFrame == nullptr)
		{
			MIKAN_LOG_ERROR("SpoutTextureWriter") << "Failed to open spout api";
			return false;
		}

		if (descriptor->color_buffer_type == MikanColorBuffer_RGBA32 ||
			descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
		{
			m_spoutColorFrame->EnableSpoutLog();
			m_spoutColorFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutColorFrame->SetSenderName(m_colorFrameSenderName.c_str());

			if (descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
				m_spoutColorFrame->SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutColorFrame->SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutColorFrame->SetFrameCount(bEnableFrameCounter);
		}
		else
		{
			MIKAN_LOG_INFO("SpoutOpenGLTextureWriter::init()") << "color buffer type not supported: " << descriptor->color_buffer_type;
			bSuccess= false;
		}

		if (descriptor->depth_buffer_type == MikanDepthBuffer_PACK_DEPTH_RGBA)
		{
			m_spoutDepthFrame->EnableSpoutLog();
			m_spoutDepthFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutDepthFrame->SetSenderName(m_depthFrameSenderName.c_str());
			m_spoutDepthFrame->SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
			m_spoutDepthFrame->SetFrameCount(bEnableFrameCounter);
		}
		else if (descriptor->depth_buffer_type == MikanDepthBuffer_NODEPTH)
		{
			m_spoutDepthFrame= nullptr;
		}
		else
		{
			MIKAN_LOG_INFO("SpoutDX11TextureWriter::init()") << "depth buffer type not supported: " << descriptor->depth_buffer_type;
			bSuccess = false;
		}

		m_descriptor= *descriptor;

		return bSuccess;
	}

	void dispose()
	{
		if (m_spoutColorFrame != nullptr)
		{
			m_spoutColorFrame->Release();
			m_spoutColorFrame = nullptr;
		}

		if (m_spoutDepthFrame != nullptr)
		{
			m_spoutDepthFrame->Release();
			m_spoutDepthFrame = nullptr;
		}
	}

	bool writeColorFrameTexture(GLuint textureID)
	{
		if (m_spoutColorFrame != nullptr && m_spoutColorFrame->IsInitialized())
		{
			return m_spoutColorFrame->SendTexture(textureID, GL_TEXTURE_2D, m_descriptor.width, m_descriptor.height);
		}

		return false;
	}

	bool writeDepthFrameTexture(GLuint textureID)
	{
		if (m_spoutDepthFrame != nullptr && m_spoutDepthFrame->IsInitialized())
		{
			return m_spoutDepthFrame->SendTexture(textureID, GL_TEXTURE_2D, m_descriptor.width, m_descriptor.height);
		}

		return false;
	}

private:
	std::string m_colorFrameSenderName;
	std::string m_depthFrameSenderName;
	MikanRenderTargetDescriptor m_descriptor;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
};

#ifdef ENABLE_SPOUT_DX
class SpoutDX11TextureWriter
{
public:
	SpoutDX11TextureWriter::SpoutDX11TextureWriter(
		const std::string& clientName)
		: m_colorFrameSenderName(clientName+"_color")
		, m_depthFrameSenderName(clientName+"_depth")
	{
	}

	virtual ~SpoutDX11TextureWriter()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor, bool bEnableFrameCounter, void* apiDeviceInterface)
	{
		ID3D11Device* d3d11Device= (ID3D11Device*)apiDeviceInterface;
		bool bSuccess = true;

		dispose();

		EnableSpoutLog();
		EnableSpoutLogFile("sender.log");
		SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

		if (descriptor->color_buffer_type == MikanColorBuffer_RGBA32 ||
			descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
		{
			m_spoutColorFrame.OpenDirectX11(d3d11Device);
			m_spoutColorFrame.SetSenderName(m_colorFrameSenderName.c_str());
			
			if (descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
				m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

			if (!bEnableFrameCounter)
				m_spoutColorFrame.DisableFrameCount();
		}
		else
		{
			MIKAN_LOG_INFO("SpoutDX11TextureWriter::init()") << "color buffer type not supported: " << descriptor->color_buffer_type;
			bSuccess= false;
		}

		if (descriptor->depth_buffer_type == MikanDepthBuffer_PACK_DEPTH_RGBA)
		{
			m_spoutDepthFrame.OpenDirectX11(d3d11Device);
			m_spoutDepthFrame.SetSenderName(m_depthFrameSenderName.c_str());

			m_spoutDepthFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

			if (!bEnableFrameCounter)
				m_spoutDepthFrame.DisableFrameCount();
		}
		else
		{
			MIKAN_LOG_INFO("SpoutDX11TextureWriter::init()") << "depth buffer type not supported: " << descriptor->depth_buffer_type;
			bSuccess = false;
		}

		return bSuccess;
	}

	void dispose()
	{
		m_spoutColorFrame.ReleaseSender();
		m_spoutColorFrame.CloseDirectX11();

		m_spoutDepthFrame.ReleaseSender();
		m_spoutDepthFrame.CloseDirectX11();

		DisableSpoutLog();
	}

	bool writeColorFrameTexture(ID3D11Texture2D* pTexture)
	{
		return m_spoutColorFrame.IsInitialized() ? m_spoutColorFrame.SendTexture(pTexture) : false;
	}

	bool writeDepthFrameTexture(ID3D11Texture2D* pTexture)
	{
		return m_spoutColorFrame.IsInitialized() ? m_spoutDepthFrame.SendTexture(pTexture) : false;
	}

private:
	std::string m_colorFrameSenderName;
	std::string m_depthFrameSenderName;
	spoutDX m_spoutColorFrame;
	spoutDX m_spoutDepthFrame;
};
#endif // ENABLE_SPOUT_DX


//-- InterprocessRenderTargetWriteAccessor -----
struct RenderTargetWriterImpl
{
	union
	{	
#ifdef ENABLE_SPOUT_DX
		SpoutDX11TextureWriter* spoutDX11TextureWriter;
#endif // ENABLE_SPOUT_DX
		SpoutOpenGLTextureWriter* spoutOpenGLTextureWriter;
	} writerApi;
	MikanClientGraphicsApi graphicsAPI;
};

InterprocessRenderTargetWriteAccessor::InterprocessRenderTargetWriteAccessor(const std::string& clientName)
	: m_clientName(clientName)
	, m_writerImpl(new RenderTargetWriterImpl)
{
	memset(m_writerImpl, 0, sizeof(RenderTargetWriterImpl));
}

InterprocessRenderTargetWriteAccessor::~InterprocessRenderTargetWriteAccessor()
{
	dispose();
	delete m_writerImpl;
}

bool InterprocessRenderTargetWriteAccessor::initialize(
	const MikanRenderTargetDescriptor* descriptor,
	bool bEnableFrameCounter,
	void* apiDeviceInterface)
{
	dispose();

	m_bWantsDepth= descriptor->depth_buffer_type != MikanDepthBuffer_NODEPTH;

	if (descriptor->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		m_writerImpl->writerApi.spoutOpenGLTextureWriter = new SpoutOpenGLTextureWriter(m_clientName);
		m_writerImpl->graphicsAPI = MikanClientGraphicsApi_OpenGL;

		m_bIsInitialized = m_writerImpl->writerApi.spoutOpenGLTextureWriter->init(descriptor, bEnableFrameCounter);
	}
#ifdef ENABLE_SPOUT_DX
	else if (descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		m_writerImpl->writerApi.spoutDX11TextureWriter = new SpoutDX11TextureWriter(m_clientName);
		m_writerImpl->graphicsAPI = MikanClientGraphicsApi_Direct3D11;

		m_bIsInitialized = m_writerImpl->writerApi.spoutDX11TextureWriter->init(descriptor, bEnableFrameCounter, apiDeviceInterface);
	}
#endif // ENABLE_SPOUT_DX

	return m_bIsInitialized;
}

void InterprocessRenderTargetWriteAccessor::dispose()
{
	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		if (m_writerImpl->writerApi.spoutOpenGLTextureWriter != nullptr)
		{
			m_writerImpl->writerApi.spoutOpenGLTextureWriter->dispose();
			delete m_writerImpl->writerApi.spoutOpenGLTextureWriter;
			m_writerImpl->writerApi.spoutOpenGLTextureWriter = nullptr;
		}
	}
#ifdef ENABLE_SPOUT_DX
	else if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		if (m_writerImpl->writerApi.spoutDX11TextureWriter != nullptr)
		{
			m_writerImpl->writerApi.spoutDX11TextureWriter->dispose();
			delete m_writerImpl->writerApi.spoutDX11TextureWriter;
			m_writerImpl->writerApi.spoutDX11TextureWriter = nullptr;
		}
	}
#endif // ENABLE_SPOUT_DX

	m_writerImpl->graphicsAPI= MikanClientGraphicsApi_UNKNOWN;
	m_bIsInitialized = false;
}

bool InterprocessRenderTargetWriteAccessor::writeColorFrameTexture(void* apiTexturePtr)
{
	bool bSuccess = false;

	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		GLuint* textureId= (GLuint*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutOpenGLTextureWriter->writeColorFrameTexture(*textureId);
	}
#ifdef ENABLE_SPOUT_DX
	else if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		ID3D11Texture2D* dx11Texture = (ID3D11Texture2D*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutDX11TextureWriter->writeColorFrameTexture(dx11Texture);
	}
#endif // ENABLE_SPOUT_DX
	else
	{
		bSuccess = false;
	}

	return true;
}

bool InterprocessRenderTargetWriteAccessor::writeDepthFrameTexture(void* apiTexturePtr)
{
	bool bSuccess = false;

	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		GLuint* textureId = (GLuint*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutOpenGLTextureWriter->writeDepthFrameTexture(*textureId);
	}
#ifdef ENABLE_SPOUT_DX
	else if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		ID3D11Texture2D* dx11Texture = (ID3D11Texture2D*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutDX11TextureWriter->writeDepthFrameTexture(dx11Texture);
	}
#endif // ENABLE_SPOUT_DX
	else
	{
		bSuccess = false;
	}

	return true;
}