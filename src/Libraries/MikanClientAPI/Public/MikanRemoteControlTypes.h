#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanRemoteControlTypes.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanRemoteControlTypes")) MikanAppStageInfo
{
	FIELD()
	Serialization::String app_state_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAppStageInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanRemoteControlTypes_GENERATED
#endif