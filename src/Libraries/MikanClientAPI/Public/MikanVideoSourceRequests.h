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

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) GetVideoSourceMode :
	public MikanRequest
{
public:
	GetVideoSourceMode()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetVideoSourceMode)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVideoSourceMode_GENERATED
	#endif
};

// Video Source Response Types
// ------

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
	MikanVideoSourceApi video_source_api;
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

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceRequests_GENERATED
#endif