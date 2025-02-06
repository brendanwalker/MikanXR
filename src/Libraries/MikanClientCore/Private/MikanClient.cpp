//-- includes -----
#include "InterprocessRenderTargetWriter.h"
#include "WebsocketInterprocessMessageClient.h"
#include "MikanClientLogger.h"
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
	, m_messageClient(new WebsocketInterprocessMessageClient(MikanConstants_ClientAPIVersion))
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
MikanCoreResult MikanClient::startup(ClientLogSeverityLevel log_level, t_logCallback log_callback)
{
	MikanCoreResult resultCode= MikanCoreResult_Success;

	// Reset status flags
	m_bIsConnected= false;

	ClientLoggerSettings settings = {};
	settings.min_log_level = log_level;
	settings.log_callback= log_callback;

	client_log_init(settings);

	if (!ix::initNetSystem())
	{
		MIKAN_LOG_WARNING("WebsocketInterprocessMessageServer::initialize()")
			<< "Failed to initialize net system";
		resultCode= MikanCoreResult_RequestFailed;
	}

    return resultCode;
}

MikanCoreResult MikanClient::connect(
	const std::string& host, 
	const std::string& port)
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

MikanCoreResult MikanClient::disconnect(uint16_t code, const std::string& reason)
{
	MikanCoreResult resultCode= MikanCoreResult_NotConnected;

	// Free any existing buffer if we called allocate already
	freeRenderTargetTextures();

	if (m_messageClient->getIsConnected())
	{
		m_messageClient->disconnect(code, reason);
		resultCode= MikanCoreResult_Success;
	}

	return resultCode;
}

MikanCoreResult MikanClient::fetchNextEvent(
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written)
{
	// Events can arrive even when not connected (e.g. disconnect event)
	// So we don't check for connection here
	return m_messageClient->fetchNextEvent(utf8_buffer_size, out_utf8_buffer, out_utf8_bytes_written);
}

MikanCoreResult MikanClient::setTextResponseCallback(MikanTextResponseCallback callback, void* callback_userdata)
{
	m_textResponseCallback= callback;
	m_textResponseCallbackUserData= callback_userdata;

	return MikanCoreResult_Success;
}

MikanCoreResult MikanClient::setBinaryResponseCallback(MikanBinaryResponseCallback callback, void* callback_userdata)
{
	m_binaryResponseCallback = callback;
	m_binaryResponseCallbackUserData = callback_userdata;

	return MikanCoreResult_Success;
}

MikanCoreResult MikanClient::sendRequestJSON(const char* utf8_request_json)
{
	if (m_messageClient->getIsConnected())
	{
		return m_messageClient->sendRequest(utf8_request_json);
	}

	return MikanCoreResult_NotConnected;
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

MikanCoreResult MikanClient::shutdown()
{
	if (m_messageClient != nullptr)
	{
		m_messageClient->disconnect(0, "");
	}
	ix::uninitNetSystem();
	client_log_dispose();

	return MikanCoreResult_Success;
}

MikanCoreResult MikanClient::allocateRenderTargetTextures(
	const MikanRenderTargetDescriptor& desiredDescriptor)
{
	MikanCoreResult resultCode;

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
		resultCode = MikanCoreResult_Success;
	}
	else
	{
		resultCode = MikanCoreResult_RequestFailed;
	}

	return resultCode;
}

MikanCoreResult MikanClient::getRenderTargetDescriptor(MikanRenderTargetDescriptor& outDescriptor)
{
	const MikanRenderTargetDescriptor* desc= m_renderTargetWriter->getRenderTargetDescriptor();
	if (desc != nullptr)
	{
		outDescriptor= *desc;

		return MikanCoreResult_Success;
	}

	return MikanCoreResult_Uninitialized;
}

MikanCoreResult MikanClient::freeRenderTargetTextures()
{
	// Free shared and local memory buffers
	m_renderTargetWriter->dispose();

	return MikanCoreResult_Success;
}

MikanCoreResult MikanClient::writeColorRenderTargetTexture(void* apiColorTexturePtr)
{
	if (m_renderTargetWriter->writeColorFrameTexture(apiColorTexturePtr))
	{
		return MikanCoreResult_Success;
	}

	return MikanCoreResult_RequestFailed;
}

MikanCoreResult MikanClient::writeDepthRenderTargetTexture(void* apiDepthTexturePtr, float zNear, float zFar)
{
	MikanDepthBufferType depthBufferType = m_renderTargetWriter->getRenderTargetDescriptor()->depth_buffer_type;

	if (depthBufferType != MikanDepthBuffer_NODEPTH)
	{
		if (m_renderTargetWriter->writeDepthFrameTexture(apiDepthTexturePtr, zNear, zFar))
		{
			return MikanCoreResult_Success;
		}
	}

	return MikanCoreResult_RequestFailed;
}

void* MikanClient::getPackDepthTextureResourcePtr() const
{
	return m_renderTargetWriter != nullptr ? m_renderTargetWriter->getPackDepthTextureResourcePtr() : nullptr;
}

MikanCoreResult MikanClient::setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsApi_COUNT)
		return MikanCoreResult_InvalidParam;

	m_graphicsDeviceInterfaces[api] = graphicsDeviceInterface;

	return MikanCoreResult_Success;
}

MikanCoreResult MikanClient::getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsApi_COUNT)
		return MikanCoreResult_InvalidParam;
	if (outGraphicsDeviceInterface == nullptr)
		return MikanCoreResult_NullParam;

	*outGraphicsDeviceInterface = m_graphicsDeviceInterfaces[api];
	return MikanCoreResult_Success;
}