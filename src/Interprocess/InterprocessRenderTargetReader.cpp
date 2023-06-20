#include "InterprocessRenderTargetReader.h"
#include "InterprocessRenderTargetShared.h"
#include "GlTexture.h"
#include "Logger.h"
#include "SpoutLibrary.h"

#include <easy/profiler.h>

class BoostSharedMemoryReader
{
public:
	BoostSharedMemoryReader(
		InterprocessRenderTargetReadAccessor* parentAccessor,
		const std::string& clientName, 
		MikanRenderTargetMemory& localMemory)
		: m_parentAccessor(parentAccessor)
		, m_sharedMemoryName(clientName + "_renderTarget")
		, m_sharedMemoryObject(nullptr)
		, m_region(nullptr)
		, m_localMemory(localMemory)
	{
	}

	virtual ~BoostSharedMemoryReader()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor)
	{
		bool bSuccess = false;

		dispose();

		try
		{
			MIKAN_LOG_INFO("BoostSharedMemoryReader::initialize()") << "Opening shared memory: " << m_sharedMemoryName;

			// Create the shared memory object
			m_sharedMemoryObject =
				new boost::interprocess::shared_memory_object(
					boost::interprocess::open_only,
					m_sharedMemoryName.c_str(),
					boost::interprocess::read_write); // needs to be read/write for mutex access

			// Map all of the shared memory for read/write access
			m_region = new boost::interprocess::mapped_region(*m_sharedMemoryObject, boost::interprocess::read_write);

			// Allocate local memory used to copy render target data from shared memory
			initializeRenderTargetMemory(descriptor, &m_localMemory);

			bSuccess = true;
		}
		catch (boost::interprocess::interprocess_exception& ex)
		{
			dispose();
			MIKAN_LOG_ERROR("SharedMemory::initialize()")
				<< "Failed to allocated shared memory: " << m_sharedMemoryName
				<< ", reason: " << ex.what();
		}
		catch (std::exception& ex)
		{
			dispose();
			MIKAN_LOG_ERROR("SharedMemory::initialize()")
				<< "Failed to allocated shared memory: " << m_sharedMemoryName
				<< ", reason: " << ex.what();
		}

		return bSuccess;
	}

	void dispose()
	{
		// Clean up the local memory
		disposeRenderTargetMemory(&m_localMemory);

		if (m_region != nullptr)
		{
			delete m_region;
			m_region = nullptr;
		}

		if (m_sharedMemoryObject != nullptr)
		{
			delete m_sharedMemoryObject;
			m_sharedMemoryObject = nullptr;
		}
	}

	bool readRenderTargetMemory()
	{
		if (m_region == nullptr || m_sharedMemoryObject == nullptr)
			return false;

		bool bReadOk = false;

		{
			EASY_BLOCK("copy from shared memory");

			InterprocessRenderTargetView* sharedMemoryView = getRenderTargetView();
			boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(sharedMemoryView->getMutex());

			// Make sure the shared memory is the size we expect
			assert(m_region->get_size() >= sharedMemoryView->getTotalSize());

			// Copy over the render target buffers if the frame index changed
			InterprocessRenderTargetHeader& sharedMemoryHeader = sharedMemoryView->getHeader();
			if (m_localMemory.width == sharedMemoryHeader.width &&
				m_localMemory.height == sharedMemoryHeader.height &&
				m_localMemory.color_buffer_size == sharedMemoryHeader.colorBufferSize &&
				m_localMemory.depth_buffer_size == sharedMemoryHeader.depthBufferSize)
			{
				// Copy over the shared memory color buffer into local memory
				if (sharedMemoryHeader.colorBufferSize > 0)
				{
					assert(m_localMemory.color_buffer != nullptr);
					std::memcpy(
						m_localMemory.color_buffer,
						sharedMemoryView->getBuffer(MikanRenderTarget_COLOR),
						m_localMemory.color_buffer_size);
				}

				// Copy over the shared memory depth buffer into local memory
				if (sharedMemoryHeader.depthBufferSize > 0)
				{
					assert(m_localMemory.color_buffer != nullptr);
					std::memcpy(
						m_localMemory.depth_buffer,
						sharedMemoryView->getBuffer(MikanRenderTarget_DEPTH),
						m_localMemory.depth_buffer_size);
				}

				bReadOk = true;
			}
		}

		if (bReadOk)
		{
			GlTexturePtr colorTexture= m_parentAccessor->getColorTexture();
			if (colorTexture != nullptr)
			{
				EASY_BLOCK("copy to color texture");

				colorTexture->copyBufferIntoTexture(m_localMemory.color_buffer);
			}

			GlTexturePtr depthTexture = m_parentAccessor->getDepthTexture();
			if (depthTexture != nullptr)
			{
				EASY_BLOCK("copy to depth texture");

				depthTexture->copyBufferIntoTexture(m_localMemory.depth_buffer);
			}

		}

		return bReadOk;
	}

	InterprocessRenderTargetView* getRenderTargetView()
	{
		return reinterpret_cast<InterprocessRenderTargetView*>(m_region->get_address());
	}
private:
	InterprocessRenderTargetReadAccessor* m_parentAccessor;
	std::string m_sharedMemoryName;
	boost::interprocess::shared_memory_object* m_sharedMemoryObject;
	boost::interprocess::mapped_region* m_region;
	MikanRenderTargetMemory& m_localMemory;
};

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
		BoostSharedMemoryReader* boostSharedMemoryReader;
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
	memset(&m_localMemory, 0, sizeof(MikanRenderTargetMemory));
	m_readerImpl->readerApi.boostSharedMemoryReader= nullptr;
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
	else
	{
		m_readerImpl->readerApi.boostSharedMemoryReader = new BoostSharedMemoryReader(this, m_clientName, m_localMemory);
		m_readerImpl->graphicsAPI = MikanClientGraphicsApi_UNKNOWN;

		bSuccess = m_readerImpl->readerApi.boostSharedMemoryReader->init(descriptor);
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
	else
	{
		if (m_readerImpl->readerApi.boostSharedMemoryReader != nullptr)
		{
			m_readerImpl->readerApi.boostSharedMemoryReader->dispose();
			delete m_readerImpl->readerApi.boostSharedMemoryReader;
			m_readerImpl->readerApi.boostSharedMemoryReader = nullptr;
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
	else
	{
		if (m_readerImpl->readerApi.boostSharedMemoryReader != nullptr &&
			m_readerImpl->readerApi.boostSharedMemoryReader->readRenderTargetMemory())
		{
			bSuccess= true;
		}
	}

	return bSuccess;
}