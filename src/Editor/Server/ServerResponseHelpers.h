#pragma once

#include "BinarySerializer.h"
#include "JsonDeserializer.h"
#include "JsonSerializer.h"
#include "InterprocessMessageServerInterface.h"
#include "Logger.h"
#include "MikanAPITypes.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

template <typename t_mikan_type>
bool readTypedRequest(const std::string& utf8RequestString, t_mikan_type& outParameters)
{
	try
	{
		return Serialization::deserializeFromJsonString(utf8RequestString, outParameters);
	}
	catch (json::exception& e)
	{
		MIKAN_LOG_ERROR("MikanServer::readRequestPayload") << "Failed to parse JSON: " << e.what();
		return false;
	}
}

template <typename t_mikan_type>
void writeTypedJsonResponse(MikanRequestID requestId, t_mikan_type& result, ClientResponse& response)
{
	result.requestId = requestId;
	result.resultCode = MikanAPIResult::Success;

	Serialization::serializeToJsonString(result, response.utf8String);
}

template <typename t_mikan_type>
void writeTypedBinaryResponse(
	MikanRequestID requestId,
	t_mikan_type& result,
	ClientResponse& response)
{
	result.requestId = requestId;
	result.resultCode = MikanAPIResult::Success;

	Serialization::serializeToBytes<t_mikan_type>(result, response.binaryData);
}

void writeSimpleJsonResponse(MikanRequestID requestId, MikanAPIResult result, ClientResponse& response);
void writeSimpleBinaryResponse(MikanRequestID requestId, MikanAPIResult result, ClientResponse& response);