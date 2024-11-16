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

// Video Source Response Types
// ------

/// Bundle containing all intrinsic video source properties
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) MikanVideoSourceIntrinsicsResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceIntrinsicsResponse";

	FIELD()
	MikanVideoSourceIntrinsics intrinsics;

	MikanVideoSourceIntrinsicsResponse()
		: MikanResponse(k_typeName)
		, intrinsics()
	{}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceIntrinsicsResponse_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

/// Static properties about video source tracker attachment
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) MikanVideoSourceAttachmentInfoResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceAttachmentInfoResponse";

	FIELD()
	MikanVRDeviceID attached_vr_device_id;
	FIELD()
	MikanMatrix4f vr_device_offset_xform;

	MikanVideoSourceAttachmentInfoResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceAttachmentInfoResponse_GENERATED
	#endif
};

/// Static properties about a video source
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceRequest")) MikanVideoSourceModeResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceModeResponse";

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

	MikanVideoSourceModeResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceModeResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceRequests_GENERATED
#endif