#pragma once

#include "MikanAPIExport.h"
#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"
#include "SerializableString.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanAPITypes.rfkh.h"
#endif // MIKANAPI_REFLECTION_ENABLED

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

	FIELD()
	Serialization::String requestType;
	FIELD()
	MikanRequestID requestId;
	FIELD()
	int version;

	MikanRequest() = default;
	MikanRequest(const std::string& inRequestType) : requestType(inRequestType) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRequest_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct STRUCT() MikanResponse
{
	inline static const std::string k_typeName = "MikanResponse";

	FIELD()
	Serialization::String responseType;
	FIELD()
	MikanRequestID requestId;
	FIELD()
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

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanResponse_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct STRUCT() MikanEvent
{
	FIELD()
	Serialization::String eventType;

	MikanEvent() = default;
	MikanEvent(const std::string& inEventType) : eventType(inEventType) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanEvent_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct STRUCT(Serialization::CodeGenModule("MikanAPI")) MikanClientInfo
{
	inline static const std::string k_typeName = "MikanClientInfo";

	FIELD()
	Serialization::String clientId;
	FIELD()
	Serialization::String engineName;
	FIELD()
	Serialization::String engineVersion;
	FIELD()
	Serialization::String applicationName;
	FIELD()
	Serialization::String applicationVersion;
	FIELD()
	Serialization::String xrDeviceName;
	FIELD()
	MikanClientGraphicsApi graphicsAPI;
	FIELD()
	int mikanCoreSdkVersion;
	FIELD()
	bool supportsRGB24;
	FIELD()
	bool supportsRGBA32;
	FIELD()
	bool supportsBGRA32;
	FIELD()
	bool supportsDepth;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanClientInfo_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

/// A float RGB color with [0,1] components.
struct STRUCT() MikanColorRGB
{
	inline static const std::string k_typeName = "MikanColorRGB";

	FIELD()
	float r;
	FIELD()
	float g;
	FIELD()
	float b;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanColorRGB_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanAPITypes_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED