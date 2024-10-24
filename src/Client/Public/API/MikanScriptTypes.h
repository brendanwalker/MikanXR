#pragma once

#include "MikanEventTypes.h"

#ifdef REFLECTION_CODE_BUILT
#include "MikanScriptTypes.rfkh.h"
#endif

// Script Event Types
struct STRUCT() MikanScriptMessageInfo 
{
	Serialization::String content;

	#ifdef REFLECTION_CODE_BUILT
	MikanScriptMessageInfo_GENERATED
	#endif
};

#ifdef REFLECTION_CODE_BUILT
File_MikanScriptTypes_GENERATED
#endif