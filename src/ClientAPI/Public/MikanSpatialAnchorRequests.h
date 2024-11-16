#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanSpatialAnchorTypes.h"
#include "SerializationProperty.h"
#include "SerializableString.h"
#include "SerializableList.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSpatialAnchorRequests.rfkh.h"
#endif

// Spatial Anchor Request Types
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

// Spatial Anchor Response Types
struct STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorListResponse")) MikanSpatialAnchorListResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorListResponse";

	FIELD()
	Serialization::List<MikanSpatialAnchorID> spatial_anchor_id_list;

	MikanSpatialAnchorListResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSpatialAnchorListResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorInfoResponse")) MikanSpatialAnchorInfoResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorInfoResponse";

	FIELD()
	MikanSpatialAnchorInfo anchor_info;

	MikanSpatialAnchorInfoResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSpatialAnchorInfoResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSpatialAnchorRequests_GENERATED
#endif