#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVRDeviceEvents.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanVRDeviceEvents")) MikanVRDevicePoseUpdateEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVRDevicePoseUpdateEvent";

	FIELD()
	MikanMatrix4f transform;
	FIELD()
	MikanVRDeviceID device_id;
	FIELD()
	uint64_t frame;

	MikanVRDevicePoseUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDevicePoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanVRDeviceEvents")) MikanVRDeviceListUpdateEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanVRDeviceListUpdateEvent";

	MikanVRDeviceListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDeviceListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVRDeviceEvents_GENERATED
#endif