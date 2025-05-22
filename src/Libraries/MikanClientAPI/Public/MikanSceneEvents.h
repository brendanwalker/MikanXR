#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSceneEvents.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneEvents")) MikanSceneNameUpdateEvent : 
	public MikanEvent
{
	MikanSceneNameUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanSceneNameUpdateEvent)
	}

	FIELD()
	MikanSceneID stencil_id;
	FIELD()
	Serialization::String stencil_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSceneNameUpdateEvent_GENERATED
	#endif
};


struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneEvents")) MikanSceneListUpdateEvent : 
	public MikanEvent
{
	MikanSceneListUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanSceneListUpdateEvent)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSceneListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSceneEvents_GENERATED
#endif