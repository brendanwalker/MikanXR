#include "InterprocessRenderTargetReader.h"
#include "InterprocessRenderTargetShared.h"
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
		, m_senderName(clientName)
		, m_spout(nullptr)
	{
	}

	virtual ~SpoutTextureReader()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor)
	{
		dispose();

		m_spout = GetSpout();
		if (m_spout == nullptr)
		{
			MIKAN_LOG_ERROR("SpoutTextureReader") << "Failed to open spout api";
			return false;
		}

		m_spout->EnableSpoutLog();
		m_spout->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
		//m_spout->SetReceiverName(m_senderName.c_str());

		return true;
	}

	void dispose()
	{
		if (m_spout != nullptr)
		{
			m_spout->Release();
			m_spout= nullptr;
		}
	}

	bool readRenderTargetTexture()
	{

		GlTexturePtr colorTexture = m_parentAccessor->getColorTexture();
		if (colorTexture != nullptr)
		{
			EASY_BLOCK("receive texture");

			if (m_spout->IsUpdated())
			{
				colorTexture->disposeTexture();
				colorTexture->setSize(m_spout->GetSenderWidth(), m_spout->GetSenderHeight());
				colorTexture->setTextureFormat(GL_RGBA);
				colorTexture->setBufferFormat(GL_RGBA);
				colorTexture->createTexture();
			}

			return (m_spout->ReceiveTexture(colorTexture->getGlTextureId(), GL_TEXTURE_2D));
		}

		return false;
	}

private:
	InterprocessRenderTargetReadAccessor* m_parentAccessor;
	std::string m_senderName;
	SPOUTLIBRARY* m_spout;
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
	memset(&m_descriptor, 0, sizeof(MikanRenderTargetDescriptor));
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

bool InterprocessRenderTargetReadAccessor::readRenderTargetMemory()
{
	bool bSuccess = false;

	if (m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D9 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D12 ||
		m_readerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		if (m_readerImpl->readerApi.spoutTextureReader != nullptr &&
			m_readerImpl->readerApi.spoutTextureReader->readRenderTargetTexture())
		{
			bSuccess = true;
		}
	}

	return bSuccess;
}