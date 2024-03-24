#pragma once

#include "MikanCoreCAPI.h"
#include "MikanClientTypes.h"
#include "MikanCoreTypes.h"
#include "MikanMathTypes.h"
#include "MikanVideoSourceTypes.h"
#include "Logger.h"

#include "MikanClientTypes_json.h"
#include "MikanCoreTypes_json.h"
#include "MikanMathTypes_json.h"
#include "MikanVideoSourceTypes_json.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using MikanResponsePtr = std::shared_ptr<MikanResponse>;
using MikanResponsePromise = std::promise<MikanResponsePtr>;
using MikanResponseFuture = std::future<MikanResponsePtr>;
using MikanResponseFactory = std::function<MikanResponsePtr(const std::string& responseType)>;

class MikanRequestManager
{
public:
	MikanRequestManager() = default;

	MikanResult init()
	{
		MikanResult result = Mikan_SetResponseCallback(responseHanderStatic, this);
		if (result != MikanResult_Success)
		{
			return result;
		}

		return MikanResult_Success;
	}

	MikanResponseFuture sendRequest(const std::string& requestType, int version= 0)
	{
		return sendRequestInternal(requestType, "", version);
	}

	template <typename t_payload_type>
	MikanResponseFuture sendRequestWithPayload(
		const std::string& requestType, 
		const t_payload_type& payload,
		int version= 0)
	{
		// Use nlohmann/json to serialize the payload
		json payloadJson = payload;

		// Convert the json object to a string
		std::string payloadString = payloadJson.dump();

		return sendRequestInternal(requestType, payloadString, version);
	}

	MikanResponseFuture sendRequestInternal(
		const std::string& requestType, 
		const std::string& payloadString,
		int version= 0)
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

	template <typename t_response_type>
	void addResponseFactory()
	{
		MikanResponseFactory factory =
			[](json jsonResponse) -> MikanResponsePtr {
			auto responsePtr = std::make_shared<t_response_type>();

			*responsePtr = jsonResponse.get<t_response_type>();

			return responsePtr;
		};

		m_responseFactories.insert(std::make_pair(t_response_type::k_typeName, factory));
	}

	MikanResponseFuture addResponseHandler(MikanRequestID requestId, MikanResult result)
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
			errorResponse->responstType = MikanResponse::k_typeName;
			errorResponse->requestId = requestId;
			errorResponse->resultCode = result;

			promise.set_value(errorResponse);
		}

		return future;
	}

	void responseHander(MikanRequestID requestId, const char* utf8ResponseString)
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

protected:
	static void responseHanderStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata)
	{
		MikanRequestManager* self = reinterpret_cast<MikanRequestManager*>(userdata);

		self->responseHander(requestId, utf8ResponseString);
	}

	MikanResponsePtr parseResponseString(const char* utf8ResponseString)
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

private:
	std::map<std::string, MikanResponseFactory> m_responseFactories;

	struct PendingRequest
	{
		MikanRequestID id;
		MikanResponsePromise promise;
	};
	using PendingRequestPtr = std::shared_ptr<PendingRequest>;

	std::map<MikanRequestID, PendingRequestPtr> m_pendingRequests;
	std::mutex m_pending_request_map_mutex;
};
