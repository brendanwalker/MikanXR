#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanRemoteControlEvents.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanRemoteControlEvents")) MikanAppStageChagedEvent : public MikanEvent
{
	MikanAppStageChagedEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanAppStageChagedEvent)
	}

	FIELD()
	Serialization::String new_app_state_name;
	
	FIELD()
	Serialization::String old_app_state_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAppStageChagedEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanRemoteControlEvents_GENERATED
#endif