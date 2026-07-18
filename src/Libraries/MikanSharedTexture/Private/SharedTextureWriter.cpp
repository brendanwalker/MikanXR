#include "SharedTextureWriter.h"
#include "SpoutDX.h"
#include "SpoutDX12.h"
#include "SpoutDXDepthTexturePacker.h"
#include "SpoutGLDepthTexturePacker.h"
#include "SpoutLibrary.h"
#include "SharedTextureLogger.h"
#include "SharedTextureUtility.h"

#include "assert.h"
#include <sstream>

// -- SharedTextureWriteAccessor -----
class SharedTextureWriteAccessor : public ISharedTextureWriteAccessor
{
public:
	SharedTextureWriteAccessor(const std::string& senderPrefix, MikanCameraID cameraId);
	~SharedTextureWriteAccessor();

	inline SharedTextureLogger& getLogger() { return m_logger; }
	inline SharedClientGraphicsApi getGraphicsApi() const { return m_graphicsAPI; }
	inline void* getApiDeviceInterface() const { return m_apiDeviceInterface; }
	inline void* getApiCommandQueueInterface() const { return m_apiCommandQueueInterface; }
	inline const std::string& getColorSenderName() const { return m_colorSenderName; }
	inline const std::string& getDepthSenderName() const { return m_depthSenderName; }
	inline const std::string& getShadowSenderName() const { return m_shadowSenderName; }
	inline bool getEnableFrameCounter() const { return m_bEnableFrameCounter; }

	virtual bool getIsInitialized() const;
	virtual void setLogCallback(SharedTextureLogCallback callback) override;

	virtual bool initialize(const SharedTextureDescriptor* descriptor, bool bEnableFrameCounter,
							void* apiDeviceInterface, void* apiCommandQueueInterface) override;
	virtual void dispose() override;

	virtual const SharedTextureDescriptor* getRenderTargetDescriptor() const override;
	virtual bool writeColorFrameTexture(void* apiTexturePtr) override;
	virtual bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar) override;
	virtual bool writeShadowFrameTexture(void* apiTexturePtr) override;
	virtual void* getPackDepthTextureResourcePtr() const override;

private:
	bool m_bIsInitialized= false;
	const std::string m_senderPrefix;
	MikanCameraID m_cameraId= -1;
	std::string m_colorSenderName;
	std::string m_depthSenderName;
	std::string m_shadowSenderName;
	SharedTextureDescriptor m_renderTargetDescriptor;
	bool m_bEnableFrameCounter= false;
	void* m_apiDeviceInterface= nullptr;
	// Optional native command queue (e.g. ID3D12CommandQueue*) the client renders on. When provided
	// for D3D12, the D3D11On12 device is created against this queue so the shared-texture copy is
	// serialized on the GPU after the client's rendering, instead of racing it on a separate queue.
	void* m_apiCommandQueueInterface= nullptr;
	union
	{
		class SpoutDX11TextureWriter* spoutDX11TextureWriter;
		class SpoutDX12TextureWriter* spoutDX12TextureWriter;
		class SpoutOpenGLTextureWriter* spoutOpenGLTextureWriter;
	} m_writerApi;
	SharedClientGraphicsApi m_graphicsAPI;
	SharedTextureLogger m_logger;
};

// -- SpoutOpenGLTextureWriter -----
class SpoutOpenGLTextureWriter
{
public:
	SpoutOpenGLTextureWriter(SharedTextureWriteAccessor* parentAccessor)
		: m_parentAccessor(parentAccessor)
		, m_logger(parentAccessor->getLogger())
		, m_spoutColorFrame(nullptr)
		, m_spoutDepthFrame(nullptr)
		, m_spoutShadowFrame(nullptr)
	{
	}

	virtual ~SpoutOpenGLTextureWriter() { dispose(); }

	bool init()
	{
		const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();
		bool bSuccess= true;

		dispose();

		m_spoutColorFrame= GetSpout();
		if (m_spoutColorFrame == nullptr)
		{
			m_logger.log(SharedTextureLogLevel::error, "SpoutTextureWriter - Failed to open spout api");
			return false;
		}

		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32
			|| descriptor->color_buffer_type == SharedColorBufferType::BGRA32
			|| descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
		{
			m_spoutColorFrame->EnableSpoutLog();
			m_spoutColorFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutColorFrame->SetSenderName(m_parentAccessor->getColorSenderName().c_str());

			if (descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R16G16B16A16_FLOAT);
			else if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutColorFrame->SetFrameCount(m_parentAccessor->getEnableFrameCounter());
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - color buffer type not supported: ";
			ss << (int)descriptor->color_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		if (descriptor->depth_buffer_type == SharedDepthBufferType::PACK_DEPTH_RGBA
			|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
			|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
		{
			m_spoutDepthFrame= GetSpout();
			if (m_spoutDepthFrame == nullptr)
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutOpenGLTextureWriter::init() - Failed to open spout api for depth");
				return false;
			}

			m_spoutDepthFrame->EnableSpoutLog();
			m_spoutDepthFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutDepthFrame->SetSenderName(m_parentAccessor->getDepthSenderName().c_str());

			// Initialize the depth texture packer if we are sending float depth textures
			if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
				|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
			{
				m_depthTexturePacker= new SpoutGLDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
				if (!m_depthTexturePacker->init())
				{
					m_logger.log(SharedTextureLogLevel::info,
								 "SpoutOpenGLTextureWriter::init() - Error initializing float depth packer");
					return false;
				}
			}

			m_spoutDepthFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);
			m_spoutDepthFrame->SetFrameCount(m_parentAccessor->getEnableFrameCounter());
		}
		else if (descriptor->depth_buffer_type == SharedDepthBufferType::NODEPTH)
		{
			m_spoutDepthFrame= nullptr;
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - depth buffer type not supported: ";
			ss << (int)descriptor->depth_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		// Initialize the (optional) shadow spout frame. It's a color-like RGBA8/BGRA8 buffer,
		// so no depth packer is needed - it mirrors the color frame path.
		if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA32
			|| descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32
			|| descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
		{
			m_spoutShadowFrame= GetSpout();
			if (m_spoutShadowFrame == nullptr)
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutOpenGLTextureWriter::init() - Failed to open spout api for shadow");
				return false;
			}

			m_spoutShadowFrame->EnableSpoutLog();
			m_spoutShadowFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutShadowFrame->SetSenderName(m_parentAccessor->getShadowSenderName().c_str());

			if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
				m_spoutShadowFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R16G16B16A16_FLOAT);
			else if (descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32)
				m_spoutShadowFrame->SetSenderFormat((DWORD)DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutShadowFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutShadowFrame->SetFrameCount(m_parentAccessor->getEnableFrameCounter());
		}
		else if (descriptor->shadow_buffer_type == SharedShadowBufferType::NOSHADOW)
		{
			m_spoutShadowFrame= nullptr;
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - shadow buffer type not supported: ";
			ss << (int)descriptor->shadow_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		return bSuccess;
	}

	void dispose()
	{
		if (m_depthTexturePacker != nullptr)
		{
			delete m_depthTexturePacker;
			m_depthTexturePacker= nullptr;
		}

		if (m_spoutColorFrame != nullptr)
		{
			m_spoutColorFrame->Release();
			m_spoutColorFrame= nullptr;
		}

		if (m_spoutDepthFrame != nullptr)
		{
			m_spoutDepthFrame->Release();
			m_spoutDepthFrame= nullptr;
		}

		if (m_spoutShadowFrame != nullptr)
		{
			m_spoutShadowFrame->Release();
			m_spoutShadowFrame= nullptr;
		}
	}

	bool writeColorFrameTexture(GLuint textureID)
	{
		if (m_spoutColorFrame != nullptr)
		{
			const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();

			return m_spoutColorFrame->SendTexture(textureID, GL_TEXTURE_2D, descriptor->width, descriptor->height);
		}

		return false;
	}

	bool writeDepthFrameTexture(GLuint textureID, float zNear, float zFar)
	{
		if (m_spoutDepthFrame != nullptr)
		{
			const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();

			if (m_depthTexturePacker != nullptr)
			{
				// Convert the float depth texture to a RGBA8 texture using a shader
				// (Spout can only send RGBA8 textures)
				GLuint packedDepthTexture= m_depthTexturePacker->packDepthTexture(textureID, zNear, zFar);

				if (packedDepthTexture != 0)
				{
					return m_spoutDepthFrame->SendTexture(packedDepthTexture, GL_TEXTURE_2D, descriptor->width,
														  descriptor->height);
				}
			}
			else
			{
				return m_spoutDepthFrame->SendTexture(textureID, GL_TEXTURE_2D, descriptor->width, descriptor->height);
			}
		}

		return false;
	}

	bool writeShadowFrameTexture(GLuint textureID)
	{
		if (m_spoutShadowFrame != nullptr)
		{
			const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();

			return m_spoutShadowFrame->SendTexture(textureID, GL_TEXTURE_2D, descriptor->width, descriptor->height);
		}

		return false;
	}

	void* getPackDepthTextureResourcePtr() const
	{
		return m_depthTexturePacker != nullptr ? (void*)m_depthTexturePacker->getPackedDepthTextureResourcePtr()
											   : nullptr;
	}

private:
	SharedTextureWriteAccessor* m_parentAccessor= nullptr;
	SharedTextureLogger& m_logger;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
	SPOUTLIBRARY* m_spoutShadowFrame;
	SpoutGLDepthTexturePacker* m_depthTexturePacker= nullptr;
};

class SpoutDX11TextureWriter
{
public:
	SpoutDX11TextureWriter(SharedTextureWriteAccessor* parentAccessor)
		: m_parentAccessor(parentAccessor)
		, m_logger(parentAccessor->getLogger())
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_spoutShadowFrame()
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX11TextureWriter() { dispose(); }

	bool init()
	{
		const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();
		ID3D11Device* d3d11Device= (ID3D11Device*)m_parentAccessor->getApiDeviceInterface();
		bool bSuccess= true;

		dispose();

		EnableSpoutLog();
		EnableSpoutLogFile("sender.log");
		SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32
			|| descriptor->color_buffer_type == SharedColorBufferType::BGRA32
			|| descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
		{
			if (m_spoutColorFrame.OpenDirectX11(d3d11Device)
				&& m_spoutColorFrame.SetSenderName(m_parentAccessor->getColorSenderName().c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
				else if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_parentAccessor->getEnableFrameCounter())
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
				&& m_spoutDepthFrame.SetSenderName(m_parentAccessor->getDepthSenderName().c_str()))
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

				if (!m_parentAccessor->getEnableFrameCounter())
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
				&& m_spoutShadowFrame.SetSenderName(m_parentAccessor->getShadowSenderName().c_str()))
			{
				if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
				else if (descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32)
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_parentAccessor->getEnableFrameCounter())
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

	bool writeColorFrameTexture(ID3D11Texture2D* pTexture)
	{
		return m_bIsColorFrameInitialized ? m_spoutColorFrame.SendTexture(pTexture) : false;
	}

	bool writeDepthFrameTexture(ID3D11Texture2D* pTexture, float zNear, float zFar)
	{
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

	bool writeShadowFrameTexture(ID3D11Texture2D* pTexture)
	{
		return m_bIsShadowFrameInitialized ? m_spoutShadowFrame.SendTexture(pTexture) : false;
	}

	void* getPackDepthTextureResourcePtr() const
	{
		return m_depthTexturePacker != nullptr ? m_depthTexturePacker->getPackedDepthTextureResourcePtr() : nullptr;
	}

private:
	SharedTextureWriteAccessor* m_parentAccessor;
	SharedTextureLogger& m_logger;
	spoutDX m_spoutColorFrame;
	spoutDX m_spoutDepthFrame;
	spoutDX m_spoutShadowFrame;
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
	bool m_bIsShadowFrameInitialized= false;
};

class SpoutDX12TextureWriter
{
public:
	SpoutDX12TextureWriter(SharedTextureWriteAccessor* parentAccessor)
		: m_parentAccessor(parentAccessor)
		, m_logger(parentAccessor->getLogger())
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_spoutShadowFrame()
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX12TextureWriter() { dispose(); }

	bool init()
	{
		const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();
		ID3D12Device* d3d12Device= (ID3D12Device*)m_parentAccessor->getApiDeviceInterface();
		// Optional client command queue. When supplied, the D3D11On12 device shares it so the
		// wrapped-resource copy is GPU-ordered after the client's rendering (no flicker/tearing).
		IUnknown* commandQueue= (IUnknown*)m_parentAccessor->getApiCommandQueueInterface();
		IUnknown** ppCommandQueue= (commandQueue != nullptr) ? &commandQueue : nullptr;
		bool bSuccess= true;

		dispose();

		EnableSpoutLog();
		EnableSpoutLogFile("sender.log");
		SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32
			|| descriptor->color_buffer_type == SharedColorBufferType::BGRA32
			|| descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
		{
			if (m_spoutColorFrame.OpenDirectX12(d3d12Device, ppCommandQueue)
				&& m_spoutColorFrame.SetSenderName(m_parentAccessor->getColorSenderName().c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
				else if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_parentAccessor->getEnableFrameCounter())
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
				&& m_spoutDepthFrame.SetSenderName(m_parentAccessor->getDepthSenderName().c_str()))
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

				if (!m_parentAccessor->getEnableFrameCounter())
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
				&& m_spoutShadowFrame.SetSenderName(m_parentAccessor->getShadowSenderName().c_str()))
			{
				if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
				else if (descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32)
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutShadowFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_parentAccessor->getEnableFrameCounter())
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

	bool writeColorFrameTexture(ID3D12Resource* dx12TextureResource)
	{
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

	bool writeDepthFrameTexture(ID3D12Resource* dx12TextureResource, float zNear, float zFar)
	{
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

	bool writeShadowFrameTexture(ID3D12Resource* dx12TextureResource)
	{
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

	void* getPackDepthTextureResourcePtr() const
	{
		return m_depthTexturePacker != nullptr ? m_depthTexturePacker->getPackedDepthTextureResourcePtr() : nullptr;
	}

private:
	SharedTextureWriteAccessor* m_parentAccessor;
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

//-- SharedTextureWriteAccessor -----
SharedTextureWriteAccessor::SharedTextureWriteAccessor(const std::string& senderPrefix, MikanCameraID cameraId)
	: m_senderPrefix(senderPrefix)
	, m_cameraId(cameraId)
	, m_graphicsAPI(SharedClientGraphicsApi::UNKNOWN)
{
}

SharedTextureWriteAccessor::~SharedTextureWriteAccessor() { dispose(); }

bool SharedTextureWriteAccessor::getIsInitialized() const { return m_bIsInitialized; }

void SharedTextureWriteAccessor::setLogCallback(SharedTextureLogCallback callback)
{
	m_logger.setLogCallback(callback);
}

bool SharedTextureWriteAccessor::initialize(const SharedTextureDescriptor* descriptor, bool bEnableFrameCounter,
											void* apiDeviceInterface, void* apiCommandQueueInterface)
{
	dispose();

	m_renderTargetDescriptor= *descriptor;
	m_bEnableFrameCounter= bEnableFrameCounter;
	m_apiDeviceInterface= apiDeviceInterface;
	m_apiCommandQueueInterface= apiCommandQueueInterface;

	if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::COLOR, m_colorSenderName))
	{
		m_logger.log(SharedTextureLogLevel::error,
					 "SharedTextureWriteAccessor::initialize() - Failed to create spout color texture sender name");
		return false;
	}

	if (descriptor->depth_buffer_type != SharedDepthBufferType::NODEPTH)
	{
		if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::DEPTH, m_depthSenderName))
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SharedTextureWriteAccessor::initialize() - Failed to create spout depth texture sender name");
			return false;
		}
	}
	else
	{
		m_depthSenderName= "";
	}

	if (descriptor->shadow_buffer_type != SharedShadowBufferType::NOSHADOW)
	{
		if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::SHADOW, m_shadowSenderName))
		{
			m_logger.log(
				SharedTextureLogLevel::error,
				"SharedTextureWriteAccessor::initialize() - Failed to create spout shadow texture sender name");
			return false;
		}
	}
	else
	{
		m_shadowSenderName= "";
	}

	if (descriptor->graphicsAPI == SharedClientGraphicsApi::OpenGL)
	{
		m_writerApi.spoutOpenGLTextureWriter= new SpoutOpenGLTextureWriter(this);
		m_graphicsAPI= SharedClientGraphicsApi::OpenGL;

		m_bIsInitialized= m_writerApi.spoutOpenGLTextureWriter->init();
	}
	else if (descriptor->graphicsAPI == SharedClientGraphicsApi::Direct3D11)
	{
		m_writerApi.spoutDX11TextureWriter= new SpoutDX11TextureWriter(this);
		m_graphicsAPI= SharedClientGraphicsApi::Direct3D11;

		m_bIsInitialized= m_writerApi.spoutDX11TextureWriter->init();
	}
	else if (descriptor->graphicsAPI == SharedClientGraphicsApi::Direct3D12)
	{
		m_writerApi.spoutDX12TextureWriter= new SpoutDX12TextureWriter(this);
		m_graphicsAPI= SharedClientGraphicsApi::Direct3D12;

		m_bIsInitialized= m_writerApi.spoutDX12TextureWriter->init();
	}

	// Override the depth buffer type to RGBA8, as Spout only supports sending RGBA8/BGR8 textures
	if (m_bIsInitialized
		&& (m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
			|| m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH))
	{
		m_renderTargetDescriptor.depth_buffer_type= SharedDepthBufferType::PACK_DEPTH_RGBA;
	}

	return m_bIsInitialized;
}

void SharedTextureWriteAccessor::dispose()
{
	if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
	{
		if (m_writerApi.spoutOpenGLTextureWriter != nullptr)
		{
			m_writerApi.spoutOpenGLTextureWriter->dispose();
			delete m_writerApi.spoutOpenGLTextureWriter;
			m_writerApi.spoutOpenGLTextureWriter= nullptr;
		}
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
	{
		if (m_writerApi.spoutDX11TextureWriter != nullptr)
		{
			m_writerApi.spoutDX11TextureWriter->dispose();
			delete m_writerApi.spoutDX11TextureWriter;
			m_writerApi.spoutDX11TextureWriter= nullptr;
		}
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
	{
		if (m_writerApi.spoutDX12TextureWriter != nullptr)
		{
			m_writerApi.spoutDX12TextureWriter->dispose();
			delete m_writerApi.spoutDX12TextureWriter;
			m_writerApi.spoutDX12TextureWriter= nullptr;
		}
	}

	m_bEnableFrameCounter= false;
	m_apiDeviceInterface= nullptr;
	m_apiCommandQueueInterface= nullptr;
	m_graphicsAPI= SharedClientGraphicsApi::UNKNOWN;
	m_bIsInitialized= false;
}

const SharedTextureDescriptor* SharedTextureWriteAccessor::getRenderTargetDescriptor() const
{
	return &m_renderTargetDescriptor;
}

bool SharedTextureWriteAccessor::writeColorFrameTexture(void* apiTexturePtr)
{
	bool bSuccess= false;

	if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
	{
		GLuint* textureId= (GLuint*)apiTexturePtr;

		bSuccess= m_writerApi.spoutOpenGLTextureWriter->writeColorFrameTexture(*textureId);
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
	{
		ID3D11Texture2D* dx11Texture= (ID3D11Texture2D*)apiTexturePtr;

		bSuccess= m_writerApi.spoutDX11TextureWriter->writeColorFrameTexture(dx11Texture);
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
	{
		ID3D12Resource* dx12Texture= (ID3D12Resource*)apiTexturePtr;

		bSuccess= m_writerApi.spoutDX12TextureWriter->writeColorFrameTexture(dx12Texture);
	}
	else
	{
		bSuccess= false;
	}

	return true;
}

bool SharedTextureWriteAccessor::writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar)
{
	bool bSuccess= false;

	if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
	{
		GLuint* textureId= (GLuint*)apiTexturePtr;

		bSuccess= m_writerApi.spoutOpenGLTextureWriter->writeDepthFrameTexture(*textureId, zNear, zFar);
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
	{
		ID3D11Texture2D* dx11Texture= (ID3D11Texture2D*)apiTexturePtr;

		bSuccess= m_writerApi.spoutDX11TextureWriter->writeDepthFrameTexture(dx11Texture, zNear, zFar);
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
	{
		ID3D12Resource* dx12Texture= (ID3D12Resource*)apiTexturePtr;

		bSuccess= m_writerApi.spoutDX12TextureWriter->writeDepthFrameTexture(dx12Texture, zNear, zFar);
	}
	else
	{
		bSuccess= false;
	}

	return true;
}

bool SharedTextureWriteAccessor::writeShadowFrameTexture(void* apiTexturePtr)
{
	bool bSuccess= false;

	if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
	{
		GLuint* textureId= (GLuint*)apiTexturePtr;

		bSuccess= m_writerApi.spoutOpenGLTextureWriter->writeShadowFrameTexture(*textureId);
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
	{
		ID3D11Texture2D* dx11Texture= (ID3D11Texture2D*)apiTexturePtr;

		bSuccess= m_writerApi.spoutDX11TextureWriter->writeShadowFrameTexture(dx11Texture);
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
	{
		ID3D12Resource* dx12Texture= (ID3D12Resource*)apiTexturePtr;

		bSuccess= m_writerApi.spoutDX12TextureWriter->writeShadowFrameTexture(dx12Texture);
	}
	else
	{
		bSuccess= false;
	}

	return bSuccess;
}

void* SharedTextureWriteAccessor::getPackDepthTextureResourcePtr() const
{
	if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
	{
		return m_writerApi.spoutOpenGLTextureWriter->getPackDepthTextureResourcePtr();
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
	{
		return m_writerApi.spoutDX11TextureWriter->getPackDepthTextureResourcePtr();
	}
	else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
	{
		return m_writerApi.spoutDX12TextureWriter->getPackDepthTextureResourcePtr();
	}
	else
	{
		return nullptr;
	}
}

ISharedTextureWriteAccessorPtr createSharedTextureWriteAccessor(const std::string& prefix, MikanCameraID cameraId)
{
	return std::make_shared<SharedTextureWriteAccessor>(prefix, cameraId);
}