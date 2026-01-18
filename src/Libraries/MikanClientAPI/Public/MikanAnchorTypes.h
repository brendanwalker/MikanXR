#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanTransformTypes.h"
#include "SerializableList.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanAnchorTypes.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanAnchorTypes")) MikanAnchorComponentValues :
	public MikanTransformComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	MikanStageID stage_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorComponentValues_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanAnchorTypes_GENERATED
#endif