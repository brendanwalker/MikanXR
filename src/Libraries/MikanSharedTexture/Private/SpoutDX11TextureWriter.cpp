#include "SharedTextureWriterBackend.h"
#include "SharedTextureLogger.h"
#include "SpoutDX.h"
#include "SpoutDXDepthTexturePacker.h"

#include <sstream>
#include <string>

class SpoutDX11TextureWriter : public ISharedTextureWriterBackend
{
public:
	SpoutDX11TextureWriter(const SharedTextureWriterContext& context)
		: m_context(context)
		, m_logger(*context.logger)
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_spoutShadowFrame()
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX11TextureWriter() { dispose(); }

	bool init() override
	{
		const SharedTextureDescriptor* descriptor= m_context.descriptor;
		ID3D11Device* d3d11Device= (ID3D11Device*)m_context.apiDeviceInterface;
		bool bSuccess= true;

		dispose();

		applySpoutDXLogPolicy();

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32
			|| descriptor->color_buffer_type == SharedColorBufferType::BGRA32
			|| descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
		{
			if (m_spoutColorFrame.OpenDirectX11(d3d11Device)
				&& m_spoutColorFrame.SetSenderName(m_context.colorSenderName.c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
				else if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_context.bEnableFrameCounter)
					m_spoutColorFrame.DisableFrameCount();

				m_bIsColorFrameInitialized= true;
			}
			else
			{
				m_logger.log(SharedTextureLogLevel::info,
							 "SpoutDX11TextureWriter::init() - Error initializing color spout frame");
				return false;
			}
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutDX11TextureWriter::init() - color buffer type not supported: ";
			ss << (int)descriptor->color_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			return false;
		}

		// Initialize the depth spout frame, if requested
		if (descriptor->depth_buffer_type != SharedDepthBufferType::NODEPTH)
		{
			if (m_spoutDepthFrame.OpenDirectX11(d3d11Device)
				&& m_spoutDepthFrame.SetSenderName(m_context.depthSenderName.c_str()))
			{
				// Initialize the depth texture packer if we are sending float depth textures
				if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
					|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
				{
					m_depthTexturePacker= new SpoutDXDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
					if (!m_depthTexturePacker->init())
					{
						m_logger.log(SharedTextureLogLevel::info,
									 "SpoutDX11TextureWriter::init() - Error initializing float depth packer");
						return false;
					}
				}

				m_spoutDepthFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_context.bEnableFrameCounter)
					m_spoutDepthFrame.DisableFrameCount();

				m_bIsDepthFrameInitialized= true;
			}
			else
			{
				m_logger.log(SharedTextureLogLevel::info,
							 "SpoutDX11TextureWriter::init() - Error initializing depth spout frame");
				return false;
			}
		}

		// Initialize the (optional) shadow spout frame. It's a color-like RGBA8/BGRA8 buffer.
		if (descriptor->shadow_buffer_type != SharedShadowBufferType::NOSHADOW)
		{
			if (m_spoutShadowFrame.OpenDirectX11(d3d11Device)
				&& m_spoutShadowFrame.SetSenderName(m_context.shadowSenderName.c_str()))
			{
				if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
				else if (descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32)
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_context.bEnableFrameCounter)
					m_spoutShadowFrame.DisableFrameCount();

				m_bIsShadowFrameInitialized= true;
			}
			else
			{
				m_logger.log(SharedTextureLogLevel::info,
							 "SpoutDX11TextureWriter::init() - Error initializing shadow spout frame");
				return false;
			}
		}

		return true;
	}

	void dispose()
	{
		if (m_depthTexturePacker != nullptr)
		{
			delete m_depthTexturePacker;
			m_depthTexturePacker= nullptr;
		}

		m_spoutColorFrame.ReleaseSender();
		m_spoutColorFrame.CloseDirectX11();
		m_bIsColorFrameInitialized= false;

		m_spoutDepthFrame.ReleaseSender();
		m_spoutDepthFrame.CloseDirectX11();
		m_bIsDepthFrameInitialized= false;

		m_spoutShadowFrame.ReleaseSender();
		m_spoutShadowFrame.CloseDirectX11();
		m_bIsShadowFrameInitialized= false;

		DisableSpoutLog();
	}

	bool writeColorFrameTexture(void* apiTexturePtr) override
	{
		ID3D11Texture2D* pTexture= (ID3D11Texture2D*)apiTexturePtr;

		return m_bIsColorFrameInitialized ? m_spoutColorFrame.SendTexture(pTexture) : false;
	}

	bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar) override
	{
		ID3D11Texture2D* pTexture= (ID3D11Texture2D*)apiTexturePtr;

		if (m_bIsDepthFrameInitialized)
		{
			if (m_depthTexturePacker != nullptr)
			{
				// Convert the float depth texture to a RGBA8 texture using a shader
				// (Spout can only send RGBA8 textures)
				ID3D11Texture2D* packedDepthTexture= m_depthTexturePacker->packDepthTexture(pTexture, zNear, zFar);

				if (packedDepthTexture != nullptr)
				{
					return m_spoutDepthFrame.SendTexture(packedDepthTexture);
				}
			}
			else
			{
				m_spoutDepthFrame.SendTexture(pTexture);
			}
		}

		return false;
	}

	bool writeShadowFrameTexture(void* apiTexturePtr) override
	{
		ID3D11Texture2D* pTexture= (ID3D11Texture2D*)apiTexturePtr;

		return m_bIsShadowFrameInitialized ? m_spoutShadowFrame.SendTexture(pTexture) : false;
	}

	void* getPackDepthTextureResourcePtr() const override
	{
		return m_depthTexturePacker != nullptr ? m_depthTexturePacker->getPackedDepthTextureResourcePtr() : nullptr;
	}

private:
	const SharedTextureWriterContext& m_context;
	SharedTextureLogger& m_logger;
	spoutDX m_spoutColorFrame;
	spoutDX m_spoutDepthFrame;
	spoutDX m_spoutShadowFrame;
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
	bool m_bIsShadowFrameInitialized= false;
};

ISharedTextureWriterBackendPtr createDX11TextureWriter(const SharedTextureWriterContext& context)
{
	return std::make_unique<SpoutDX11TextureWriter>(context);
}
