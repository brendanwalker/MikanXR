#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStageTypes.rfkh.h"
#endif

enum class ENUM(Serialization::CodeGenModule("MikanStageTypes")) MikanStageTrackingSystem
{
	StaticMarker ENUMVALUE_STRING("StaticMarker") = 0,
	SteamVR ENUMVALUE_STRING("SteamVR") = 1
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStageTypes")) MikanStageInfo
{
	FIELD()
	MikanStageID stage_id;
	FIELD()
	Serialization::String stage_name;
	FIELD()
	MikanStageTrackingSystem tracking_system;
	FIELD()
	MikanMarkerID origin_marker_id;
	FIELD()
	float origin_marker_size;
	FIELD()
	MikanMarkerID utility_marker_id;
	FIELD()
	float utility_marker_size;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStageInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStageTypes_GENERATED
#endif