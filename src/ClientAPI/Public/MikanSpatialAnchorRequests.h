#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanSpatialAnchorTypes.h"
#include "SerializationProperty.h"
#include "SerializableString.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSpatialAnchorRequests.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorRequest")) GetSpatialAnchorList :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetSpatialAnchorList";
	GetSpatialAnchorList() : MikanRequest(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetSpatialAnchorList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorRequest")) GetSpatialAnchorInfo :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetSpatialAnchorList";
	GetSpatialAnchorInfo() : MikanRequest(k_typeName) {}

	FIELD()
	MikanSpatialAnchorID anchorId;

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetSpatialAnchorInfo_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorRequest")) FindSpatialAnchorInfoByName :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "FindSpatialAnchorInfoByName";
	FindSpatialAnchorInfoByName() : MikanRequest(k_typeName) {}

	FIELD()
	Serialization::String anchorName;

#ifdef MIKANAPI_REFLECTION_ENABLED
	FindSpatialAnchorInfoByName_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSpatialAnchorRequests_GENERATED
#endif