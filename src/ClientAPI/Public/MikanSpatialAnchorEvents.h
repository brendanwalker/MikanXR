#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSpatialAnchorEvents.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorEvents")) MikanAnchorNameUpdateEvent : public MikanEvent
{
	FIELD()
	MikanSpatialAnchorID anchor_id;
	FIELD()
	Serialization::String anchor_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorNameUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorEvents")) MikanAnchorPoseUpdateEvent : public MikanEvent
{
	FIELD()
	MikanTransform transform;
	FIELD()
	MikanSpatialAnchorID anchor_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorPoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanSpatialAnchorEvents")) MikanAnchorListUpdateEvent : public MikanEvent
{
	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSpatialAnchorEvents_GENERATED
#endif