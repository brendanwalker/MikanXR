#pragma once

#include "MikanAPITypes.h"
#include "SerializableList.h"

#ifdef REFLECTION_CODE_BUILT
#include "MikanVRDeviceTypes.rfkh.h"
#endif

// Shared Constants
//-----------------

/// The list of possible vr device drivers used by MikanXR Client API
enum ENUM() MikanVRDeviceApi
{
	MikanVRDeviceApi_INVALID,
	MikanVRDeviceApi_STEAM_VR,
};

/// The list of possible vr device types used by MikanXR Client API
enum ENUM() MikanVRDeviceType
{
	MikanVRDeviceType_INVALID,
	MikanVRDeviceType_HMD,
	MikanVRDeviceType_CONTROLLER,
	MikanVRDeviceType_TRACKER
};


// VR Device Response Types
struct STRUCT() MikanVRDeviceList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceList";

	FIELD()
	Serialization::List<MikanVRDeviceID> vr_device_id_list;

	MikanVRDeviceList() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
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
	std::string device_path;

	MikanVRDeviceInfo() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanVRDeviceInfo_GENERATED
	#endif
};

#ifdef REFLECTION_CODE_BUILT
File_MikanVRDeviceTypes_GENERATED
#endif