#pragma once

#include "MikanVRDeviceTypes.h"

#include "nlohmann/json.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM(MikanVRDeviceApi, {
	{MikanVRDeviceApi_INVALID, nullptr},
	{MikanVRDeviceApi_STEAM_VR, "STEAM_VR"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(MikanVRDeviceType, {
	{MikanVRDeviceType_INVALID, nullptr},
	{MikanVRDeviceType_HMD, "HMD"},
	{MikanVRDeviceType_CONTROLLER, "CONTROLLER"},
	{MikanVRDeviceType_TRACKER, "TRACKER"},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVRDeviceList,
								   vr_device_id_list
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVRDeviceInfo,
								   vr_device_api,
								   vr_device_type,
								   device_path
)