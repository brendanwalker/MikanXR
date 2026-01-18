#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanPropertyTypes.h"
#include "MikanTransformTypes.h"
#include "SerializableList.h"
#include "SerializableObjectPtr.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanCameraTypes.rfkh.h"
#endif

#include <assert.h>

// Structures
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanCameraTypes")) MikanCameraComponentValues : 
	public MikanTransformComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	MikanStageID stage_id;
	FIELD()
	MikanTrackingMountID tracking_mount_id;
	FIELD()
	MikanVideoSourceID video_source_id;
	FIELD()
	int tracking_frame_delay;
	FIELD()
	MikanQuatd aperture_orientation_offset;
	FIELD()
	MikanVector3d aperture_position_offset;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraComponentValues_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanCameraTypes_GENERATED
#endif