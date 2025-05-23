#include "MikanRequestManager.h"
#include "MikanAPITypes.h"
#include "MikanCoreCAPI.h"
#include "MikanCoreTypes.h"
#include "Logger.h"
#include "BinaryDeserializer.h"
#include "JsonDeserializer.h"
#include "SerializableObjectPtr.h"

#include <Refureku/Refureku.h>
#include <nlohmann/json.hpp>

#include "assert.h"

using json = nlohmann::json;

MikanAPIResult MikanRequestManager::init(MikanContext context)
{
	m_context= context;

	MikanAPIResult result = 
		(MikanAPIResult)Mikan_SetTextResponseCallback(
			context, textResponseHandlerStatic, this);
	if (result != MikanAPIResult::Success)
	{
		return result;
	}

	result = 
		(MikanAPIResult)Mikan_SetBinaryResponseCallback(
			context, binaryResponseHandlerStatic, this);
	if (result != MikanAPIResult::Success)
	{
		return result;
	}

	return MikanAPIResult::Success;
}

MikanResponseFuture MikanRequestManager::sendRequest(MikanRequest& inRequest)
{
	Serialization::MikanClassId mikanRequestTypeId = inRequest.requestTypeId;
	Serialization::RfkClassId rfkRequestTypeId = Serialization::toRfkClassId(mikanRequestTypeId);

	rfk::Struct const* requestStruct = rfk::getDatabase().getStructById(rfkRequestTypeId);
	assert(requestStruct != nullptr);

	// Stamp the request with the next available request ID
	inRequest.requestId = m_nextRequestID;
	m_nextRequestID++;

	std::string	jsonString;
	Serialization::serializeToJsonString(&inRequest, *requestStruct, jsonString);

	MikanAPIResult result =
		(MikanAPIResult)Mikan_SendRequestJSON(
			m_context,
			jsonString.c_str());

	return addResponseHandler(inRequest.requestId, result);
}

MikanResponseFuture MikanRequestManager::addResponseHandler(MikanRequestID requestId, MikanAPIResult result)
{
	MikanResponsePromise promise;
	MikanResponseFuture future(this, requestId, promise);

	if (result == MikanAPIResult::Success)
	{
		auto pendingRequest = std::make_shared<PendingRequest>();
		pendingRequest->id = requestId;
		pendingRequest->promise = std::move(promise);

		// Insert into the pending request map
		insertPendingRequest(pendingRequest);
	}
	else
	{
		auto errorResponse = std::make_shared<MikanResponse>();
		rfk::Struct const& responseStruct = MikanResponse::staticGetArchetype();
		errorResponse->responseTypeId = responseStruct.getId();
		errorResponse->responseTypeName = responseStruct.getName();
		errorResponse->requestId = requestId;
		errorResponse->resultCode = result;

		promise.set_value(errorResponse);
	}

	return future;
}

void MikanRequestManager::insertPendingRequest(MikanRequestManager::PendingRequestPtr pendingRequest)
{
	std::lock_guard<std::mutex> lock(m_pending_request_map_mutex);

	m_pendingRequests.insert({pendingRequest->id, pendingRequest});
}

MikanRequestManager::PendingRequestPtr MikanRequestManager::removePendingRequest(MikanRequestID requestId)
{
	std::lock_guard<std::mutex> lock(m_pending_request_map_mutex);

	PendingRequestPtr pendingRequest;
	auto it = m_pendingRequests.find(requestId);
	if (it != m_pendingRequests.end())
	{
		pendingRequest = it->second;
		m_pendingRequests.erase(it);
	}

	return pendingRequest;
}

MikanAPIResult MikanRequestManager::cancelRequest(MikanRequestID requestId)
{
	PendingRequestPtr existingRequest= removePendingRequest(requestId);

	return existingRequest ? MikanAPIResult::Success : MikanAPIResult::InvalidParam;
}

void MikanRequestManager::textResponseHander(MikanRequestID requestId, const char* utf8ResponseString)
{
	// Find the pending request and remove it from the pending request map
	PendingRequestPtr pendingRequest= removePendingRequest(requestId);

	// Fulfill the promise with the response
	if (pendingRequest)
	{
		MikanResponsePtr response = parseResponseString(utf8ResponseString);

		if (!response)
		{
			response = std::make_shared<MikanResponse>();
			rfk::Struct const& responseStruct = MikanResponse::staticGetArchetype();
			response->responseTypeId = responseStruct.getId();
			response->responseTypeName = responseStruct.getName();
			response->requestId = requestId;
			response->resultCode = MikanAPIResult::MalformedResponse;
		}

		pendingRequest->promise.set_value(response);
	}
	else
	{
		MIKAN_MT_LOG_ERROR("MikanInterface::responseHander") << "Request ID not found: " << requestId;
	}
}

void MikanRequestManager::textResponseHandlerStatic(MikanRequestID requestId, const char* utf8ResponseString, void* userdata)
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

		MikanResponse responseHeader= {};
		bool parseHeader = 
			Serialization::deserializeFromJson(
				jsonResponse, &responseHeader, MikanResponse::staticGetArchetype());
		if (!parseHeader)
		{
			throw std::runtime_error("Failed to parse response header");
		}

		Serialization::MikanClassId mikanResponseTypeId = responseHeader.responseTypeId;
		Serialization::RfkClassId rfkResponseTypeId = Serialization::toRfkClassId(mikanResponseTypeId);
		rfk::Struct const* responseStruct = rfk::getDatabase().getStructById(rfkResponseTypeId);
		if (responseStruct != nullptr)
		{
			responsePtr = responseStruct->makeSharedInstance<MikanResponse>();

			if (!Serialization::deserializeFromJson(jsonResponse, responsePtr.get(), *responseStruct))
			{
				std::stringstream ss;
				ss << "Failed to parse struct of type " << responseHeader.responseTypeName.getValue();

				throw std::runtime_error(ss.str());
			}
		}
		else
		{
			std::stringstream ss;
			ss << "Failed to find struct of type " << responseHeader.responseTypeName.getValue();

			throw std::runtime_error(ss.str());
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
		MikanResponse responseHeader= {};
		bool parseHeader = 
			Serialization::deserializeFromBytes(
				buffer, bufferSize, &responseHeader, MikanResponse::staticGetArchetype());
		if (!parseHeader)
		{
			throw std::runtime_error("Failed to parse response header");
		}

		// Find the pending request and remove it from the pending request map
		pendingRequest= removePendingRequest(responseHeader.requestId);

		// Fulfill the promise with the response
		if (pendingRequest)
		{
			MikanResponsePtr response = parseResponseBinaryReader(responseHeader, buffer, bufferSize);

			if (!response)
			{
				rfk::Struct const& responseStruct = MikanResponse::staticGetArchetype();
				response = std::make_shared<MikanResponse>();
				response->responseTypeId = responseStruct.getId();
				response->responseTypeName = responseStruct.getName();
				response->requestId = responseHeader.requestId;
				response->resultCode = MikanAPIResult::MalformedResponse;
			}

			pendingRequest->promise.set_value(response);
		}
		else
		{
			MIKAN_MT_LOG_ERROR("MikanInterface::responseHander") 
				<< "Request ID not found: " << responseHeader.requestId;
		}
	}
	catch (std::exception& e)
	{
		MIKAN_MT_LOG_ERROR("MikanClient::binaryResponseHander()")
			<< "Failed to parse response: " << e.what();
	}
}

void MikanRequestManager::binaryResponseHandlerStatic(
	const uint8_t* buffer,
	size_t bufferSize,
	void* userdata)
{
	MikanRequestManager* self = reinterpret_cast<MikanRequestManager*>(userdata);

	self->binaryResponseHander(buffer, bufferSize);
}

MikanResponsePtr MikanRequestManager::parseResponseBinaryReader(
	const MikanResponse& responseHeader,
	const uint8_t* buffer,
	size_t bufferSize)
{
	MikanResponsePtr responsePtr;

	Serialization::MikanClassId mikanResponseTypeId = responseHeader.responseTypeId;
	Serialization::RfkClassId rfkResponseTypeId = Serialization::toRfkClassId(mikanResponseTypeId);
	rfk::Struct const* responseStruct = rfk::getDatabase().getStructById(rfkResponseTypeId);
	if (responseStruct != nullptr)
	{
		responsePtr = responseStruct->makeSharedInstance<MikanResponse>();

		Serialization::deserializeFromBytes(buffer, bufferSize, responsePtr.get(), *responseStruct);
	}
	else
	{
		MIKAN_MT_LOG_WARNING("MikanClient::parseResponseBinaryReader()")
			<< "Received response of unknown responseType: " << responseHeader.responseTypeId;
	}

	return responsePtr;
}