#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanSceneTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSceneRequests.rfkh.h"
#endif

// Stencil Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneRequest")) GetSceneList :
	public MikanRequest
{
public:
	GetSceneList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetSceneList)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetSceneList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneRequest")) GetScene :
	public MikanRequest
{
public:
	GetScene()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetScene)
	}

	FIELD()
	MikanSceneID scene_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetScene_GENERATED
	#endif
};


// Scene Response Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneRequest")) MikanSceneInfoResponse : 
	public MikanResponse
{
	MikanSceneInfoResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanSceneInfoResponse)
	}

	FIELD()
	MikanSceneInfo scene_info; 

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSceneInfoResponse_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneRequest")) MikanSceneListResponse :
	public MikanResponse
{
	MikanSceneListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanSceneListResponse)
	}

	FIELD()
	Serialization::List<MikanSceneID> scene_id_list;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSceneListResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSceneRequests_GENERATED
#endif