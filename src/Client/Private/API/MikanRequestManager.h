#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class IMikanResponseFactory
{
public:
	virtual MikanResponsePtr createResponse(json jsonResponse) = 0;
};
using IMikanResponseFactoryPtr = std::shared_ptr<IMikanResponseFactory>;

template <typename t_response_type>
class MikanResponseFactoryTyped : public IMikanResponseFactory
{
public:
	virtual MikanResponsePtr createResponse(json jsonEvent) override
	{
		auto responsePtr = std::make_shared<t_response_type>();

		t_response_type localResponse;
		from_json(jsonEvent, localResponse);

		*responsePtr = localResponse;

		return responsePtr;
	}
};

class MikanRequestManager
{
public:
	MikanRequestManager() = default;

	MikanResult init();
	MikanResponseFuture sendRequest(const std::string& requestType, int version= 0);

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

	template <typename t_response_type>
	void addResponseFactory()
	{
		IMikanResponseFactoryPtr factory = std::make_shared<MikanResponseFactoryTyped<t_response_type>>();

		m_responseFactories.insert(std::make_pair(t_response_type::k_typeName, factory));
	}

	MikanResponseFuture addResponseHandler(MikanRequestID requestId, MikanResult result);

protected:
	MikanResponseFuture sendRequestInternal(
		const std::string& requestType,
		const std::string& payloadString,
		int version = 0);

	static void responseHanderStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata);
	void responseHander(MikanRequestID requestId, const char* utf8ResponseString);
	MikanResponsePtr parseResponseString(const char* utf8ResponseString);

private:
	std::map<std::string, IMikanResponseFactoryPtr> m_responseFactories;

	struct PendingRequest
	{
		MikanRequestID id;
		MikanResponsePromise promise;
	};
	using PendingRequestPtr = std::shared_ptr<PendingRequest>;

	std::map<MikanRequestID, PendingRequestPtr> m_pendingRequests;
	std::mutex m_pending_request_map_mutex;
};
