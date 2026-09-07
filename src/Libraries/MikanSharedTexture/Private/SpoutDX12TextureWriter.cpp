#include "SharedTextureWriterBackend.h"
#include "SharedTextureLogger.h"
#include "SpoutDX.h"
#include "SpoutDX12.h"
#include "SpoutDXDepthTexturePacker.h"

#include <sstream>
#include <string>

class SpoutDX12TextureWriter : public ISharedTextureWriterBackend
{
public:
	SpoutDX12TextureWriter(const SharedTextureWriterContext& context)
		: m_context(context)
		, m_logger(*context.logger)
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_spoutShadowFrame()
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX12TextureWriter() { dispose(); }

	bool init() override
	{
		const SharedTextureDescriptor* descriptor= m_context.descriptor;
		ID3D12Device* d3d12Device= (ID3D12Device*)m_context.apiDeviceInterface;
		// Optional client command queue. When supplied, the D3D11On12 device shares it so the
		// wrapped-resource copy is GPU-ordered after the client's rendering (no flicker/tearing).
		IUnknown* commandQueue= (IUnknown*)m_context.apiCommandQueueInterface;
		IUnknown** ppCommandQueue= (commandQueue != nullptr) ? &commandQueue : nullptr;
		bool bSuccess= true;

		dispose();

		applySpoutDXLogPolicy();

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32
			|| descriptor->color_buffer_type == SharedColorBufferType::BGRA32
			|| descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
		{
			if (m_spoutColorFrame.OpenDirectX12(d3d12Device, ppCommandQueue)
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
			if (m_spoutDepthFrame.OpenDirectX12(d3d12Device, ppCommandQueue)
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
			if (m_spoutShadowFrame.OpenDirectX12(d3d12Device, ppCommandQueue)
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
							 "SpoutDX12TextureWriter::init() - Error initializing shadow spout frame");
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
		m_spoutColorFrame.CloseDirectX12();
		m_bIsColorFrameInitialized= false;

		m_spoutDepthFrame.ReleaseSender();
		m_spoutDepthFrame.CloseDirectX12();
		m_bIsDepthFrameInitialized= false;

		m_spoutShadowFrame.ReleaseSender();
		m_spoutShadowFrame.CloseDirectX12();
		m_bIsShadowFrameInitialized= false;

		DisableSpoutLog();
	}

	bool writeColorFrameTexture(void* apiTexturePtr) override
	{
		ID3D12Resource* dx12TextureResource= (ID3D12Resource*)apiTexturePtr;

		bool bSuccess= false;

		if (m_bIsColorFrameInitialized)
		{
			if (m_spoutDX12ColorTexture != dx12TextureResource)
			{
				if (m_spoutDX11ColorTexture != nullptr)
				{
					m_spoutDX11ColorTexture->Release();
					m_spoutDX11ColorTexture= nullptr;
				}

				// Wrap as GENERIC_READ to match the rest state (SRVMask) the client leaves the
				// staging texture in. Combined with InState == OutState in WrapDX12Resource, this
				// keeps 11on12 from issuing barriers that conflict with the client's state tracker.
				if (dx12TextureResource != nullptr
					&& m_spoutColorFrame.WrapDX12Resource(dx12TextureResource, &m_spoutDX11ColorTexture,
														  D3D12_RESOURCE_STATE_GENERIC_READ))
				{
					m_spoutDX12ColorTexture= dx12TextureResource;
				}
			}

			if (m_spoutDX11ColorTexture != nullptr)
			{
				bSuccess= m_spoutColorFrame.SendDX11Resource(m_spoutDX11ColorTexture);
			}
		}

		return bSuccess;
	}

	bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar) override
	{
		ID3D12Resource* dx12TextureResource= (ID3D12Resource*)apiTexturePtr;

		bool bSuccess= false;

		if (m_bIsDepthFrameInitialized)
		{
			if (m_spoutDX12DepthTexture != dx12TextureResource)
			{
				if (m_spoutDX11DepthTexture != nullptr)
				{
					m_spoutDX11DepthTexture->Release();
					m_spoutDX11DepthTexture= nullptr;
				}

				// See note in writeColorFrameTexture: GENERIC_READ matches the staging texture's
				// SRVMask rest state so 11on12 never conflicts with the client's state tracker.
				if (dx12TextureResource != nullptr
					&& m_spoutDepthFrame.WrapDX12Resource(dx12TextureResource, &m_spoutDX11DepthTexture,
														  D3D12_RESOURCE_STATE_GENERIC_READ))
				{
					m_spoutDX12DepthTexture= dx12TextureResource;
				}
			}

			if (m_spoutDX11DepthTexture != nullptr)
			{
				if (m_depthTexturePacker != nullptr)
				{
					// Convert the float depth texture to a RGBA8 texture using a shader
					// (Spout can only send RGBA8 textures)
					ID3D11Texture2D* pTexture11= (ID3D11Texture2D*)m_spoutDX11DepthTexture;
					ID3D11Texture2D* packedDepthTexture=
						m_depthTexturePacker->packDepthTexture(pTexture11, zNear, zFar);

					if (packedDepthTexture != nullptr)
					{
						bSuccess= m_spoutDepthFrame.SendTexture(packedDepthTexture);
					}
				}
				else
				{
					bSuccess= m_spoutDepthFrame.SendDX11Resource(m_spoutDX11DepthTexture);
				}
			}
		}

		return bSuccess;
	}

	bool writeShadowFrameTexture(void* apiTexturePtr) override
	{
		ID3D12Resource* dx12TextureResource= (ID3D12Resource*)apiTexturePtr;

		bool bSuccess= false;

		if (m_bIsShadowFrameInitialized)
		{
			if (m_spoutDX12ShadowTexture != dx12TextureResource)
			{
				if (m_spoutDX11ShadowTexture != nullptr)
				{
					m_spoutDX11ShadowTexture->Release();
					m_spoutDX11ShadowTexture= nullptr;
				}

				// See note in writeColorFrameTexture: GENERIC_READ matches the staging texture's
				// SRVMask rest state so 11on12 never conflicts with the client's state tracker.
				if (dx12TextureResource != nullptr
					&& m_spoutShadowFrame.WrapDX12Resource(dx12TextureResource, &m_spoutDX11ShadowTexture,
														   D3D12_RESOURCE_STATE_GENERIC_READ))
				{
					m_spoutDX12ShadowTexture= dx12TextureResource;
				}
			}

			if (m_spoutDX11ShadowTexture != nullptr)
			{
				bSuccess= m_spoutShadowFrame.SendDX11Resource(m_spoutDX11ShadowTexture);
			}
		}

		return bSuccess;
	}

	void* getPackDepthTextureResourcePtr() const override
	{
		return m_depthTexturePacker != nullptr ? m_depthTexturePacker->getPackedDepthTextureResourcePtr() : nullptr;
	}

private:
	const SharedTextureWriterContext& m_context;
	SharedTextureLogger& m_logger;
	spoutDX12 m_spoutColorFrame;
	ID3D12Resource* m_spoutDX12ColorTexture= nullptr;
	ID3D11Resource* m_spoutDX11ColorTexture= nullptr;
	spoutDX12 m_spoutDepthFrame;
	ID3D12Resource* m_spoutDX12DepthTexture= nullptr;
	ID3D11Resource* m_spoutDX11DepthTexture= nullptr;
	spoutDX12 m_spoutShadowFrame;
	ID3D12Resource* m_spoutDX12ShadowTexture= nullptr;
	ID3D11Resource* m_spoutDX11ShadowTexture= nullptr;
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
	bool m_bIsShadowFrameInitialized= false;
};

ISharedTextureWriterBackendPtr createDX12TextureWriter(const SharedTextureWriterContext& context)
{
	return std::make_unique<SpoutDX12TextureWriter>(context);
}
