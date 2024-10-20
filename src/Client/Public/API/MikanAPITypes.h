#pragma once

#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"

#ifdef REFLECTION_CODE_BUILT
#include "MikanAPITypes.rfkh.h"
#endif

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
	std::string requestType;
	FIELD()
	MikanRequestID requestId;
	FIELD()
	int version;

	MikanRequest() = default;
	MikanRequest(const std::string& inRequestType) : requestType(inRequestType) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanRequest_GENERATED
	#endif
};

struct STRUCT() MikanResponse
{
	inline static const std::string k_typeName = "MikanResponse";

	FIELD()
	std::string responseType;
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

	#ifdef REFLECTION_CODE_BUILT
	MikanResponse_GENERATED
	#endif
};

struct STRUCT() MikanEvent
{
	FIELD()
	std::string eventType;

	MikanEvent() = default;
	MikanEvent(const std::string& inEventType) : eventType(inEventType) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanEvent_GENERATED
	#endif
};

struct STRUCT() MikanClientInfo
{
	inline static const std::string k_typeName = "MikanClientInfo";

	FIELD()
	std::string clientId;
	FIELD()
	std::string engineName;
	FIELD()
	std::string engineVersion;
	FIELD()
	std::string applicationName;
	FIELD()
	std::string applicationVersion;
	FIELD()
	std::string xrDeviceName;
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

	#ifdef REFLECTION_CODE_BUILT
	MikanClientInfo_GENERATED
	#endif
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

	#ifdef REFLECTION_CODE_BUILT
	MikanColorRGB_GENERATED
	#endif
};

#ifdef REFLECTION_CODE_BUILT
File_MikanAPITypes_GENERATED
#endif