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

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraRequest")) FindCameraInfoByName :
	public MikanRequest
{
public:
	FindCameraInfoByName()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(FindCameraInfoByName)
	}

	FIELD()
	Serialization::String camera_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	FindCameraInfoByName_GENERATED
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

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraRequests_GENERATED
#endif