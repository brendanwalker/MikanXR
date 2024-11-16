#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVideoSourceEvents.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceEvents")) MikanVideoSourceOpenedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVideoSourceOpenedEvent";

	MikanVideoSourceOpenedEvent() : MikanEvent(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceOpenedEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceEvents")) MikanVideoSourceClosedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVideoSourceClosedEvent";

	MikanVideoSourceClosedEvent() : MikanEvent(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceClosedEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceEvents")) MikanVideoSourceNewFrameEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVideoSourceNewFrameEvent";

	FIELD()
		MikanVector3f cameraForward;
	FIELD()
		MikanVector3f cameraUp;
	FIELD()
		MikanVector3f cameraPosition;
	FIELD()
		uint64_t frame;

	MikanVideoSourceNewFrameEvent() : MikanEvent(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceNewFrameEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceEvents")) MikanVideoSourceAttachmentChangedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVideoSourceAttachmentChangedEvent";

	MikanVideoSourceAttachmentChangedEvent() : MikanEvent(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceAttachmentChangedEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceEvents")) MikanVideoSourceIntrinsicsChangedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVideoSourceIntrinsicsChangedEvent";

	MikanVideoSourceIntrinsicsChangedEvent() : MikanEvent(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceIntrinsicsChangedEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceEvents")) MikanVideoSourceModeChangedEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVideoSourceModeChangedEvent";

	MikanVideoSourceModeChangedEvent() : MikanEvent(k_typeName) {}

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceModeChangedEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceEvents_GENERATED
#endif