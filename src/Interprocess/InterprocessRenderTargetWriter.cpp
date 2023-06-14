#include "InterprocessRenderTargetWriter.h"
#include "InterprocessRenderTargetShared.h"
#include "Logger.h"
#ifdef ENABLE_SPOUT_DX
#include "SpoutDX.h"
#endif // ENABLE_SPOUT_DX
#include "SpoutLibrary.h"

// Define DXGI_FORMAT enum values ourselves if we don't use SPOUTDX API
#ifndef ENABLE_SPOUT_DX
enum DXGI_FORMAT
{
	DXGI_FORMAT_R8G8B8A8_UNORM                          = 28,
};
#endif // !ENABLE_SPOUT_DX

class BoostSharedMemoryWriter
{
public:
	BoostSharedMemoryWriter(const std::string& clientName, MikanRenderTargetMemory& localMemory)
		: m_sharedMemoryName(clientName + "_renderTarget")
		, m_sharedMemoryObject(nullptr)
		, m_region(nullptr)
		, m_localMemory(localMemory)
	{		
	}

	virtual ~BoostSharedMemoryWriter() 
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor)
	{
		bool bSuccess= false;

		dispose();

		try
		{
			MIKAN_LOG_INFO("SharedMemory::initialize()") << "Allocating shared memory: " << m_sharedMemoryName;

			// Make sure the shared memory block has been removed first
			boost::interprocess::shared_memory_object::remove(m_sharedMemoryName.c_str());

			// Allow non admin-level processed to access the shared memory
			boost::interprocess::permissions permissions;
			permissions.set_unrestricted();

			// Create the shared memory object
			m_sharedMemoryObject =
				new boost::interprocess::shared_memory_object(
					boost::interprocess::create_only,
					m_sharedMemoryName.c_str(),
					boost::interprocess::read_write,
					permissions);

			// Resize the shared memory
			m_sharedMemoryObject->truncate(InterprocessRenderTargetView::computeTotalSize(descriptor));

			// Map all of the shared memory for read/write access
			m_region = new boost::interprocess::mapped_region(*m_sharedMemoryObject, boost::interprocess::read_write);

			// Initialize the shared memory (call constructor using placement new)
			// This make sure the mutex has the constructor called on it.
			InterprocessRenderTargetView* sharedMemoryView =
				new (getRenderTargetView()) InterprocessRenderTargetView(descriptor);
			assert(m_region->get_size() >= sharedMemoryView->getTotalSize());

			// Allocate local memory used to copy render target data into shared memory
			m_localMemory.width = descriptor->width;
			m_localMemory.height = descriptor->height;
			m_localMemory.color_buffer_size =
				computeRenderTargetColorBufferSize(
					descriptor->color_buffer_type, descriptor->width, descriptor->height);
			if (m_localMemory.color_buffer_size > 0)
			{
				m_localMemory.color_buffer = new uint8_t[m_localMemory.color_buffer_size];
				memset(m_localMemory.color_buffer, 0, m_localMemory.color_buffer_size);
			}
			m_localMemory.depth_buffer_size =
				computeRenderTargetDepthBufferSize(
					descriptor->depth_buffer_type, descriptor->width, descriptor->height);
			if (m_localMemory.depth_buffer_size > 0)
			{
				m_localMemory.depth_buffer = new uint8_t[m_localMemory.depth_buffer_size];
				memset(m_localMemory.depth_buffer, 0, m_localMemory.depth_buffer_size);
			}

			bSuccess = true;
		}
		catch (boost::interprocess::interprocess_exception* e)
		{
			dispose();
			MIKAN_LOG_ERROR("SharedMemory::initialize()")
				<< "Failed to allocated shared memory: " << m_sharedMemoryName
				<< ", reason: " << e->what();
		}

		return bSuccess;
	}

	void dispose()
	{
		// Clean up the local memory
		if (m_localMemory.color_buffer != nullptr)
		{
			delete[] m_localMemory.color_buffer;
			m_localMemory.color_buffer = nullptr;
		}
		if (m_localMemory.depth_buffer != nullptr)
		{
			delete[] m_localMemory.depth_buffer;
			m_localMemory.depth_buffer = nullptr;
		}
		m_localMemory.color_buffer_size = 0;
		m_localMemory.depth_buffer_size = 0;
		m_localMemory.width = 0;
		m_localMemory.height = 0;

		if (m_region != nullptr)
		{
			// Call the destructor manually on the frame header since it was constructed via placement new
			// This will make sure the mutex has the destructor called on it.
			getRenderTargetView()->~InterprocessRenderTargetView();

			delete m_region;
			m_region = nullptr;
		}

		if (m_sharedMemoryObject != nullptr)
		{
			delete m_sharedMemoryObject;
			m_sharedMemoryObject = nullptr;

			if (!boost::interprocess::shared_memory_object::remove(m_sharedMemoryName.c_str()))
			{
				MIKAN_LOG_WARNING("SharedMemory::dispose") << "Failed to free render target shared memory file: " << m_sharedMemoryName;
			}
		}
	}

	bool writeRenderTargetMemory(uint64_t frame_index)
	{
		if (m_region == nullptr || m_sharedMemoryObject == nullptr)
			return false;

		InterprocessRenderTargetView* sharedMemoryView = getRenderTargetView();
		boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(sharedMemoryView->getMutex());
		assert(m_region->get_size() >= sharedMemoryView->getTotalSize());

		if (m_localMemory.color_buffer != nullptr)
		{
			std::memcpy(
				sharedMemoryView->getBufferMutable(MikanRenderTarget_COLOR),
				m_localMemory.color_buffer,
				m_localMemory.color_buffer_size);
		}

		if (m_localMemory.depth_buffer != nullptr)
		{
			std::memcpy(
				sharedMemoryView->getBufferMutable(MikanRenderTarget_DEPTH),
				m_localMemory.depth_buffer,
				m_localMemory.depth_buffer_size);
		}

		// Update the frame index in shared so that the read accessor can tell there is a new frame
		sharedMemoryView->getHeader().frameIndex = frame_index;

		return true;
	}

	InterprocessRenderTargetView* getRenderTargetView()
	{
		return reinterpret_cast<InterprocessRenderTargetView*>(m_region->get_address());
	}

private:
	std::string m_sharedMemoryName;
	boost::interprocess::shared_memory_object* m_sharedMemoryObject;
	boost::interprocess::mapped_region* m_region;
	MikanRenderTargetMemory& m_localMemory;
};

class SpoutOpenGLTextureWriter
{
public:
	SpoutOpenGLTextureWriter(const std::string& clientName)
		: m_senderName(clientName)
		, m_spout(nullptr)
	{
		memset(&m_descriptor, 0, sizeof(MikanRenderTargetDescriptor));
	}

	virtual ~SpoutOpenGLTextureWriter()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor)
	{
		dispose();

		m_spout = GetSpout();
		if (m_spout == nullptr)
		{
			MIKAN_LOG_ERROR("SpoutTextureWriter") << "Failed to open spout api";
			return false;
		}

		m_spout->EnableSpoutLog();
		m_spout->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
		m_spout->SetSenderName(m_senderName.c_str());
		assert(descriptor->color_buffer_type == MikanColorBuffer_RGBA32);
		m_spout->SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
		m_spout->SetFrameCount(true);

		m_descriptor= *descriptor;

		return true;
	}

	void dispose()
	{
		if (m_spout != nullptr)
		{
			m_spout->Release();
			m_spout = nullptr;
		}
	}

	bool writeRenderTargetTexture(GLuint textureID)
	{
		return m_spout->SendTexture(textureID, GL_TEXTURE_2D, m_descriptor.width, m_descriptor.height);
	}

private:
	std::string m_senderName;
	MikanRenderTargetDescriptor m_descriptor;
	SPOUTLIBRARY* m_spout;
};

#ifdef ENABLE_SPOUT_DX
class SpoutDX11TextureWriter
{
public:
	SpoutDX11TextureWriter::SpoutDX11TextureWriter(
		const std::string& clientName)
		: m_senderName(clientName)
	{
	}

	virtual ~SpoutDX11TextureWriter()
	{
		dispose();
	}

	bool init(const MikanRenderTargetDescriptor* descriptor, void* apiDeviceInterface)
	{
		ID3D11Device* d3d11Device= (ID3D11Device*)apiDeviceInterface;
		bool bSuccess = true;

		dispose();

		if (bSuccess)
		{
			EnableSpoutLog();
			EnableSpoutLogFile("sender.log");
			SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);

			assert(descriptor->color_buffer_type == MikanColorBuffer_RGBA32);
			m_spout.OpenDirectX11(d3d11Device);
			m_spout.SetSenderName(m_senderName.c_str());
			m_spout.SetSenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		return bSuccess;
	}

	void dispose()
	{
		m_spout.ReleaseSender();
		m_spout.CloseDirectX11();

		DisableSpoutLog();
	}

	bool writeRenderTargetTexture(ID3D11Texture2D* pTexture)
	{
		return m_spout.SendTexture(pTexture);
	}

private:
	std::string m_senderName;
	spoutDX m_spout;
};
#endif // ENABLE_SPOUT_DX


//-- InterprocessRenderTargetWriteAccessor -----
struct RenderTargetWriterImpl
{
	union
	{	
#ifdef ENABLE_SPOUT_DX
		SpoutDX11TextureWriter* spoutDX11TextureWriter;
#endif // ENABLE_SPOUT_DX
		SpoutOpenGLTextureWriter* spoutOpenGLTextureWriter;
		BoostSharedMemoryWriter* boostSharedMemoryWriter;
	} writerApi;
	MikanClientGraphicsApi graphicsAPI;
};

InterprocessRenderTargetWriteAccessor::InterprocessRenderTargetWriteAccessor(const std::string& clientName)
	: m_clientName(clientName)
	, m_localFrameIndex(0)	
	, m_writerImpl(new RenderTargetWriterImpl)
{
	memset(&m_localMemory, 0, sizeof(MikanRenderTargetMemory));
	memset(m_writerImpl, 0, sizeof(RenderTargetWriterImpl));
}

InterprocessRenderTargetWriteAccessor::~InterprocessRenderTargetWriteAccessor()
{
	dispose();
	delete m_writerImpl;
}

bool InterprocessRenderTargetWriteAccessor::initialize(
	const MikanRenderTargetDescriptor* descriptor,
	void* apiDeviceInterface)
{
	dispose();

	m_localFrameIndex = 0;

	if (descriptor->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		m_writerImpl->writerApi.spoutOpenGLTextureWriter = new SpoutOpenGLTextureWriter(m_clientName);
		m_writerImpl->graphicsAPI = MikanClientGraphicsApi_OpenGL;

		m_bIsInitialized = m_writerImpl->writerApi.spoutOpenGLTextureWriter->init(descriptor);
	}
#ifdef ENABLE_SPOUT_DX
	else if (descriptor->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		m_writerImpl->writerApi.spoutDX11TextureWriter = new SpoutDX11TextureWriter(m_clientName);
		m_writerImpl->graphicsAPI = MikanClientGraphicsApi_Direct3D11;

		m_bIsInitialized = m_writerImpl->writerApi.spoutDX11TextureWriter->init(descriptor, apiDeviceInterface);
	}
#endif // ENABLE_SPOUT_DX
	else
	{
		m_writerImpl->writerApi.boostSharedMemoryWriter = new BoostSharedMemoryWriter(m_clientName, m_localMemory);
		m_writerImpl->graphicsAPI = MikanClientGraphicsApi_UNKNOWN;

		m_bIsInitialized= m_writerImpl->writerApi.boostSharedMemoryWriter->init(descriptor);
	}

	return m_bIsInitialized;
}

void InterprocessRenderTargetWriteAccessor::dispose()
{
	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		if (m_writerImpl->writerApi.spoutOpenGLTextureWriter != nullptr)
		{
			m_writerImpl->writerApi.spoutOpenGLTextureWriter->dispose();
			delete m_writerImpl->writerApi.spoutOpenGLTextureWriter;
			m_writerImpl->writerApi.spoutOpenGLTextureWriter = nullptr;
		}
	}
#ifdef ENABLE_SPOUT_DX
	else if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		if (m_writerImpl->writerApi.spoutDX11TextureWriter != nullptr)
		{
			m_writerImpl->writerApi.spoutDX11TextureWriter->dispose();
			delete m_writerImpl->writerApi.spoutDX11TextureWriter;
			m_writerImpl->writerApi.spoutDX11TextureWriter = nullptr;
		}
	}
#endif // ENABLE_SPOUT_DX
	else
	{
		if (m_writerImpl->writerApi.boostSharedMemoryWriter != nullptr)
		{
			m_writerImpl->writerApi.boostSharedMemoryWriter->dispose();
			delete m_writerImpl->writerApi.boostSharedMemoryWriter;
			m_writerImpl->writerApi.boostSharedMemoryWriter = nullptr;
		}
	}

	m_writerImpl->graphicsAPI= MikanClientGraphicsApi_UNKNOWN;
	m_bIsInitialized = false;
}

bool InterprocessRenderTargetWriteAccessor::writeRenderTargetMemory(uint64_t frame_index)
{
	bool bSuccess= false; 

	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_UNKNOWN)
	{
		bSuccess = m_writerImpl->writerApi.boostSharedMemoryWriter->writeRenderTargetMemory(frame_index);
	}
	else
	{
		bSuccess= false;
	}

	if (bSuccess)
	{
		// Also track the last frame posted locally for debugging
		m_localFrameIndex = frame_index;
	}

	return true;
}

bool InterprocessRenderTargetWriteAccessor::writeRenderTargetTexture(void* apiTexturePtr, uint64_t frameIndex)
{
	bool bSuccess = false;

	if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_OpenGL)
	{
		GLuint* textureId= (GLuint*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutOpenGLTextureWriter->writeRenderTargetTexture(*textureId);
	}
#ifdef ENABLE_SPOUT_DX
	else if (m_writerImpl->graphicsAPI == MikanClientGraphicsApi_Direct3D11)
	{
		ID3D11Texture2D* dx11Texture = (ID3D11Texture2D*)apiTexturePtr;

		bSuccess = m_writerImpl->writerApi.spoutDX11TextureWriter->writeRenderTargetTexture(dx11Texture);
	}
#endif // ENABLE_SPOUT_DX
	else
	{
		bSuccess = false;
	}

	if (bSuccess)
	{
		// Also track the last frame posted locally for debugging
		m_localFrameIndex = frameIndex;
	}

	return true;
}