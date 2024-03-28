#include "MikanRequestManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "MikanCoreTypes.h"
#include "Logger.h"

MikanResult MikanRequestManager::init()
{
	MikanResult result = Mikan_SetResponseCallback(responseHanderStatic, this);
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

void MikanRequestManager::responseHander(MikanRequestID requestId, const char* utf8ResponseString)
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

void MikanRequestManager::responseHanderStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata)
{
	MikanRequestManager* self = reinterpret_cast<MikanRequestManager*>(userdata);

	self->responseHander(requestId, utf8ResponseString);
}

MikanResponsePtr MikanRequestManager::parseResponseString(const char* utf8ResponseString)
{
	MikanResponsePtr responsePtr;

	try
	{
		json jsonResponse = json::parse(utf8ResponseString);
		std::string requestType = jsonResponse["responseType"].get<std::string>();
		auto it = m_responseFactories.find(requestType);

		if (it != m_responseFactories.end())
		{
			MikanResponseFactory factory = it->second;

			responsePtr = factory(jsonResponse);
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