#pragma once
/**
\file
*/

#pragma once

#include "MikanTypeFwd.h"
#include "MikanCoreTypes.h"


#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
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

	std::string requestType;
	MikanRequestID requestId;
	int version;

	MikanRequest() = default;
	MikanRequest(const std::string& inRequestType) : requestType(inRequestType) {}

	#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
	MikanRequest_GENERATED
	#endif
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

	#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
	MikanResponse_GENERATED
	#endif
};

struct STRUCT() MikanEvent
{
	std::string eventType;

	MikanEvent() = default;
	MikanEvent(const std::string& inEventType) : eventType(inEventType) {}

	#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
	MikanEvent_GENERATED
	#endif
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

	#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
	MikanClientInfo_GENERATED
	#endif
};

/// A float RGB color with [0,1] components.
struct STRUCT() MikanColorRGB
{
	inline static const std::string k_typeName = "MikanColorRGB";

	float r, g, b;

	#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
	MikanColorRGB_GENERATED
	#endif
};

#if defined(ENABLE_REFLECTION) && !defined(KODGEN_PARSING)
File_MikanAPITypes_GENERATED
#endif // ENABLE_REFLECTION

/**
@}
*/

//cut_after
