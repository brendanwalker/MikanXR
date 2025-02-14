#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanRemoteControlTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanRemoteControlRequests.rfkh.h"
#endif

// Remtoe Control Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRemoteControlRequest")) GotoAppStage :
	public MikanRequest
{
public:
	GotoAppStage()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GotoAppStage)
	}
	
	FIELD()
	Serialization::String app_state_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GotoAppStage_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRemoteControlRequest")) GetAppStageInfo :
	public MikanRequest
{
public:
	GetAppStageInfo()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetAppStageInfo)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetAppStageInfo_GENERATED
	#endif
};

// Remote Control Response Types
// ------

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanAppStageInfoResponse : 
	public MikanResponse
{
	MikanAppStageInfoResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanAppStageInfoResponse)
	}

	FIELD()
	MikanAppStageInfo app_stage_info; 

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAppStageInfoResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanRemoteControlRequests_GENERATED
#endif