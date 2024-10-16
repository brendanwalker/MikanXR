#include "MikanRequestManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "MikanCoreTypes.h"
#include "Logger.h"

MikanResult MikanRequestManager::init()
{
	MikanResult result = Mikan_SetTextResponseCallback(textResponseHanderStatic, this);
	if (result != MikanResult_Success)
	{
		return result;
	}

	result = Mikan_SetBinaryResponseCallback(binaryResponseHanderStatic, this);
	if (result != MikanResult_Success)
	{
		return result;
	}

	return MikanResult_Success;
}

MikanResponseFuture MikanRequestManager::sendRequest(const std::string& requestType, int version)
{
	return sendRequestInternal(requestType, "", version);
}

MikanResponseFuture MikanRequestManager::sendRequestInternal(
	const std::string& requestType,
	const std::string& payloadString,
	int version)
{
	MikanRequestID requestId = INVALID_MIKAN_ID;
	MikanResult result =
		Mikan_SendRequest(
			requestType.c_str(),
			!payloadString.empty() ? payloadString.c_str() : nullptr,
			version,
			&requestId);

	return addResponseHandler(requestId, result);
}

MikanResponseFuture MikanRequestManager::addResponseHandler(MikanRequestID requestId, MikanResult result)
{
	MikanResponsePromise promise;
	MikanResponseFuture future = promise.get_future();

	if (result == MikanResult_Success)
	{
		auto pendingRequest = std::make_shared<PendingRequest>();
		pendingRequest->id = requestId;
		pendingRequest->promise = std::move(promise);

		// Insert into the pending request map
		{
			std::lock_guard<std::mutex> lock(m_pending_request_map_mutex);

			m_pendingRequests.insert({requestId, pendingRequest});
		}
	}
	else
	{
		auto errorResponse = std::make_shared<MikanResponse>();
		errorResponse->responseType = MikanResponse::k_typeName;
		errorResponse->requestId = requestId;
		errorResponse->resultCode = result;

		promise.set_value(errorResponse);
	}

	return future;
}

void MikanRequestManager::textResponseHander(MikanRequestID requestId, const char* utf8ResponseString)
{
	PendingRequestPtr pendingRequest;

	// Find the pending request and remove it from the pending request map
	{
		std::lock_guard<std::mutex> lock(m_pending_request_map_mutex);

		auto it = m_pendingRequests.find(requestId);
		if (it != m_pendingRequests.end())
		{
			pendingRequest = it->second;
			m_pendingRequests.erase(it);
		}
	}

	// Fulfill the promise with the response
	if (pendingRequest)
	{
		MikanResponsePtr response = parseResponseString(utf8ResponseString);

		if (!response)
		{
			response = std::make_shared<MikanResponse>();
			response->responseType = MikanResponse::k_typeName;
			response->requestId = requestId;
			response->resultCode = MikanResult_MalformedResponse;
		}

		pendingRequest->promise.set_value(response);
	}
	else
	{
		MIKAN_MT_LOG_ERROR("MikanInterface::responseHander") << "Request ID not found: " << requestId;
	}
}

void MikanRequestManager::textResponseHanderStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata)
{
	MikanRequestManager* self = reinterpret_cast<MikanRequestManager*>(userdata);

	self->textResponseHander(requestId, utf8ResponseString);
}

MikanResponsePtr MikanRequestManager::parseResponseString(const char* utf8ResponseString)
{
	MikanResponsePtr responsePtr;

	try
	{
		json jsonResponse = json::parse(utf8ResponseString);
		std::string requestType = jsonResponse["responseType"].get<std::string>();
		auto it = m_textResponseFactories.find(requestType);

		if (it != m_textResponseFactories.end())
		{
			IMikanTextResponseFactoryPtr factory = it->second;

			responsePtr = factory->createResponse(jsonResponse);
		}
		else
		{
			MIKAN_MT_LOG_WARNING("MikanClient::parseResponseString()")
				<< "Received response of unknown responseType: " << requestType;
		}
	}
	catch (json::parse_error& e)
	{
		MIKAN_MT_LOG_ERROR("MikanClient::parseResponseString()")
			<< "Failed to parse response: " << e.what();
	}
	catch (json::exception& e)
	{
		MIKAN_MT_LOG_ERROR("MikanClient::parseResponseString()")
			<< "Failed to parse response: " << e.what();
	}

	return responsePtr;
}

void MikanRequestManager::binaryResponseHander(
	const uint8_t* buffer, 
	size_t bufferSize)
{
	BinaryReader reader(buffer, bufferSize);
	PendingRequestPtr pendingRequest;

	try
	{
		// Read the response type
		std::string responseType;
		from_binary(reader, responseType);

		// Read the request ID
		MikanRequestID requestId;
		from_binary(reader, requestId);

		// Read the result code
		MikanResult resultCode;
		from_binary(reader, (int32_t &)resultCode);

		// Find the pending request and remove it from the pending request map
		{
			std::lock_guard<std::mutex> lock(m_pending_request_map_mutex);

			auto it = m_pendingRequests.find(requestId);
			if (it != m_pendingRequests.end())
			{
				pendingRequest = it->second;
				m_pendingRequests.erase(it);
			}
		}

		// Fulfill the promise with the response
		if (pendingRequest)
		{
			MikanResponsePtr response = parseResponseBinaryReader(responseType, buffer, bufferSize);

			if (!response)
			{
				response = std::make_shared<MikanResponse>();
				response->responseType = MikanResponse::k_typeName;
				response->requestId = requestId;
				response->resultCode = MikanResult_MalformedResponse;
			}

			pendingRequest->promise.set_value(response);
		}
		else
		{
			MIKAN_MT_LOG_ERROR("MikanInterface::responseHander") << "Request ID not found: " << requestId;
		}
	}
	catch (std::out_of_range& e)
	{
		MIKAN_MT_LOG_ERROR("MikanClient::binaryResponseHander()")
			<< "Failed to parse response: " << e.what();
	}
}

void MikanRequestManager::binaryResponseHanderStatic(
	const uint8_t* buffer,
	size_t bufferSize,
	void* userdata)
{
	MikanRequestManager* self = reinterpret_cast<MikanRequestManager*>(userdata);

	self->binaryResponseHander(buffer, bufferSize);
}

MikanResponsePtr MikanRequestManager::parseResponseBinaryReader(
	const std::string& responseType,
	const uint8_t* buffer,
	size_t bufferSize)
{
	MikanResponsePtr responsePtr;

	auto it = m_binaryResponseFactories.find(responseType);
	if (it != m_binaryResponseFactories.end())
	{
		BinaryReader reader(buffer, bufferSize);
		IMikanBinaryResponseFactoryPtr factory = it->second;

		// Reset the read
		responsePtr = factory->createResponse(reader);
	}
	else
	{
		MIKAN_MT_LOG_WARNING("MikanClient::parseResponseString()")
			<< "Received response of unknown responseType: " << responseType;
	}

	return responsePtr;
}