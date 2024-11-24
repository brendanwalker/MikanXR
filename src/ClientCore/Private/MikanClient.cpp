//-- includes -----
#include "InterprocessRenderTargetWriter.h"
#include "InterprocessMessages.h"
#include "WebsocketInterprocessMessageClient.h"
#include "MikanClient.h"
#include "MikanCoreCAPI.h"
#include "MikanCoreTypes.h"
#include "JsonUtils.h"
#include "JsonSerializer.h"
#include "RandomUtils.h"

#include "ixwebsocket/IXNetSystem.h"

#include <assert.h>

// -- methods -----
MikanClient::MikanClient()
	: m_clientUniqueID(RandomUtils::RandomHexString(16))
	, m_renderTargetWriter(new InterprocessRenderTargetWriteAccessor(m_clientUniqueID))
	, m_messageClient(new WebsocketInterprocessMessageClient())
{
	for (int i = 0; i < MikanClientGraphicsApi_COUNT; i++)
	{
		m_graphicsDeviceInterfaces[i] = nullptr;
	};

	m_messageClient->setTextResponseHandler([this](const std::string& utf8ResponseString) {
		textResponseHandler(utf8ResponseString);
	});
	m_messageClient->setBinaryResponseHandler([this](const uint8_t* buffer, size_t bufferSize) {
		binaryResponseHandler(buffer, bufferSize);
	});
}

MikanClient::~MikanClient()
{
	freeRenderTargetTextures();
	delete m_renderTargetWriter;
	delete m_messageClient;
}

// -- ClientMikanAPI System -----
MikanResult MikanClient::startup(LogSeverityLevel log_level, t_logCallback log_callback)
{
	MikanResult resultCode= MikanResult_Success;

	// Reset status flags
	m_bIsConnected= false;

	LoggerSettings settings = {};
	settings.min_log_level = log_level;
	settings.log_callback= log_callback;

	log_init(settings);

	if (!ix::initNetSystem())
	{
		MIKAN_LOG_WARNING("WebsocketInterprocessMessageServer::initialize()")
			<< "Failed to initialize net system";
		resultCode= MikanResult_InitFailed;
	}

    return resultCode;
}

MikanResult MikanClient::setClientInfo(const std::string& clientInfo)
{
	return m_messageClient->setClientInfo(clientInfo);
}

MikanResult MikanClient::connect(const std::string& host, const std::string& port)
{
	return m_messageClient->connect(host, port);
}

bool MikanClient::getIsConnected() const
{
	return m_messageClient->getIsConnected();
}

const std::string& MikanClient::getClientUniqueID() const
{
	return m_clientUniqueID;
}

MikanResult MikanClient::disconnect()
{
	MikanResult resultCode= MikanResult_NotConnected;

	// Free any existing buffer if we called allocate already
	freeRenderTargetTextures();

	if (m_messageClient->getIsConnected())
	{
		m_messageClient->disconnect();
		resultCode= MikanResult_Success;
	}

	return resultCode;
}

MikanResult MikanClient::fetchNextEvent(
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written)
{
	if (m_messageClient->getIsConnected())
	{
		return m_messageClient->fetchNextEvent(utf8_buffer_size, out_utf8_buffer, out_utf8_bytes_written);
	}

	return MikanResult_NotConnected;
}

MikanResult MikanClient::setTextResponseCallback(MikanTextResponseCallback callback, void* callback_userdata)
{
	m_textResponseCallback= callback;
	m_textResponseCallbackUserData= callback_userdata;

	return MikanResult_Success;
}

MikanResult MikanClient::setBinaryResponseCallback(MikanBinaryResponseCallback callback, void* callback_userdata)
{
	m_binaryResponseCallback = callback;
	m_binaryResponseCallbackUserData = callback_userdata;

	return MikanResult_Success;
}

MikanResult MikanClient::sendRequestJSON(const char* utf8_request_json)
{
	if (m_messageClient->getIsConnected())
	{
		return m_messageClient->sendRequest(utf8_request_json);
	}

	return MikanResult_NotConnected;
}

void MikanClient::textResponseHandler(const std::string& utf8ResponseString)
{
	if (m_textResponseCallback != nullptr)
	{
		JsonSaxIntegerValueSearcher searcher;
		int requestId = -1;

		if (searcher.fetchKeyValuePair(utf8ResponseString, "requestId", requestId))
		{
			m_textResponseCallback(
				(MikanRequestID)requestId,
				utf8ResponseString.c_str(),
				m_textResponseCallbackUserData);
		}
		else
		{
			MIKAN_MT_LOG_WARNING("MikanClient::responseHandler()")
				<< "Received response missing a request id: " << utf8ResponseString;
		}
	}
	else
	{
		MIKAN_MT_LOG_WARNING("MikanClient::responseHandler()") << "No response callback set";
	}
}

void MikanClient::binaryResponseHandler(const uint8_t* buffer, size_t bufferSize)
{
	if (m_binaryResponseCallback != nullptr)
	{
		m_binaryResponseCallback(buffer, bufferSize, m_binaryResponseCallbackUserData);
	}
	else
	{
		MIKAN_MT_LOG_WARNING("MikanClient::binaryResponseHandler()") << "No binary response callback set";
	}
}

MikanResult MikanClient::shutdown()
{
	ix::uninitNetSystem();
	log_dispose();
	m_messageClient->disconnect();

	return MikanResult_Success;
}

MikanResult MikanClient::allocateRenderTargetTextures(
	const MikanRenderTargetDescriptor& desiredDescriptor)
{
	MikanResult resultCode;

	// Fetch the cached graphics API interface, if any
	void* apiInterface = nullptr;
	if (desiredDescriptor.graphicsAPI != MikanClientGraphicsApi_UNKNOWN)
	{
		Mikan_GetGraphicsDeviceInterface(this, desiredDescriptor.graphicsAPI, &apiInterface);
	}

	// Create the shared texture
	bool bSuccess = false;
	const bool bEnableFrameCounter = false; // use frameRendered RPC to send frame index
	if (m_renderTargetWriter->initialize(&desiredDescriptor, bEnableFrameCounter, apiInterface))
	{
		resultCode = MikanResult_Success;
	}
	else
	{
		resultCode = MikanResult_SharedTextureError;
	}

	return resultCode;
}

MikanResult MikanClient::getRenderTargetDescriptor(MikanRenderTargetDescriptor& outDescriptor)
{
	const MikanRenderTargetDescriptor* desc= m_renderTargetWriter->getRenderTargetDescriptor();
	if (desc != nullptr)
	{
		outDescriptor= *desc;

		return MikanResult_Success;
	}

	return MikanResult_Uninitialized;
}

MikanResult MikanClient::freeRenderTargetTextures()
{
	// Free shared and local memory buffers
	m_renderTargetWriter->dispose();

	return MikanResult_Success;
}

MikanResult MikanClient::writeColorRenderTargetTexture(void* apiColorTexturePtr)
{
	if (m_renderTargetWriter->writeColorFrameTexture(apiColorTexturePtr))
	{
		return MikanResult_Success;
	}

	return MikanResult_SharedTextureError;
}

MikanResult MikanClient::writeDepthRenderTargetTexture(void* apiDepthTexturePtr, float zNear, float zFar)
{
	MikanDepthBufferType depthBufferType = m_renderTargetWriter->getRenderTargetDescriptor()->depth_buffer_type;

	if (depthBufferType != MikanDepthBuffer_NODEPTH)
	{
		if (m_renderTargetWriter->writeDepthFrameTexture(apiDepthTexturePtr, zNear, zFar))
		{
			return MikanResult_Success;
		}
	}

	return MikanResult_SharedTextureError;
}

void* MikanClient::getPackDepthTextureResourcePtr() const
{
	return m_renderTargetWriter != nullptr ? m_renderTargetWriter->getPackDepthTextureResourcePtr() : nullptr;
}

MikanResult MikanClient::setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsApi_COUNT)
		return MikanResult_InvalidAPI;

	m_graphicsDeviceInterfaces[api] = graphicsDeviceInterface;

	return MikanResult_Success;
}

MikanResult MikanClient::getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsApi_COUNT)
		return MikanResult_InvalidAPI;
	if (outGraphicsDeviceInterface == nullptr)
		return MikanResult_NullParam;

	*outGraphicsDeviceInterface = m_graphicsDeviceInterfaces[api];
	return MikanResult_Success;
}