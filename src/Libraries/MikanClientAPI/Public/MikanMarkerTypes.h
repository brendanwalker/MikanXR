#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanComponentTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanMarkerTypes.rfkh.h"
#endif

#include <assert.h>

// Structures
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanMarkerTypes")) MikanMarkerComponentValues :
	public MikanComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	int aruco_id;
	FIELD()
	float length_mm;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanMarkerComponentValues_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanMarkerTypes_GENERATED
#endif
