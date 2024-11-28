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
	FIELD()
	uint64_t requestTypeId = 0;
	FIELD()
	Serialization::String requestTypeName;
	FIELD()
	MikanRequestID requestId = -1;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRequest_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAPITypes")) MikanResponse
{
	FIELD()
	uint64_t responseTypeId = 0;
	FIELD()
	Serialization::String responseTypeName;
	FIELD()
	MikanRequestID requestId= INVALID_MIKAN_ID;
	FIELD()
	MikanResult resultCode= MikanResult_Success;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanResponse_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAPITypes")) MikanEvent
{
	FIELD()
	uint64_t eventTypeId = 0;
	FIELD()
	Serialization::String eventTypeName;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanEvent_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanAPITypes_GENERATED
#endif // MIKANAPI_REFLECTION_ENABLED