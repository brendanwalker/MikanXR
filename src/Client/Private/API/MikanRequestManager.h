#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "SerializationUtils.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ITextMikanResponseFactory
{
public:
	virtual MikanResponsePtr createResponse(json jsonResponse) = 0;
};
using IMikanTextResponseFactoryPtr = std::shared_ptr<ITextMikanResponseFactory>;

template <typename t_response_type>
class MikanTextResponseFactoryTyped : public ITextMikanResponseFactory
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

class IBinaryMikanResponseFactory
{
public:
	virtual MikanResponsePtr createResponse(
		int requestId,
		MikanResult resultCode,
		const std::string& responseType,
		class BinaryReader& reader) = 0;
};
using IMikanBinaryResponseFactoryPtr = std::shared_ptr<IBinaryMikanResponseFactory>;

template <typename t_response_type>
class MikanBinaryResponseFactoryTyped : public IBinaryMikanResponseFactory
{
public:
	virtual MikanResponsePtr createResponse(
		int requestId,
		MikanResult resultCode,
		const std::string& responseType,
		class BinaryReader& reader) override
	{
		auto responsePtr = std::make_shared<t_response_type>();
		t_response_type& responseRef= *responsePtr.get();

		from_binary(reader, responseRef);

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
	void addTextResponseFactory()
	{
		IMikanTextResponseFactoryPtr factory = std::make_shared<MikanTextResponseFactoryTyped<t_response_type>>();

		m_textResponseFactories.insert(std::make_pair(t_response_type::k_typeName, factory));
	}

	template <typename t_response_type>
	void addBinaryResponseFactory()
	{
		IMikanBinaryResponseFactoryPtr factory = std::make_shared<MikanBinaryResponseFactoryTyped<t_response_type>>();

		m_binaryResponseFactories.insert(std::make_pair(t_response_type::k_typeName, factory));
	}

	MikanResponseFuture addResponseHandler(MikanRequestID requestId, MikanResult result);

protected:
	MikanResponseFuture sendRequestInternal(
		const std::string& requestType,
		const std::string& payloadString,
		int version = 0);

	static void textResponseHanderStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata);
	void textResponseHander(MikanRequestID requestId, const char* utf8ResponseString);
	MikanResponsePtr parseResponseString(const char* utf8ResponseString);

	static void binaryResponseHanderStatic(const uint8_t* buffer, size_t bufferSize, void* userdata);
	void binaryResponseHander(const uint8_t* buffer, size_t bufferSize);
	MikanResponsePtr parseResponseBinaryReader(
		int requestId,
		MikanResult resultCode,
		const std::string& responseType,
		BinaryReader& reader);

private:
	std::map<std::string, IMikanTextResponseFactoryPtr> m_textResponseFactories;
	std::map<std::string, IMikanBinaryResponseFactoryPtr> m_binaryResponseFactories;

	struct PendingRequest
	{
		MikanRequestID id;
		MikanResponsePromise promise;
	};
	using PendingRequestPtr = std::shared_ptr<PendingRequest>;

	std::map<MikanRequestID, PendingRequestPtr> m_pendingRequests;
	std::mutex m_pending_request_map_mutex;
};
