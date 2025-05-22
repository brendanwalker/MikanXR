#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSceneTypes.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneTypes")) MikanSceneInfo
{
	FIELD()
	MikanSceneID scene_id;
	FIELD()
	Serialization::String scene_name;
	FIELD()
	MikanStageID parent_stage_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSceneInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSceneTypes_GENERATED
#endif