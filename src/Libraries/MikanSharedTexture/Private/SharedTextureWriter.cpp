#include "SharedTextureWriter.h"
#include "SpoutDX.h"
#include "SpoutDX12.h"
#include "SpoutDXDepthTexturePacker.h"
#include "SpoutLibrary.h"
#include "SharedTextureLogger.h"

#include "assert.h"
#include <sstream>

// Define DXGI_FORMAT enum values ourselves if we don't use SPOUTDX API
//enum DXGI_FORMAT
//{
//	DXGI_FORMAT_R8G8B8A8_UNORM                          = 28,
//    DXGI_FORMAT_B8G8R8A8_UNORM                          = 87,
//};

class SpoutOpenGLTextureWriter
{
public:
	SpoutOpenGLTextureWriter(
		SharedTextureLogger& logger,
		const std::string& clientName)
		: m_logger(logger)
		, m_colorFrameSenderName(clientName+"_color")
		, m_depthFrameSenderName(clientName+"_depth")
		, m_spoutColorFrame(nullptr)
		, m_spoutDepthFrame(nullptr)
		, m_descriptor()
	{
	}

	virtual ~SpoutOpenGLTextureWriter()
	{
		dispose();
	}

	bool init(const SharedTextureDescriptor* descriptor, bool bEnableFrameCounter)
	{
		bool bSuccess = true;

		dispose();

		m_spoutColorFrame = GetSpout();
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
			m_spoutColorFrame->SetSenderName(m_colorFrameSenderName.c_str());

			if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutColorFrame->SetFrameCount(bEnableFrameCounter);
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - color buffer type not supported: ";
			ss << (int)descriptor->color_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		if (descriptor->depth_buffer_type == SharedDepthBufferType::PACK_DEPTH_RGBA)
		{
			m_spoutDepthFrame->EnableSpoutLog();
			m_spoutDepthFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutDepthFrame->SetSenderName(m_depthFrameSenderName.c_str());

			m_spoutDepthFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutDepthFrame->SetFrameCount(bEnableFrameCounter);
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
		if (m_spoutColorFrame != nullptr)
		{
			return m_spoutColorFrame->SendTexture(textureID, GL_TEXTURE_2D, m_descriptor.width, m_descriptor.height);
		}

		return false;
	}

	bool writeDepthFrameTexture(GLuint textureID, float zNear, float zFar)
	{
		if (m_spoutDepthFrame != nullptr)
		{
			return m_spoutDepthFrame->SendTexture(
				textureID, GL_TEXTURE_2D, 
				m_descriptor.width, m_descriptor.height);
		}

		return false;
	}

	void* getPackDepthTextureResourcePtr() const
	{
		// TODO: Implement this
		return nullptr;
	}

private:
	SharedTextureLogger& m_logger;
	std::string m_colorFrameSenderName;
	std::string m_depthFrameSenderName;
	SharedTextureDescriptor m_descriptor;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
};

class SpoutDX11TextureWriter
{
public:
	SpoutDX11TextureWriter::SpoutDX11TextureWriter(
		SharedTextureLogger& logger,
		const std::string& clientName)
		: m_logger(logger)
		, m_colorFrameSenderName(clientName+"_color")
		, m_depthFrameSenderName(clientName+"_depth")
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_depthTexturePacker(nullptr)
	{
	}

	virtual ~SpoutDX11TextureWriter()
	{
		dispose();
	}

	bool init(const SharedTextureDescriptor* descriptor, bool bEnableFrameCounter, void* apiDeviceInterface)
	{
		ID3D11Device* d3d11Device= (ID3D11Device*)apiDeviceInterface;
		bool bSuccess = true;

		dispose();

		EnableSpoutLog();
		EnableSpoutLogFile("sender.log");
		SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32 ||
			descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
		{
			if (m_spoutColorFrame.OpenDirectX11(d3d11Device) &&
				m_spoutColorFrame.SetSenderName(m_colorFrameSenderName.c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!bEnableFrameCounter)
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
				m_spoutDepthFrame.SetSenderName(m_depthFrameSenderName.c_str()))
			{
				// Initialize the depth texture packer if we are sending float depth textures
				if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
					descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
				{
					m_depthTexturePacker = new SpoutDXDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
					if (!m_depthTexturePacker->init())
					{
						m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing float depth packer");
						return false;
					}
				}

				m_spoutDepthFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!bEnableFrameCounter)
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
			m_depthTexturePacker = nullptr;
		}

		m_spoutColorFrame.ReleaseSender();
		m_spoutColorFrame.CloseDirectX11();
		m_bIsColorFrameInitialized = false;

		m_spoutDepthFrame.ReleaseSender();
		m_spoutDepthFrame.CloseDirectX11();
		m_bIsDepthFrameInitialized = false;

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
				ID3D11Texture2D* packedDepthTexture = 
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
	SharedTextureLogger& m_logger;
	std::string m_colorFrameSenderName;
	std::string m_depthFrameSenderName;
	spoutDX m_spoutColorFrame;
	spoutDX m_spoutDepthFrame;
	SpoutDXDepthTexturePacker* m_depthTexturePacker= nullptr;
	bool m_bIsColorFrameInitialized= false;
	bool m_bIsDepthFrameInitialized= false;
};

class SpoutDX12TextureWriter
{
public:
	SpoutDX12TextureWriter::SpoutDX12TextureWriter(
		SharedTextureLogger& logger,
		const std::string& clientName)
		: m_logger(logger)
		, m_colorFrameSenderName(clientName + "_color")
		, m_depthFrameSenderName(clientName + "_depth")
		, m_spoutColorFrame()
		, m_spoutDepthFrame()
		, m_depthTexturePacker(nullptr)
	{}

	virtual ~SpoutDX12TextureWriter()
	{
		dispose();
	}

	bool init(const SharedTextureDescriptor* descriptor, bool bEnableFrameCounter, void* apiDeviceInterface)
	{
		ID3D12Device* d3d12Device = (ID3D12Device*)apiDeviceInterface;
		bool bSuccess = true;

		dispose();

		EnableSpoutLog();
		EnableSpoutLogFile("sender.log");
		SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

		// Initialize the color spout frame
		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32 ||
			descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
		{
			if (m_spoutColorFrame.OpenDirectX12(d3d12Device) &&
				m_spoutColorFrame.SetSenderName(m_colorFrameSenderName.c_str()))
			{
				if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_B8G8R8A8_UNORM);
				else
					m_spoutColorFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!bEnableFrameCounter)
					m_spoutColorFrame.DisableFrameCount();

				m_bIsColorFrameInitialized = true;
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
				m_spoutDepthFrame.SetSenderName(m_depthFrameSenderName.c_str()))
			{
				// Initialize the depth texture packer if we are sending float depth textures
				if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
					descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
				{
					m_depthTexturePacker = new SpoutDXDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
					if (!m_depthTexturePacker->init())
					{
						m_logger.log(SharedTextureLogLevel::info, "SpoutDX11TextureWriter::init() - Error initializing float depth packer");
						return false;
					}
				}

				m_spoutDepthFrame.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

				if (!bEnableFrameCounter)
					m_spoutDepthFrame.DisableFrameCount();

				m_bIsDepthFrameInitialized = true;
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
			m_depthTexturePacker = nullptr;
		}

		m_spoutColorFrame.ReleaseSender();
		m_spoutColorFrame.CloseDirectX12();
		m_bIsColorFrameInitialized = false;

		m_spoutDepthFrame.ReleaseSender();
		m_spoutDepthFrame.CloseDirectX12();
		m_bIsDepthFrameInitialized = false;

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
		bool bSuccess = false;

		if (m_bIsDepthFrameInitialized)
		{
			if (m_spoutDX12DepthTexture != dx12TextureResource)
			{
				if (m_spoutDX11DepthTexture != nullptr)
				{
					m_spoutDX11DepthTexture->Release();
					m_spoutDX11DepthTexture = nullptr;
				}

				if (dx12TextureResource != nullptr &&
					m_spoutDepthFrame.WrapDX12Resource(
						dx12TextureResource,
						&m_spoutDX11DepthTexture,
						D3D12_RESOURCE_STATE_COPY_SOURCE))
				{
					m_spoutDX12DepthTexture = dx12TextureResource;
				}
			}

			if (m_spoutDX11DepthTexture != nullptr)
			{
				if (m_depthTexturePacker != nullptr)
				{
					// Convert the float depth texture to a RGBA8 texture using a shader
					// (Spout can only send RGBA8 textures)
					ID3D11Texture2D* pTexture11 = (ID3D11Texture2D*)m_spoutDX11DepthTexture;
					ID3D11Texture2D* packedDepthTexture =
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
	SharedTextureLogger& m_logger;
	std::string m_colorFrameSenderName;
	std::string m_depthFrameSenderName;
	spoutDX12 m_spoutColorFrame;
	ID3D12Resource* m_spoutDX12ColorTexture = nullptr;
	ID3D11Resource* m_spoutDX11ColorTexture = nullptr;
	spoutDX12 m_spoutDepthFrame;
	ID3D12Resource* m_spoutDX12DepthTexture = nullptr;
	ID3D11Resource* m_spoutDX11DepthTexture = nullptr;
	SpoutDXDepthTexturePacker* m_depthTexturePacker = nullptr;
	bool m_bIsColorFrameInitialized = false;
	bool m_bIsDepthFrameInitialized = false;
};

//-- SharedTextureWriteAccessor -----
class SharedTextureWriteAccessor : public ISharedTextureWriteAccessor
{
public:
	SharedTextureWriteAccessor::SharedTextureWriteAccessor(const std::string& clientName)
		: m_clientName(clientName)
		, m_graphicsAPI(SharedClientGraphicsApi::UNKNOWN)
	{
	}

	SharedTextureWriteAccessor::~SharedTextureWriteAccessor()
	{
		dispose();
	}

	virtual bool getIsInitialized() const
	{ 
		return m_bIsInitialized; 
	}

	virtual void setLogCallback(SharedTextureLogCallback callback) override
	{
		m_logger.setLogCallback(callback);
	}

	virtual bool setClientName(const std::string& clientName) override
	{
		if (!clientName.empty())
		{
			if (m_clientName != clientName)
			{
				if (getIsInitialized())
				{
					// Backup the existing settings
					const SharedTextureDescriptor descriptor = m_renderTargetDescriptor;
					bool bEnableFrameCounter = m_bEnableFrameCounter;
					void* apiDeviceInterface = m_apiDeviceInterface;

					// Clean up the old writer
					dispose();

					// Set the new client name
					m_clientName = clientName;

					// Restart the writer with the new name, but same settings
					initialize(&descriptor, bEnableFrameCounter, apiDeviceInterface);
				}
				else
				{
					m_clientName = clientName;
				}
			}

			return true;
		}

		return false;
	}

	virtual bool initialize(
		const SharedTextureDescriptor* descriptor,
		bool bEnableFrameCounter,
		void* apiDeviceInterface) override
	{
		dispose();

		m_renderTargetDescriptor = *descriptor;
		m_bEnableFrameCounter = bEnableFrameCounter;
		m_apiDeviceInterface = apiDeviceInterface;

		if (descriptor->graphicsAPI == SharedClientGraphicsApi::OpenGL)
		{
			m_writerApi.spoutOpenGLTextureWriter = new SpoutOpenGLTextureWriter(m_logger, m_clientName);
			m_graphicsAPI = SharedClientGraphicsApi::OpenGL;

			m_bIsInitialized = m_writerApi.spoutOpenGLTextureWriter->init(descriptor, bEnableFrameCounter);
		}
		else if (descriptor->graphicsAPI == SharedClientGraphicsApi::Direct3D11)
		{
			m_writerApi.spoutDX11TextureWriter = new SpoutDX11TextureWriter(m_logger, m_clientName);
			m_graphicsAPI = SharedClientGraphicsApi::Direct3D11;

			m_bIsInitialized = m_writerApi.spoutDX11TextureWriter->init(descriptor, bEnableFrameCounter, apiDeviceInterface);
			if (m_bIsInitialized &&
				(m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
				 m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH))
			{
				// Override the depth buffer type to RGBA8, as Spout only supports sending RGBA8/BGR8 textures
				m_renderTargetDescriptor.depth_buffer_type = SharedDepthBufferType::PACK_DEPTH_RGBA;
			}
		}
		else if (descriptor->graphicsAPI == SharedClientGraphicsApi::Direct3D12)
		{
			m_writerApi.spoutDX12TextureWriter = new SpoutDX12TextureWriter(m_logger, m_clientName);
			m_graphicsAPI = SharedClientGraphicsApi::Direct3D12;

			m_bIsInitialized = m_writerApi.spoutDX12TextureWriter->init(descriptor, bEnableFrameCounter, apiDeviceInterface);
			if (m_bIsInitialized &&
				(m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH ||
				 m_renderTargetDescriptor.depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH))
			{
				// Override the depth buffer type to RGBA8, as Spout only supports sending RGBA8/BGR8 textures
				m_renderTargetDescriptor.depth_buffer_type = SharedDepthBufferType::PACK_DEPTH_RGBA;
			}
		}

		return m_bIsInitialized;
	}

	virtual void dispose() override
	{
		if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
		{
			if (m_writerApi.spoutOpenGLTextureWriter != nullptr)
			{
				m_writerApi.spoutOpenGLTextureWriter->dispose();
				delete m_writerApi.spoutOpenGLTextureWriter;
				m_writerApi.spoutOpenGLTextureWriter = nullptr;
			}
		}
		else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
		{
			if (m_writerApi.spoutDX11TextureWriter != nullptr)
			{
				m_writerApi.spoutDX11TextureWriter->dispose();
				delete m_writerApi.spoutDX11TextureWriter;
				m_writerApi.spoutDX11TextureWriter = nullptr;
			}
		}
		else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
		{
			if (m_writerApi.spoutDX12TextureWriter != nullptr)
			{
				m_writerApi.spoutDX12TextureWriter->dispose();
				delete m_writerApi.spoutDX12TextureWriter;
				m_writerApi.spoutDX12TextureWriter = nullptr;
			}
		}

		m_bEnableFrameCounter = false;
		m_apiDeviceInterface = nullptr;
		m_graphicsAPI = SharedClientGraphicsApi::UNKNOWN;
		m_bIsInitialized = false;
	}

	virtual const SharedTextureDescriptor* getRenderTargetDescriptor() const override
	{
		return &m_renderTargetDescriptor;
	}

	virtual bool writeColorFrameTexture(void* apiTexturePtr) override
	{
		bool bSuccess = false;

		if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
		{
			GLuint* textureId = (GLuint*)apiTexturePtr;

			bSuccess = m_writerApi.spoutOpenGLTextureWriter->writeColorFrameTexture(*textureId);
		}
		else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
		{
			ID3D11Texture2D* dx11Texture = (ID3D11Texture2D*)apiTexturePtr;

			bSuccess = m_writerApi.spoutDX11TextureWriter->writeColorFrameTexture(dx11Texture);
		}
		else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
		{
			ID3D12Resource* dx12Texture = (ID3D12Resource*)apiTexturePtr;

			bSuccess = m_writerApi.spoutDX12TextureWriter->writeColorFrameTexture(dx12Texture);
		}
		else
		{
			bSuccess = false;
		}

		return true;
	}

	virtual bool writeDepthFrameTexture(
		void* apiTexturePtr,
		float zNear,
		float zFar) override
	{
		bool bSuccess = false;

		if (m_graphicsAPI == SharedClientGraphicsApi::OpenGL)
		{
			GLuint* textureId = (GLuint*)apiTexturePtr;

			bSuccess = m_writerApi.spoutOpenGLTextureWriter->writeDepthFrameTexture(*textureId, zNear, zFar);
		}
		else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D11)
		{
			ID3D11Texture2D* dx11Texture = (ID3D11Texture2D*)apiTexturePtr;

			bSuccess = m_writerApi.spoutDX11TextureWriter->writeDepthFrameTexture(dx11Texture, zNear, zFar);
		}
		else if (m_graphicsAPI == SharedClientGraphicsApi::Direct3D12)
		{
			ID3D12Resource* dx12Texture = (ID3D12Resource*)apiTexturePtr;

			bSuccess = m_writerApi.spoutDX12TextureWriter->writeDepthFrameTexture(dx12Texture, zNear, zFar);
		}
		else
		{
			bSuccess = false;
		}

		return true;
	}

	virtual void* getPackDepthTextureResourcePtr() const override
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

private:
	bool m_bIsInitialized = false;
	std::string m_clientName;
	SharedTextureDescriptor m_renderTargetDescriptor;
	bool m_bEnableFrameCounter= false;
	void* m_apiDeviceInterface= nullptr;
	union
	{
		SpoutDX11TextureWriter* spoutDX11TextureWriter;
		SpoutDX12TextureWriter* spoutDX12TextureWriter;
		SpoutOpenGLTextureWriter* spoutOpenGLTextureWriter;
	} m_writerApi;
	SharedClientGraphicsApi m_graphicsAPI;
	SharedTextureLogger m_logger;
};

ISharedTextureWriteAccessorPtr createSharedTextureWriteAccessor(const std::string& clientName)
{
	return std::make_shared<SharedTextureWriteAccessor>(clientName);
}