#pragma once

#include "MikanVRDeviceTypes.h"
#include "MikanAPITypes_json.h"

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

// MikanVRDeviceList
inline void to_json(nlohmann::json& j, const MikanVRDeviceList& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));
	j.update({
		{"vr_device_id_list", p.vr_device_id_list}
	});
}
inline void from_json(const nlohmann::json& j, MikanVRDeviceList& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
	j.at("vr_device_id_list").get_to(p.vr_device_id_list);
}

// MikanVRDeviceInfo
inline void to_json(nlohmann::json& j, const MikanVRDeviceInfo& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));
	j.update({
		{"vr_device_api", p.vr_device_api},
		{"vr_device_type", p.vr_device_type},
		{"device_path", p.device_path.getValue()}
	});
}
inline void from_json(const nlohmann::json& j, MikanVRDeviceInfo& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
	j.at("vr_device_api").get_to(p.vr_device_api);
	j.at("vr_device_type").get_to(p.vr_device_type);
	from_json(j.at("device_path"), p.device_path);
}