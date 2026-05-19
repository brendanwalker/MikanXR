#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanLightTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanLightEvents.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightEvents")) MikanRGBSpotLightCreatedEvent :
	public MikanEvent
{
	MikanRGBSpotLightCreatedEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanRGBSpotLightCreatedEvent)
	}

	FIELD()
	MikanLightID light_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBSpotLightCreatedEvent_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightEvents")) MikanRGBSpotLightDeletedEvent :
	public MikanEvent
{
	MikanRGBSpotLightDeletedEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanRGBSpotLightDeletedEvent)
	}

	FIELD()
	MikanLightID light_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBSpotLightDeletedEvent_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightEvents")) MikanRGBPixelGridCreatedEvent :
	public MikanEvent
{
	MikanRGBPixelGridCreatedEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanRGBPixelGridCreatedEvent)
	}

	FIELD()
	MikanLightID grid_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBPixelGridCreatedEvent_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightEvents")) MikanRGBPixelGridDeletedEvent :
	public MikanEvent
{
	MikanRGBPixelGridDeletedEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanRGBPixelGridDeletedEvent)
	}

	FIELD()
	MikanLightID grid_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBPixelGridDeletedEvent_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanLightEvents_GENERATED
#endif
