#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanCameraEvents.rfkh.h"
#endif

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


#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraEvents_GENERATED
#endif