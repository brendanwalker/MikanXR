#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanStageTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStageRequests.rfkh.h"
#endif

// Stage Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageRequest")) GetStageList :
	public MikanRequest
{
public:
	GetStageList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetStageList)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetStageList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageRequest")) GetStage :
	public MikanRequest
{
public:
	GetStage()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetStage)
	}

	FIELD()
	MikanStageID stage_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetStage_GENERATED
	#endif
};


// Stage Response Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageRequest")) MikanStageInfoResponse : 
	public MikanResponse
{
	MikanStageInfoResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanStageInfoResponse)
	}

	FIELD()
	MikanStageInfo stage_info; 

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStageInfoResponse_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageRequest")) MikanStageListResponse :
	public MikanResponse
{
	MikanStageListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanStageListResponse)
	}

	FIELD()
	Serialization::List<MikanStageID> stage_id_list;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStageListResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStageRequests_GENERATED
#endif