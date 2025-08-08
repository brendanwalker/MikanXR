#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanVideoSourceTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVideoSourceRequests.rfkh.h"
#endif

// Video Source Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceList :
	public MikanRequest
{
public:
	GetVideoSourceList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetVideoSourceList)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceMode :
	public MikanRequest
{
public:
	GetVideoSourceMode()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetVideoSourceMode)
	}

	FIELD()
	MikanVideoSourceID video_source_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceMode_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceIntrinsics :
	public MikanRequest
{
public:
	GetVideoSourceIntrinsics()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetVideoSourceIntrinsics)
	}

	FIELD()
	MikanVideoSourceID video_source_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceIntrinsics_GENERATED
	#endif
};

// Video Source Response Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) MikanVideoSourceListResponse :
	public MikanResponse
{
	MikanVideoSourceListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanVideoSourceListResponse)
	}

	FIELD()
	Serialization::List<MikanVideoSourceID> video_source_id_list;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceListResponse_GENERATED
	#endif
};

/// Static properties about a video source
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) MikanVideoSourceModeResponse : 
	public MikanResponse
{
	MikanVideoSourceModeResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanVideoSourceModeResponse)
	}

	FIELD()
	MikanVideoSourceType video_source_type;
	FIELD()
	Serialization::String video_source_api;
	FIELD()
	Serialization::String device_path;
	FIELD()
	Serialization::String video_mode_name;
	FIELD()
	int32_t resolution_x;
	FIELD()
	int32_t resolution_y;
	FIELD()
	float frame_rate;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceModeResponse_GENERATED
	#endif
};

/// Bundle containing all intrinsic video source properties
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) MikanVideoSourceIntrinsicsResponse :
	public MikanResponse
{
	MikanVideoSourceIntrinsicsResponse()
		: intrinsics()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanVideoSourceIntrinsicsResponse)
	}

	FIELD()
	MikanVideoSourceIntrinsics intrinsics;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceIntrinsicsResponse_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};


#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceRequests_GENERATED
#endif