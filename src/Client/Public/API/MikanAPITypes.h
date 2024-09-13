#pragma once
/**
\file
*/

#pragma once

#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"

#include "Generated/MikanAPITypes.rfkh.h"

#include <string>

#define INVALID_MIKAN_ID				-1

/// The ID of a VR Device
using MikanVRDeviceID = int32_t;

/// The ID of a stencil
using MikanStencilID = int32_t;

/// The ID of a spatial anchor
using MikanSpatialAnchorID = int32_t;

struct STRUCT() MikanRequest
{
	inline static const std::string k_typeName = "MikanRequest";

	std::string requestType;
	MikanRequestID requestId;
	int version;

	MikanRequest() = default;
	MikanRequest(const std::string& inRequestType) : requestType(inRequestType) {}

	MikanRequest_GENERATED
};

struct STRUCT() MikanResponse
{
	inline static const std::string k_typeName = "MikanResponse";

	std::string responseType;
	MikanRequestID requestId;
	MikanResult resultCode;

	MikanResponse() 
		: responseType(k_typeName) 
		, requestId(INVALID_MIKAN_ID)
		, resultCode(MikanResult_Success)
	{}
	MikanResponse(const std::string& inResponseType) 
		: responseType(inResponseType) 
		, requestId(INVALID_MIKAN_ID)
		, resultCode(MikanResult_Success)
	{}

	MikanResponse_GENERATED
};

struct STRUCT() MikanEvent
{
	std::string eventType;

	MikanEvent() = default;
	MikanEvent(const std::string& inEventType) : eventType(inEventType) {}

	MikanEvent_GENERATED
};

struct STRUCT() MikanClientInfo
{
	inline static const std::string k_typeName = "MikanClientInfo";

	std::string clientId;
	std::string engineName;
	std::string engineVersion;
	std::string applicationName;
	std::string applicationVersion;
	std::string xrDeviceName;
	MikanClientGraphicsApi graphicsAPI;
	int mikanCoreSdkVersion;
	bool supportsRGB24;
	bool supportsRGBA32;
	bool supportsBGRA32;
	bool supportsDepth;

	MikanClientInfo_GENERATED
};

/// A float RGB color with [0,1] components.
struct STRUCT() MikanColorRGB
{
	inline static const std::string k_typeName = "MikanColorRGB";

	float r, g, b;

	MikanColorRGB_GENERATED
};

File_MikanAPITypes_GENERATED
/**
@}
*/

//cut_after
