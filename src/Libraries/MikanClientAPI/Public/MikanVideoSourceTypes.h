#pragma once

#include "MikanAPIExport.h"
#include "MikanMathTypes.h"
#include "MikanVRDeviceTypes.h"
#include "SerializableObjectPtr.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVideoSourceTypes.rfkh.h"
#endif

// Constants
enum ENUM(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceType
{
	MikanVideoSourceType_MONO ENUMVALUE_STRING("MONO"),
	MikanVideoSourceType_STEREO ENUMVALUE_STRING("STEREO")
};

/// The list of possible video source APIs used by MikanXR
enum ENUM(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceApi
{
	MikanVideoSourceApi_INVALID ENUMVALUE_STRING("INVALID"),
	MikanVideoSourceApi_OPEN_CV ENUMVALUE_STRING("OPEN_CV"),
	MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION ENUMVALUE_STRING("WINDOWS_MEDIA_FOUNDATION"),
	MikanVideoSourceApi_GSTREAMER ENUMVALUE_STRING("GSTREAMER"),
	MikanVideoSourceApi_SPOUT2 ENUMVALUE_STRING("SPOUT2"),
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceTypes_GENERATED
#endif