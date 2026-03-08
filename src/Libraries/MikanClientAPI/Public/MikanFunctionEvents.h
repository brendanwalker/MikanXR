#pragma once

#include "MikanCoreTypes.h"
#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanFunctionTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanFunctionEvents.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanFunctionEvents")) MikanFunctionsUpdatedEvent :
	public MikanEvent
{
	MikanFunctionsUpdatedEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanFunctionsUpdatedEvent)
	}

	FIELD()
	Serialization::String ownerSystem;
	FIELD()
	int componentId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanFunctionsUpdatedEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanFunctionEvents_GENERATED
#endif
