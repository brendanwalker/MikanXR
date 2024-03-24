#include "InterprocessRenderTargetWriter.h"
#include "Logger.h"
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
		: m_senderName(clientName)
		, m_spout(nullptr)
	{
		memset(&m_descriptor, 0, sizeof(MikanRenderTargetDescriptor));
	}

	virtual ~SpoutOpenGLTextureWriter()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor, bool bEnableFrameCounter)
	{
		dispose();

		m_spout = GetSpout();
		if (m_spout == nullptr)
		{
			MIKAN_LOG_ERROR("SpoutTextureWriter") << "Failed to open spout api";
			return false;
		}

		if (descriptor->color_buffer_type == MikanColorBuffer_RGBA32 ||
			descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
		{
			m_spout->EnableSpoutLog();
			m_spout->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spout->SetSenderName(m_senderName.c_str());

			if (descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
				m_spout->SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spout->SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spout->SetFrameCount(bEnableFrameCounter);
		}
		else
		{
			MIKAN_LOG_INFO("SpoutOpenGLTextureWriter::init()") << "color buffer type not supported: " << descriptor->color_buffer_type;
			return false;
		}

		m_descriptor= *descriptor;

		return true;
	}

	void dispose()
	{
		if (m_spout != nullptr)
		{
			m_spout->Release();
			m_spout = nullptr;
		}
	}

	bool writeRenderTargetTexture(GLuint textureID)
	{
		return m_spout->SendTexture(textureID, GL_TEXTURE_2D, m_descriptor.width, m_descriptor.height);
	}

private:
	std::string m_senderName;
	MikanRenderTargetDescriptor m_descriptor;
	SPOUTLIBRARY* m_spout;
};

#ifdef ENABLE_SPOUT_DX
class SpoutDX11TextureWriter
{
public:
	SpoutDX11TextureWriter::SpoutDX11TextureWriter(
		const std::string& clientName)
		: m_senderName(clientName)
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

		if (descriptor->color_buffer_type == MikanColorBuffer_RGBA32 ||
			descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
		{
			EnableSpoutLog();
			EnableSpoutLogFile("sender.log");
			SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

			m_spout.OpenDirectX11(d3d11Device);
			m_spout.SetSenderName(m_senderName.c_str());
			
			if (descriptor->color_buffer_type == MikanColorBuffer_BGRA32)
				m_spout.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spout.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

			if (!bEnableFrameCounter)
				m_spout.DisableFrameCount();
		}
		else
		{
			MIKAN_LOG_INFO("SpoutDX11TextureWriter::init()") << "color buffer type not supported: " << descriptor->color_buffer_type;
			bSuccess= false;
		}

		return bSuccess;
	}

	void dispose()
	{
		m_spout.ReleaseSender();
		m_spout.CloseDirectX11();

		DisableSpoutLog();
	}

	bool writeRenderTargetTexture(ID3D11Texture2D* pTexture)
	{
		return m_spout.SendTexture(pTexture);
	}

private:
	std::string m_senderName;
	spoutDX m_spout;
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

bool InterprocessRenderTargetWriteAccessor::writeRenderTargetTexture(void* apiTexturePtr)
{
	bool bSuccess = false;

	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		GLuint* textureId= (GLuint*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutOpenGLTextureWriter->writeRenderTargetTexture(*textureId);
	}
#ifdef ENABLE_SPOUT_DX
	else if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		ID3D11Texture2D* dx11Texture = (ID3D11Texture2D*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutDX11TextureWriter->writeRenderTargetTexture(dx11Texture);
	}
#endif // ENABLE_SPOUT_DX
	else
	{
		bSuccess = false;
	}

	return true;
}