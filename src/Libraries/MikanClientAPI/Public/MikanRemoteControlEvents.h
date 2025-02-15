#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializableList.h"
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

struct STRUCT(Serialization::CodeGenModule("MikanRemoteControlEvents")) MikanRemoteControlEvent : public MikanEvent
{
	MikanRemoteControlEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanRemoteControlEvent)
	}

	FIELD()
	Serialization::String remoteControlEvent;

	FIELD()
	Serialization::List<Serialization::String> parameters;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRemoteControlEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanRemoteControlEvents_GENERATED
#endif