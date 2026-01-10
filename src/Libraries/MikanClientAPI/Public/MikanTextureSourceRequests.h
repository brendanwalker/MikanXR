#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanTextureSourceTypes.h"
#include "SerializationProperty.h"
#include "SerializableList.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanTextureSourceRequests.rfkh.h"
#endif

// Texture Source Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTextureSourceRequest")) GetTextureSourceList :
	public MikanRequest
{
public:
	GetTextureSourceList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetTextureSourceList)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetTextureSourceList_GENERATED
	#endif
};

// Texture Source Response Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTextureSourceRequest")) MikanTextureSourceListResponse :
	public MikanResponse
{
	MikanTextureSourceListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanTextureSourceListResponse)
	}

	FIELD()
	Serialization::List<MikanTextureSourceID> video_source_id_list;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanTextureSourceListResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanTextureSourceRequests_GENERATED
#endif