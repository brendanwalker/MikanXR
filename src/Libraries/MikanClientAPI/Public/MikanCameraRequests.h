#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanCameraTypes.h"
#include "SerializationProperty.h"
#include "SerializableString.h"
#include "SerializableList.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanCameraRequests.rfkh.h"
#endif

// Spatial Anchor Request Types
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) GetCameraList :
	public MikanRequest
{
public:
	GetCameraList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetCameraList)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetCameraList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) GetCameraInfo :
	public MikanRequest
{
public:
	GetCameraInfo()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetCameraInfo)
	}

	FIELD()
	MikanCameraID camera_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetCameraInfo_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) GetCameraIntrinsics :
	public MikanRequest
{
public:
	GetCameraIntrinsics()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetCameraIntrinsics)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetCameraIntrinsics_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) GetCameraAttachment :
	public MikanRequest
{
public:
	GetCameraAttachment()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetCameraAttachment)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetCameraAttachment_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) FindCameraByName :
	public MikanRequest
{
public:
	FindCameraByName()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(FindCameraByName)
	}

	FIELD()
	Serialization::String camera_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	FindCameraByName_GENERATED
	#endif
};

// Camera Response Types
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) MikanCameraListResponse : 
	public MikanResponse
{
	MikanCameraListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanCameraListResponse)
	}

	FIELD()
	Serialization::List<MikanCameraID> camera_id_list;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraListResponse_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) MikanCameraInfoResponse : 
	public MikanResponse
{
	MikanCameraInfoResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanCameraInfoResponse)
	}

	FIELD()
	MikanCameraInfo camera_info;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraInfoResponse_GENERATED
	#endif
};

/// Bundle containing all intrinsic video source properties
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) MikanCameraIntrinsicsResponse :
	public MikanResponse
{
	MikanCameraIntrinsicsResponse()
		: intrinsics()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanCameraIntrinsicsResponse)
	}

	FIELD()
	MikanCameraIntrinsics intrinsics;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraIntrinsicsResponse_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

/// Static properties about video source tracker attachment
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) MikanCameraAttachmentInfoResponse :
	public MikanResponse
{
	MikanCameraAttachmentInfoResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanCameraAttachmentInfoResponse)
	}

	FIELD()
	MikanVRDeviceID attached_vr_device_id;
	FIELD()
	MikanMatrix4f vr_device_offset_xform;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraAttachmentInfoResponse_GENERATED
	#endif
};


#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraRequests_GENERATED
#endif