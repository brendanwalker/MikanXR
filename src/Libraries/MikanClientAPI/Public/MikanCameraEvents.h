#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanCameraEvents.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraEvents")) MikanCameraNameUpdateEvent : 
	public MikanEvent
{
	MikanCameraNameUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanCameraNameUpdateEvent)
	}

	FIELD()
	MikanCameraID camera_id;
	FIELD()
	Serialization::String camera_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraNameUpdateEvent_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraEvents")) MikanCameraNewFrameEvent :
	public MikanEvent
{
	MikanCameraNewFrameEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanCameraNewFrameEvent)
	}

	FIELD()
	MikanVector3f cameraForward;
	FIELD()
	MikanVector3f cameraUp;
	FIELD()
	MikanVector3f cameraPosition;
	FIELD()
	int64_t frame;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraNewFrameEvent_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraEvents")) MikanCameraPoseUpdateEvent : 
	public MikanEvent
{
	MikanCameraPoseUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanCameraPoseUpdateEvent)
	}

	FIELD()
	MikanTransform transform;
	FIELD()
	MikanCameraID camera_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraPoseUpdateEvent_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSpatialCameraEvents")) MikanCameraListUpdateEvent : 
	public MikanEvent
{
	MikanCameraListUpdateEvent()
	{
		MIKAN_EVENT_TYPE_INFO_INIT(MikanCameraListUpdateEvent)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraEvents_GENERATED
#endif