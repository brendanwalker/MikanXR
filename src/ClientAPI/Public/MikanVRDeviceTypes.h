#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "SerializableList.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVRDeviceTypes.rfkh.h"
#endif

// Shared Constants
//-----------------

/// The list of possible vr device drivers used by MikanXR Client API
enum ENUM() MikanVRDeviceApi
{
	MikanVRDeviceApi_INVALID ENUMVALUE_STRING("INVALID"),
	MikanVRDeviceApi_STEAM_VR ENUMVALUE_STRING("STEAM_VR"),
};

/// The list of possible vr device types used by MikanXR Client API
enum ENUM() MikanVRDeviceType
{
	MikanVRDeviceType_INVALID ENUMVALUE_STRING("INVALID"),
	MikanVRDeviceType_HMD ENUMVALUE_STRING("HMD"),
	MikanVRDeviceType_CONTROLLER ENUMVALUE_STRING("CONTROLLER"),
	MikanVRDeviceType_TRACKER ENUMVALUE_STRING("TRACKER")
};


// VR Device Response Types
struct STRUCT() MikanVRDeviceList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceList";

	FIELD()
	Serialization::List<MikanVRDeviceID> vr_device_id_list;

	MikanVRDeviceList() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDeviceList_GENERATED
	#endif
};

struct STRUCT() MikanVRDeviceInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceInfo";

	FIELD()
	MikanVRDeviceApi vr_device_api;
	FIELD()
	MikanVRDeviceType vr_device_type;
	FIELD()
	Serialization::String device_path;

	MikanVRDeviceInfo() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDeviceInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVRDeviceTypes_GENERATED
#endif