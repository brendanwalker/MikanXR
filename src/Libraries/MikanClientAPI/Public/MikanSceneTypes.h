#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanTransformTypes.h"
#include "SerializableList.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSceneTypes.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanSceneTypes")) MikanSceneComponentValues : 
	public MikanTransformComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	MikanStageID parent_stage_id;
	FIELD()
	Serialization::List<MikanCompositorID> compositor_list;
	FIELD()
	MikanCompositorID display_compositor_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSceneComponentValues_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSceneTypes_GENERATED
#endif