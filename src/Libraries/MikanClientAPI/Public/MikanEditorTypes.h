#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanPropertyTypes.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanEditorTypes.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanEditorTypes")) MikanEditorSystemValues :
	public MikanSystemValues
{
	static const char* k_systemName;

	FIELD()
	float cameraSpeed= 0.f;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanEditorSystemValues_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanEditorTypes_GENERATED
#endif