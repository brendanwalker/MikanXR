#pragma once

#include "MikanAPIExport.h"
#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"
#include "SerializableString.h"

#ifdef MIKANCORE_REFLECTION_ENABLED
#include "MikanCoreTypes.rfkh.h"
#endif // MIKANCORE_REFLECTION_ENABLED

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanAPITypes.rfkh.h"
#endif // MIKANAPI_REFLECTION_ENABLED

#include <string>

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAPITypes")) MikanRequest
{
	inline static const char* k_typeName = "MikanRequest";

	FIELD()
	Serialization::String requestType;
	FIELD()
	MikanRequestID requestId;

	MikanRequest() = default;
	MikanRequest(const std::string& inRequestType) : requestType(inRequestType) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRequest_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAPITypes")) MikanResponse
{
	inline static const char* k_typeName = "MikanResponse";

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

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAPITypes")) MikanEvent
{
	inline static const char* k_typeName = "MikanEvent";

	FIELD()
	Serialization::String eventType;

	MikanEvent() = default;
	MikanEvent(const std::string& inEventType) : eventType(inEventType) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanEvent_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAPITypes")) MikanClientInfo
{
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

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanAPITypes_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED