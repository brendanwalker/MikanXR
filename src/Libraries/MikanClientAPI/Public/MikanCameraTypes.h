#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanCameraTypes.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraTypes")) MikanCameraInfo
{
	FIELD()
	MikanCameraID camera_id;
	FIELD()
	MikanTransform world_transform; // Transform in tracking system space
	FIELD()
	Serialization::String camera_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraTypes_GENERATED
#endif