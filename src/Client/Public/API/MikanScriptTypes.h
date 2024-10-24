#pragma once

#include "MikanExport.h"
#include "MikanEventTypes.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanScriptTypes.rfkh.h"
#endif

// Script Event Types
struct STRUCT() MikanScriptMessageInfo 
{
	Serialization::String content;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanScriptMessageInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanScriptTypes_GENERATED
#endif