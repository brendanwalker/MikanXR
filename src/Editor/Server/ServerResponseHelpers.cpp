#include "ServerResponseHelpers.h"

void writeSimpleJsonResponse(MikanRequestID requestId, MikanAPIResult result, ClientResponse& response)
{
	// Only write a response if the request ID is valid (i.e. the client expects a response)
	if (requestId != INVALID_MIKAN_ID)
	{
		MikanResponse mikanResponse;
		mikanResponse.requestId = requestId;
		mikanResponse.resultCode = result;

		Serialization::serializeToJsonString(mikanResponse, response.utf8String);
	}
	else
	{
		response.utf8String = "";
	}
}

void writeSimpleBinaryResponse(MikanRequestID requestId, MikanAPIResult result, ClientResponse& response)
{
	// Only write a response if the request ID is valid (i.e. the client expects a response)
	if (requestId != INVALID_MIKAN_ID)
	{
		MikanResponse mikanResponse;
		mikanResponse.requestId = requestId;
		mikanResponse.resultCode = result;

		Serialization::serializeToBytes<MikanResponse>(mikanResponse, response.binaryData);
	}
	else
	{
		response.binaryData.clear();
	}
}