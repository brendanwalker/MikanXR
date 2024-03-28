#pragma once

#include "MikanAPITypes.h"


// Shared Constants
//-----------------

/// The list of possible vr device drivers used by MikanXR Client API
enum MikanVRDeviceApi
{
	MikanVRDeviceApi_INVALID,
	MikanVRDeviceApi_STEAM_VR,
};

/// The list of possible vr device types used by MikanXR Client API
enum MikanVRDeviceType
{
	MikanVRDeviceType_INVALID,
	MikanVRDeviceType_HMD,
	MikanVRDeviceType_CONTROLLER,
	MikanVRDeviceType_TRACKER
};


// VR Device Response Types
struct MikanVRDeviceList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceList";

	std::vector<MikanVRDeviceID> vr_device_id_list;

	MikanVRDeviceList() : MikanResponse(k_typeName) {}
};

struct MikanVRDeviceInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceInfo";

	MikanVRDeviceApi vr_device_api;
	MikanVRDeviceType vr_device_type;
	std::string device_path;

	MikanVRDeviceInfo() : MikanResponse(k_typeName) {}
};
