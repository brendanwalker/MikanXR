#include "SharedTextureReader.h"
#include "SharedTextureUtility.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "MikanCoreCAPI.h"
#include "SpoutLibrary.h"

#include <easy/profiler.h>

class SpoutTextureReader
{
public:
	SpoutTextureReader(SharedTextureReadAccessor* parentAccessor)
		: m_parentAccessor(parentAccessor)
		, m_spoutColorFrame(nullptr)
		, m_spoutDepthFrame(nullptr)
		, m_spoutShadowFrame(nullptr)
	{
	}

	virtual ~SpoutTextureReader() { dispose(); }

	bool init(const MikanRenderTargetDescriptor* descriptor)
	{
		dispose();

		const std::string colorSenderName= m_parentAccessor->getColorSenderName();

		m_spoutColorFrame= GetSpout();
		if (m_spoutColorFrame != nullptr)
		{
			m_spoutColorFrame->EnableSpoutLog();
			m_spoutColorFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutColorFrame->SetReceiverName(colorSenderName.c_str());
		}
		else
		{
			MIKAN_LOG_ERROR("SpoutTextureReader") << "Failed to open spout for sender: " << colorSenderName;
			return false;
		}

		if (descriptor->depth_buffer_type != MikanDepthBuffer_NODEPTH)
		{
			const std::string depthSenderName= m_parentAccessor->getDepthSenderName();

			m_spoutDepthFrame= GetSpout();
			if (m_spoutDepthFrame != nullptr)
			{
				m_spoutDepthFrame->EnableSpoutLog();
				m_spoutDepthFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
				m_spoutDepthFrame->SetReceiverName(depthSenderName.c_str());
			}
			else
			{
				MIKAN_LOG_ERROR("SpoutTextureReader") << "Failed to open spout for sender: " << depthSenderName;
				return false;
			}
		}

		if (descriptor->shadow_buffer_type != MikanShadowBuffer_NOSHADOW)
		{
			const std::string shadowSenderName= m_parentAccessor->getShadowSenderName();

			m_spoutShadowFrame= GetSpout();
			if (m_spoutShadowFrame != nullptr)
			{
				m_spoutShadowFrame->EnableSpoutLog();
				m_spoutShadowFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
				m_spoutShadowFrame->SetReceiverName(shadowSenderName.c_str());
			}
			else
			{
				MIKAN_LOG_ERROR("SpoutTextureReader") << "Failed to open spout for sender: " << shadowSenderName;
				return false;
			}
		}

		return true;
	}

	void dispose()
	{
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

	bool readRenderTargetTexture()
	{
		bool bSuccess= false;

		IMkTexturePtr colorTexture= m_parentAccessor->getColorTexture();
		if (colorTexture != nullptr)
		{
			EASY_BLOCK("receive color texture");

			if (m_spoutColorFrame->IsUpdated())
			{
				colorTexture->disposeTexture();
				colorTexture->setSize(m_spoutColorFrame->GetSenderWidth(), m_spoutColorFrame->GetSenderHeight());
				colorTexture->setTextureFormat(GL_RGBA);
				colorTexture->setBufferFormat(GL_RGBA);
				colorTexture->createTexture();
			}

			bSuccess= m_spoutColorFrame->ReceiveTexture(colorTexture->getGlTextureId(), GL_TEXTURE_2D);
		}

		IMkTexturePtr depthTexture= m_parentAccessor->getDepthTexture();
		if (depthTexture != nullptr)
		{
			EASY_BLOCK("receive depth texture");

			if (m_spoutDepthFrame->IsUpdated())
			{
				depthTexture->disposeTexture();
				depthTexture->setSize(m_spoutDepthFrame->GetSenderWidth(), m_spoutDepthFrame->GetSenderHeight());
				depthTexture->setTextureFormat(GL_RGBA);
				depthTexture->setBufferFormat(GL_RGBA);
				depthTexture->createTexture();
			}

			bSuccess&= m_spoutDepthFrame->ReceiveTexture(depthTexture->getGlTextureId(), GL_TEXTURE_2D);
		}

		IMkTexturePtr shadowTexture= m_parentAccessor->getShadowTexture();
		if (shadowTexture != nullptr && m_spoutShadowFrame != nullptr)
		{
			EASY_BLOCK("receive shadow texture");

			if (m_spoutShadowFrame->IsUpdated())
			{
				shadowTexture->disposeTexture();
				shadowTexture->setSize(m_spoutShadowFrame->GetSenderWidth(), m_spoutShadowFrame->GetSenderHeight());
				shadowTexture->setTextureFormat(GL_RGBA);
				shadowTexture->setBufferFormat(GL_RGBA);
				shadowTexture->createTexture();
			}

			bSuccess&= m_spoutShadowFrame->ReceiveTexture(shadowTexture->getGlTextureId(), GL_TEXTURE_2D);
		}

		return bSuccess;
	}

private:
	SharedTextureReadAccessor* m_parentAccessor;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
	SPOUTLIBRARY* m_spoutShadowFrame;
};

//-- SharedTextureReadAccessor -----
struct RenderTargetReaderImpl
{
	union
	{
		SpoutTextureReader* spoutTextureReader;
	} readerApi;
	MikanClientGraphicsApi graphicsAPI;
};

SharedTextureReadAccessor::SharedTextureReadAccessor(const std::string& senderPrefix, MikanCameraID cameraId)
	: m_senderPrefix(senderPrefix)
	, m_cameraId(cameraId)
	, m_colorTexture(nullptr)
	, m_depthTexture(nullptr)
	, m_shadowTexture(nullptr)
	, m_readerImpl(new RenderTargetReaderImpl)
{
	m_descriptor= MikanRenderTargetDescriptor();
	m_readerImpl->readerApi.spoutTextureReader= nullptr;
	m_readerImpl->graphicsAPI= MikanClientGraphicsApi_UNKNOWN;
}

SharedTextureReadAccessor::~SharedTextureReadAccessor()
{
	dispose();
	delete m_readerImpl;
	m_readerImpl= nullptr;
}

bool SharedTextureReadAccessor::initialize(const MikanRenderTargetDescriptor* descriptor)
{
	bool bSuccess= false;

	dispose();

	m_descriptor= *descriptor;
	m_lastFrameRenderedIndex= 0;

	// Make a spout sender name for the color buffer
	if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::COLOR, m_colorSenderName))
	{
		MIKAN_LOG_ERROR("SharedTextureReadAccessor") << "Failed to create spout color texture sender name.";
		return false;
	}

	// Make a spout sender name for the depth buffer
	if (descriptor->depth_buffer_type != MikanDepthBuffer_NODEPTH)
	{
		if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::DEPTH, m_depthSenderName))
		{
			MIKAN_LOG_ERROR("SharedTextureReadAccessor") << "Failed to create spout depth texture sender name.";
			return false;
		}
	}
	else
	{
		m_depthSenderName= "";
	}

	// Make a spout sender name for the (optional) shadow buffer
	if (descriptor->shadow_buffer_type != MikanShadowBuffer_NOSHADOW)
	{
		if (!makeSpoutSenderName(m_senderPrefix, m_cameraId, SharedTextureType::SHADOW, m_shadowSenderName))
		{
			MIKAN_LOG_ERROR("SharedTextureReadAccessor") << "Failed to create spout shadow texture sender name.";
			return false;
		}
	}
	else
	{
		m_shadowSenderName= "";
	}

	// Allocate a reader implementation
	if (descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D9
		|| descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D11
		|| descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D12
		|| descriptor->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		m_readerImpl->readerApi.spoutTextureReader= new SpoutTextureReader(this);
		m_readerImpl->graphicsAPI= descriptor->graphicsAPI;

		bSuccess= m_readerImpl->readerApi.spoutTextureReader->init(descriptor);
	}

	return bSuccess;
}

void SharedTextureReadAccessor::dispose()
{
	assert(m_readerImpl != nullptr);
	if (m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D9
		|| m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11
		|| m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D12
		|| m_readerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		if (m_readerImpl->readerApi.spoutTextureReader != nullptr)
		{
			m_readerImpl->readerApi.spoutTextureReader->dispose();
			delete m_readerImpl->readerApi.spoutTextureReader;
			m_readerImpl->readerApi.spoutTextureReader= nullptr;
		}
	}

	m_readerImpl->graphicsAPI= MikanClientGraphicsApi_UNKNOWN;
	m_descriptor= {};
}

bool SharedTextureReadAccessor::readRenderTargetTextures(const int64_t newFrameIndex)
{
	bool bSuccess= false;

	if (m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D9
		|| m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11
		|| m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D12
		|| m_readerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		m_lastFrameRenderedIndex= newFrameIndex;

		if (m_readerImpl->readerApi.spoutTextureReader != nullptr
			&& m_readerImpl->readerApi.spoutTextureReader->readRenderTargetTexture())
		{
			bSuccess= true;
		}
	}

	return bSuccess;
}