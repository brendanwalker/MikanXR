//-- includes -----
#include "InterprocessRenderTargetWriter.h"
#include "InterprocessMessages.h"
#include "WebsocketInterprocessMessageClient.h"
#include "MikanClient.h"
#include "MikanCoreCAPI.h"
#include "MikanCoreTypes_json.h"
#include "JsonUtils.h"
#include "RandomUtils.h"

#include "ixwebsocket/IXNetSystem.h"

#include <assert.h>

// -- methods -----
MikanClient::MikanClient()
	: m_clientName(RandomUtils::RandomHexString(16))
	, m_renderTargetWriter(new InterprocessRenderTargetWriteAccessor(m_clientName))
	, m_messageClient(new WebsocketInterprocessMessageClient())
{
	m_messageClient->setResponseHandler([this](const std::string& utf8ResponseString) {
		responseHandler(utf8ResponseString);
	});
}

MikanClient::~MikanClient()
{
    freeRenderTargetBuffers(nullptr);
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

MikanResult MikanClient::setClientProperty(const std::string& key, const std::string& value)
{
	return m_messageClient->setClientProperty(key, value);
}

MikanResult MikanClient::connect(const std::string& host, const std::string& port)
{
	return m_messageClient->connect(host, port);
}

bool MikanClient::getIsConnected() const
{
	return m_messageClient->getIsConnected();
}

MikanResult MikanClient::disconnect()
{
	MikanResult resultCode= MikanResult_NotConnected;

	// Free any existing buffer if we called allocate already
	freeRenderTargetBuffers(nullptr);

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

MikanResult MikanClient::setResponseCallback(MikanResponseCallback callback, void* callback_userdata)
{
	m_responseCallback= callback;
	m_responseCallbackUserData= callback_userdata;

	return MikanResult_Success;
}

MikanResult MikanClient::sendRequest(
	const char* utf8_request_name,
	const char* utf8_payload,
	int request_version,
	MikanRequestID* out_request_id)
{
	if (m_messageClient->getIsConnected())
	{
		// Wrap the request in a parent JSON object
		std::stringstream ss;
		ss << "{\n";
		if (out_request_id != nullptr)
		{
			ss << "	\"requestId\":" << m_next_request_id << ",\n";
		}
		ss << "	\"requestType\":\"" << utf8_request_name << "\",\n";
		ss << "	\"version\":" << request_version << "";
		if (utf8_payload != nullptr)
		{
			ss << ",\n";
			ss << "	\"payload\":" << utf8_payload << "\n";
		}
		else
		{
			ss << "\n";
		}
		ss << "}";

		if (out_request_id != nullptr)
		{
			*out_request_id= m_next_request_id;
		}
		m_next_request_id++;

		return m_messageClient->sendRequest(ss.str());
	}

	return MikanResult_NotConnected;
}

void MikanClient::responseHandler(const std::string& utf8ResponseString)
{
	if (m_responseCallback != nullptr)
	{
		JsonSaxIntegerValueSearcher searcher;
		int requestId = -1;

		if (searcher.fetchKeyValuePair(utf8ResponseString, "requestId", requestId))
		{
			m_responseCallback(
				(MikanRequestID)requestId,
				utf8ResponseString.c_str(),
				m_responseCallbackUserData);
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

MikanResult MikanClient::shutdown()
{
	ix::uninitNetSystem();
	log_dispose();
	freeRenderTargetBuffers(nullptr);
	m_messageClient->disconnect();

	return MikanResult_Success;
}

MikanResult MikanClient::allocateRenderTargetBuffers(
	const MikanRenderTargetDescriptor& descriptor,
	MikanRequestID* out_request_id)
{
	MikanResult resultCode;

	// Fetch the cached graphics API interface, if any
	void* apiInterface = nullptr;
	if (descriptor.graphicsAPI != MikanClientGraphicsApi_UNKNOWN)
	{
		Mikan_GetGraphicsDeviceInterface(descriptor.graphicsAPI, &apiInterface);
	}

	// Create the shared memory buffer
	bool bSuccess= false;
	const bool bEnableFrameCounter= false; // use frameRendered RPC to send frame index
	if (m_renderTargetWriter->initialize(&descriptor, bEnableFrameCounter, apiInterface))
	{
		json descriptorJson = descriptor;
		std::string descriptorString = descriptorJson.dump();

		resultCode= sendRequest("allocateRenderTargetBuffers", descriptorString.c_str(), 0, out_request_id);
	}
	else
	{
		resultCode= MikanResult_SharedTextureError;
	}

	return resultCode;
}

MikanResult MikanClient::publishRenderTargetTexture(void* apiTexturePtr, const MikanClientFrameRendered& frameInfo)
{
	if (m_renderTargetWriter->writeRenderTargetTexture(apiTexturePtr))
	{
		json descriptorJson = frameInfo;
		std::string descriptorString = descriptorJson.dump();

		return sendRequest("frameRendered", descriptorString.c_str(), 0, nullptr);
	}
	else
	{
		return MikanResult_SharedTextureError;
	}
}

MikanResult MikanClient::freeRenderTargetBuffers(MikanRequestID* out_request_id)
{
	MikanResult resultCode= MikanResult_Success;

	// Tell the server to free it's existing render target (ignored if there isn't a render target allocated)
	if (m_messageClient->getIsConnected())
	{
		resultCode= sendRequest("freeRenderTargetBuffers", nullptr, 0, out_request_id);
	}

	// Free shared and local memory buffers
	m_renderTargetWriter->dispose();

	return resultCode;
}