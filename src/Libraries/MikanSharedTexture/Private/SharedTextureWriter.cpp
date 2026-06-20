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
	inline const std::string& getColorSenderName() const { return m_colorSenderName; }
	inline const std::string& getDepthSenderName() const { return m_depthSenderName; }
	inline bool getEnableFrameCounter() const { return m_bEnableFrameCounter; }

	virtual bool getIsInitialized() const;
	virtual void setLogCallback(SharedTextureLogCallback callback) override;

	virtual bool initialize(
		const SharedTextureDescriptor* descriptor,
		bool bEnableFrameCounter,
		void* apiDeviceInterface) override;
	virtual void dispose() override;

	virtual const SharedTextureDescriptor* getRenderTargetDescriptor() const override;
	virtual bool writeColorFrameTexture(void* apiTexturePtr) override;
	virtual bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar) override;
	virtual void* getPackDepthTextureResourcePtr() const override;

private:
	bool m_bIsInitialized= false;
	const std::string m_senderPrefix;
	MikanCameraID m_cameraId= -1;
	std::string m_colorSenderName;
	std::string m_depthSenderName;
	SharedTextureDescriptor m_renderTargetDescriptor;
	bool m_bEnableFrameCounter= false;
	void* m_apiDeviceInterface= nullptr;
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
	{
	}

	virtual ~SpoutOpenGLTextureWriter()
	{
		dispose();
	}

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

		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32 ||
			descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
		{
			m_spoutColorFrame->EnableSpoutLog();
			m_spoutColorFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutColorFrame->SetSenderName(m_parentAccessor->getColorSenderName().c_str());

			if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
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

		if (descriptor->depth_buffer_type == SharedDepthBufferType::PACK_DEPTH_RGBA ||
			descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
			descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
		{
			m_spoutDepthFrame= GetSpout();
			if (m_spoutDepthFrame == nullptr)
			{
				m_logger.log(SharedTextureLogLevel::error, "SpoutOpenGLTextureWriter::init() - Failed to open spout api for depth");
				return false;
			}

			m_spoutDepthFrame->EnableSpoutLog();
			m_spoutDepthFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutDepthFrame->SetSenderName(m_parentAccessor->getDepthSenderName().c_str());

			// Initialize the depth texture packer if we are sending float depth textures
			if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
				descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
			{
				m_depthTexturePacker= new SpoutGLDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
				if (!m_depthTexturePacker->init())
				{
					m_logger.log(SharedTextureLogLevel::info, "SpoutOpenGLTextureWriter::init() - Error initializing float depth packer");
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
				GLuint packedDepthTexture=
					m_depthTexturePacker->packDepthTexture(textureID, zNear, zFar);

				if (packedDepthTexture != 0)
				{
					return m_spoutDepthFrame->SendTexture(
						packedDepthTexture, GL_TEXTURE_2D,
						descriptor->width, descriptor->height);
				}
			}
			else
			{
				return m_spoutDepthFrame->SendTexture(
					textureID, GL_TEXTURE_2D,
					descriptor->width, descriptor->height);
			}
		}

		return false;
	}

	void* getPackDepthTextureResourcePtr() const
	{
		return m_depthTexturePacker != nullptr
				   ? (void*)m_depthTexturePacker->getPackedDepthTextureResourcePtr()
				   : nullptr;
	}

private:
	SharedTextureWriteAccessor* m_parentAccessor= nullptr;
	SharedTextureLogger& m_logger;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
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
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX11TextureWriter()
	{
		dispose();
	}

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
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32 ||
			descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
		{
			if (m_spoutColorFrame.OpenDirectX11(d3d11Device) &&
				m_spoutColorFrame.SetSenderName(m_parentAccessor->getColorSenderName().c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_parentAccessor->getEnableFrameCounter())
					m_spoutColorFrame.DisableFrameCount();

				m_bIsColorFrameInitialized= true;
			}
			else
			{
				m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing color spout frame");
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
			if (m_spoutDepthFrame.OpenDirectX11(d3d11Device) &&
				m_spoutDepthFrame.SetSenderName(m_parentAccessor->getDepthSenderName().c_str()))
			{
				// Initialize the depth texture packer if we are sending float depth textures
				if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
					descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
				{
					m_depthTexturePacker= new SpoutDXDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
					if (!m_depthTexturePacker->init())
					{
						m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing float depth packer");
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
				m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing depth spout frame");
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
				ID3D11Texture2D* packedDepthTexture=
					m_depthTexturePacker->packDepthTexture(pTexture, zNear, zFar);

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

	void* getPackDepthTextureResourcePtr() const
	{
		return m_depthTexturePacker != nullptr ? m_depthTexturePacker->getPackedDepthTextureResourcePtr() : nullptr;
	}

private:
	SharedTextureWriteAccessor* m_parentAccessor;
	SharedTextureLogger& m_logger;
	spoutDX m_spoutColorFrame;
	spoutDX m_spoutDepthFrame;
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
};

class SpoutDX12TextureWriter
{
public:
	SpoutDX12TextureWriter(SharedTextureWriteAccessor* parentAccessor)
		: m_parentAccessor(parentAccessor)
		, m_logger(parentAccessor->getLogger())
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX12TextureWriter()
	{
		dispose();
	}

	bool init()
	{
		const SharedTextureDescriptor* descriptor= m_parentAccessor->getRenderTargetDescriptor();
		ID3D12Device* d3d12Device= (ID3D12Device*)m_parentAccessor->getApiDeviceInterface();
		bool bSuccess= true;

		dispose();

		EnableSpoutLog();
		EnableSpoutLogFile("sender.log");
		SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32 ||
			descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
		{
			if (m_spoutColorFrame.OpenDirectX12(d3d12Device) &&
				m_spoutColorFrame.SetSenderName(m_parentAccessor->getColorSenderName().c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!m_parentAccessor->getEnableFrameCounter())
					m_spoutColorFrame.DisableFrameCount();

				m_bIsColorFrameInitialized= true;
			}
			else
			{
				m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing color spout frame");
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
			if (m_spoutDepthFrame.OpenDirectX12(d3d12Device) &&
				m_spoutDepthFrame.SetSenderName(m_parentAccessor->getDepthSenderName().c_str()))
			{
				// Initialize the depth texture packer if we are sending float depth textures
				if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
					descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
				{
					m_depthTexturePacker= new SpoutDXDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
					if (!m_depthTexturePacker->init())
					{
						m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing float depth packer");
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
				m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing depth spout frame");
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

				if (dx12TextureResource != nullptr &&
					m_spoutColorFrame.WrapDX12Resource(
						dx12TextureResource,
						&m_spoutDX11ColorTexture,
						D3D12_RESOURCE_STATE_COPY_SOURCE))
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

				if (dx12TextureResource != nullptr &&
					m_spoutDepthFrame.WrapDX12Resource(
						dx12TextureResource,
						&m_spoutDX11DepthTexture,
						D3D12_RESOURCE_STATE_COPY_SOURCE))
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
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
};

//-- SharedTextureWriteAccessor -----
SharedTextureWriteAccessor::SharedTextureWriteAccessor(const std::string& senderPrefix, MikanCameraID cameraId)
	: m_senderPrefix(senderPrefix)
	, m_cameraId(cameraId)
	, m_graphicsAPI(SharedClientGraphicsApi::UNKNOWN)
{
}

SharedTextureWriteAccessor::~SharedTextureWriteAccessor()
{
	dispose();
}

bool SharedTextureWriteAccessor::getIsInitialized() const
{
	return m_bIsInitialized;
}

void SharedTextureWriteAccessor::setLogCallback(SharedTextureLogCallback callback)
{
	m_logger.setLogCallback(callback);
}

bool SharedTextureWriteAccessor::initialize(
	const SharedTextureDescriptor* descriptor,
	bool bEnableFrameCounter,
	void* apiDeviceInterface)
{
	dispose();

	m_renderTargetDescriptor= *descriptor;
	m_bEnableFrameCounter= bEnableFrameCounter;
	m_apiDeviceInterface= apiDeviceInterface;

	if (!makeSpoutSenderName(
			m_senderPrefix,
			m_cameraId,
			SharedTextureType::COLOR,
			m_colorSenderName))
	{
		m_logger.log(SharedTextureLogLevel::error, "SharedTextureWriteAccessor::initialize() - Failed to create spout color texture sender name");
		return false;
	}

	if (descriptor->depth_buffer_type != SharedDepthBufferType::NODEPTH)
	{
		if (!makeSpoutSenderName(
				m_senderPrefix,
				m_cameraId,
				SharedTextureType::DEPTH,
				m_depthSenderName))
		{
			m_logger.log(SharedTextureLogLevel::error, "SharedTextureWriteAccessor::initialize() - Failed to create spout depth texture sender name");
			return false;
		}
	}
	else
	{
		m_depthSenderName= "";
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
	if (m_bIsInitialized &&
		(m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
		 m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH))
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

bool SharedTextureWriteAccessor::writeDepthFrameTexture(
	void* apiTexturePtr,
	float zNear,
	float zFar)
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