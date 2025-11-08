#pragma once

#include "MikanAPIExport.h"
#include "SerializableString.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanTextureSourceTypes.rfkh.h"
#endif

// -- Structures -----

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTextureSourceTypes")) MikanClientTextureSourceInfo
{
	FIELD()
	Serialization::String client_source_name; ///< The name of the client video source

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanClientTextureSourceInfo_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTextureSourceTypes")) MikanSpoutTextureSourceInfo
{
	FIELD()
	Serialization::String spout_source_name; ///< The name of the spout video source

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSpoutTextureSourceInfo_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanTextureSourceTypes_GENERATED
#endif