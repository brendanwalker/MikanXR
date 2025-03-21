#pragma once

#include "MikanAPIExport.h"
#include "SerializationProperty.h"
#include "SerializableString.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanScriptTypes.rfkh.h"
#endif

// Script Event Types
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptTypes")) MikanScriptMessageInfo 
{
	FIELD()
	Serialization::String content;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanScriptMessageInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanScriptTypes_GENERATED
#endif