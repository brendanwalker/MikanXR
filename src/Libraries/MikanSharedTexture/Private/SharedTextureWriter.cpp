#include "SharedTextureWriter.h"
#include "SharedTextureWriterBackend.h"
#include "SharedTextureLogger.h"
#include "SharedTextureUtility.h"

#include <string>

// -- SharedTextureWriteAccessor -----
// The API-agnostic half of a client's shared texture: it derives the Spout sender names, holds
// the context its backend reads, and forwards each frame to that backend. Everything that knows
// a graphics API lives in the per-API writer files behind ISharedTextureWriterBackend.
class SharedTextureWriteAccessor : public ISharedTextureWriteAccessor
{
public:
	SharedTextureWriteAccessor(const std::string& senderPrefix, MikanCameraID cameraId);
	~SharedTextureWriteAccessor();

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
	SharedTextureDescriptor m_renderTargetDescriptor;
	// Read by the backend for the whole of its life, so it is declared ahead of the backend and
	// therefore destroyed after it
	SharedTextureWriterContext m_context;
	ISharedTextureWriterBackendPtr m_backend;
	SharedTextureLogger m_logger;
};

SharedTextureWriteAccessor::SharedTextureWriteAccessor(const std::string& senderPrefix, MikanCameraID cameraId)
	: m_senderPrefix(senderPrefix)
	, m_cameraId(cameraId)
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
	m_context.descriptor= &m_renderTargetDescriptor;
	m_context.apiDeviceInterface= apiDeviceInterface;
	m_context.apiCommandQueueInterface= apiCommandQueueInterface;
	m_context.bEnableFrameCounter= bEnableFrameCounter;
	m_context.logger= &m_logger;

	if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::COLOR, m_context.colorSenderName))
	{
		m_logger.log(SharedTextureLogLevel::error,
					 "SharedTextureWriteAccessor::initialize() - Failed to create spout color texture sender name");
		return false;
	}

	if (descriptor->depth_buffer_type != SharedDepthBufferType::NODEPTH)
	{
		if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::DEPTH, m_context.depthSenderName))
		{
			m_logger.log(SharedTextureLogLevel::error,
						 "SharedTextureWriteAccessor::initialize() - Failed to create spout depth texture sender name");
			return false;
		}
	}
	else
	{
		m_context.depthSenderName= "";
	}

	if (descriptor->shadow_buffer_type != SharedShadowBufferType::NOSHADOW)
	{
		if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::SHADOW, m_context.shadowSenderName))
		{
			m_logger.log(
				SharedTextureLogLevel::error,
				"SharedTextureWriteAccessor::initialize() - Failed to create spout shadow texture sender name");
			return false;
		}
	}
	else
	{
		m_context.shadowSenderName= "";
	}

	switch (descriptor->graphicsAPI)
	{
	case SharedClientGraphicsApi::OpenGL:
		m_backend= createOpenGLTextureWriter(m_context);
		break;
	case SharedClientGraphicsApi::Direct3D11:
		m_backend= createDX11TextureWriter(m_context);
		break;
	case SharedClientGraphicsApi::Direct3D12:
		m_backend= createDX12TextureWriter(m_context);
		break;
	case SharedClientGraphicsApi::Vulkan:
		m_backend= createVulkanTextureWriter(m_context);
		break;
	default:
		break;
	}

	// A backend that fails to initialize is kept until dispose, which is what frees whatever it
	// managed to open before the failure
	m_bIsInitialized= m_backend != nullptr && m_backend->init();

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
	// The backend's destructor releases its Spout senders and its graphics API resources, and it
	// reads the context while doing so, so the backend goes first
	m_backend.reset();

	m_context= SharedTextureWriterContext();
	m_bIsInitialized= false;
}

const SharedTextureDescriptor* SharedTextureWriteAccessor::getRenderTargetDescriptor() const
{
	return &m_renderTargetDescriptor;
}

bool SharedTextureWriteAccessor::writeColorFrameTexture(void* apiTexturePtr)
{
	return m_backend != nullptr && m_backend->writeColorFrameTexture(apiTexturePtr);
}

bool SharedTextureWriteAccessor::writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar)
{
	return m_backend != nullptr && m_backend->writeDepthFrameTexture(apiTexturePtr, zNear, zFar);
}

bool SharedTextureWriteAccessor::writeShadowFrameTexture(void* apiTexturePtr)
{
	return m_backend != nullptr && m_backend->writeShadowFrameTexture(apiTexturePtr);
}

void* SharedTextureWriteAccessor::getPackDepthTextureResourcePtr() const
{
	return m_backend != nullptr ? m_backend->getPackDepthTextureResourcePtr() : nullptr;
}

ISharedTextureWriteAccessorPtr createSharedTextureWriteAccessor(const std::string& prefix, MikanCameraID cameraId)
{
	return std::make_shared<SharedTextureWriteAccessor>(prefix, cameraId);
}
