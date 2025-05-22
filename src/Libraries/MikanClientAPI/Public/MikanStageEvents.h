#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStageEvents.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageEvents")) MikanStageNameUpdateEvent : 
	public MikanEvent
{
	MikanStageNameUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanStageNameUpdateEvent)
	}

	FIELD()
	MikanStageID stage_id;
	FIELD()
	Serialization::String stage_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStageNameUpdateEvent_GENERATED
	#endif
};


struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageEvents")) MikanStageListUpdateEvent : 
	public MikanEvent
{
	MikanStageListUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanStageListUpdateEvent)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStageListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStageEvents_GENERATED
#endif