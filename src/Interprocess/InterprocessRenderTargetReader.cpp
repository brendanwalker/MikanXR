#include "InterprocessRenderTargetReader.h"
#include "GlTexture.h"
#include "Logger.h"
#include "SpoutLibrary.h"

#include <easy/profiler.h>

class SpoutTextureReader
{
public:
	SpoutTextureReader(
		InterprocessRenderTargetReadAccessor* parentAccessor,
		const std::string& clientName)
		: m_parentAccessor(parentAccessor)
		, m_colorSenderName(clientName+"_color")
		, m_depthSenderName(clientName+"_depth")
		, m_spoutColorFrame(nullptr)
		, m_spoutDepthFrame(nullptr)
	{
	}

	virtual ~SpoutTextureReader()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor)
	{
		dispose();

		m_spoutColorFrame = GetSpout();
		if (m_spoutColorFrame != nullptr)
		{
			m_spoutColorFrame->EnableSpoutLog();
			m_spoutColorFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
			m_spoutColorFrame->SetReceiverName(m_colorSenderName.c_str());
		}
		else
		{
			MIKAN_LOG_ERROR("SpoutTextureReader") << "Failed to open spout for sender: " << m_colorSenderName;
			return false;
		}

		if (descriptor->depth_buffer_type != MikanDepthBuffer_NODEPTH)
		{
			m_spoutDepthFrame = GetSpout();
			if (m_spoutDepthFrame != nullptr)
			{
				m_spoutDepthFrame->EnableSpoutLog();
				m_spoutDepthFrame->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
				m_spoutDepthFrame->SetReceiverName(m_depthSenderName.c_str());
			}
			else
			{
				MIKAN_LOG_ERROR("SpoutTextureReader") << "Failed to open spout for sender: " << m_depthSenderName;
				return false;
			}
		}

		return true;
	}

	void dispose()
	{
		if (m_spoutDepthFrame != nullptr)
		{
			m_spoutDepthFrame->Release();
			m_spoutDepthFrame = nullptr;
		}

		if (m_spoutDepthFrame != nullptr)
		{
			m_spoutDepthFrame->Release();
			m_spoutDepthFrame= nullptr;
		}
	}

	bool readRenderTargetTexture()
	{
		bool bSuccess = false;

		GlTexturePtr colorTexture = m_parentAccessor->getColorTexture();
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

		GlTexturePtr depthTexture = m_parentAccessor->getDepthTexture();
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

		return bSuccess;
	}

private:
	InterprocessRenderTargetReadAccessor* m_parentAccessor;
	std::string m_colorSenderName;
	std::string m_depthSenderName;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
};

//-- InterprocessRenderTargetReadAccessor -----
struct RenderTargetReaderImpl
{
	union
	{
		SpoutTextureReader* spoutTextureReader;
	} readerApi;
	MikanClientGraphicsApi graphicsAPI;
};

InterprocessRenderTargetReadAccessor::InterprocessRenderTargetReadAccessor(const std::string& clientName)
	: m_clientName(clientName)
	, m_colorTexture(nullptr)
	, m_depthTexture(nullptr)
	, m_readerImpl(new RenderTargetReaderImpl)
{
	m_descriptor = MikanRenderTargetDescriptor();
	m_readerImpl->readerApi.spoutTextureReader= nullptr;
	m_readerImpl->graphicsAPI = MikanClientGraphicsApi_UNKNOWN;
}

InterprocessRenderTargetReadAccessor::~InterprocessRenderTargetReadAccessor()
{
	dispose();
	delete m_readerImpl;
}

bool InterprocessRenderTargetReadAccessor::initialize(const MikanRenderTargetDescriptor* descriptor)
{
	bool bSuccess = false;

	dispose();

	m_descriptor= *descriptor;
	m_lastFrameRenderedIndex= 0;

	if (descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D9 ||
		descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D11 ||
		descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D12 ||
		descriptor->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		m_readerImpl->readerApi.spoutTextureReader = new SpoutTextureReader(this, m_clientName);
		m_readerImpl->graphicsAPI = descriptor->graphicsAPI;

		bSuccess = m_readerImpl->readerApi.spoutTextureReader->init(descriptor);
	}

	return bSuccess;
}

void InterprocessRenderTargetReadAccessor::dispose()
{
	if (m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D9 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D12 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		if (m_readerImpl->readerApi.spoutTextureReader != nullptr)
		{
			m_readerImpl->readerApi.spoutTextureReader->dispose();
			delete m_readerImpl->readerApi.spoutTextureReader;
			m_readerImpl->readerApi.spoutTextureReader = nullptr;
		}
	}

	m_readerImpl->graphicsAPI = MikanClientGraphicsApi_UNKNOWN;
}

bool InterprocessRenderTargetReadAccessor::readRenderTargetTextures(
	const uint64_t newFrameIndex)
{
	bool bSuccess = false;

	if (m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D9 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D12 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		m_lastFrameRenderedIndex = newFrameIndex;

		if (m_readerImpl->readerApi.spoutTextureReader != nullptr &&
			m_readerImpl->readerApi.spoutTextureReader->readRenderTargetTexture())
		{
			bSuccess = true;
		}
	}

	return bSuccess;
}