#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanVideoSourceTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVideoSourceRequests.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceIntrinsics :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetVideoSourceIntrinsics";
	GetVideoSourceIntrinsics() : MikanRequest(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceIntrinsics_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceMode :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetVideoSourceIntrinsics";
	GetVideoSourceMode() : MikanRequest(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceMode_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceAttachment :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetVideoSourceIntrinsics";
	GetVideoSourceAttachment() : MikanRequest(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceAttachment_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceRequests_GENERATED
#endif