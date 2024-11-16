#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanClientEvents.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanClientEvents")) MikanConnectedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanConnectedEvent";

	MikanConnectedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanConnectedEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanClientEvents")) MikanDisconnectedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanDisconnectedEvent";

	MikanDisconnectedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDisconnectedEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanClientEvents_GENERATED
#endif