#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializableObjectPtr.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanCameraTypes.rfkh.h"
#endif

#include <assert.h>

// Structures
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraTypes")) MikanCameraInfo
{
	FIELD()
	Serialization::String camera_name;
	FIELD()
	MikanCameraID camera_id;
	FIELD()
	MikanStageID stage_id;
	FIELD()
	MikanTrackingMountID tracking_mount_id;
	FIELD()
	MikanVideoSourceID video_source_id;
	FIELD()
	int tracking_frame_delay;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraTypes_GENERATED
#endif