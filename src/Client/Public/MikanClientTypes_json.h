#pragma once

#include "MikanClientTypes.h"

#include "MikanCoreTypes_json.h"
#include "MikanMathTypes_json.h"

NLOHMANN_JSON_SERIALIZE_ENUM(MikanVideoSourceType, {
	{MikanVideoSourceType_MONO, "MONO"},
	{MikanVideoSourceType_STEREO, "STEREO"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(MikanVideoSourceApi, {
	{MikanVideoSourceApi_INVALID, nullptr},
	{MikanVideoSourceApi_OPENCV_CV, "OPEN_CV"},
	{MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION, "WMF"},
})

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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanClientInfo, 
	clientId,
	engineName,
	engineVersion,
	applicationName,
	applicationVersion,
	xrDeviceName,
	graphicsAPI,
	mikanCoreSdkVersion,
	supportsRBG24,
	supportsRBGA32,
	supportsBGRA32,
	supportsDepth)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanColorRGB, 
	r,
	g,
	b)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanResponse,
	responseType,
	requestId,
	resultCode
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanStencilList,
	stencil_id_list
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVRDeviceList,
	vr_device_id_list
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVRDeviceInfo,
	vr_device_api,
	vr_device_type,
	device_path
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanSpatialAnchorList,
	spatial_anchor_id_list
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanScriptMessageInfo, 
	content
)

// MikanStencilQuad
inline void to_json(nlohmann::json& j, const MikanStencilQuad& p)
{
	nlohmann::json transformJson;
	to_json(transformJson, p.relative_transform);

	j = nlohmann::json{
		{"stencil_id", p.stencil_id},
		{"parent_anchor_id", p.parent_anchor_id},
		{"relative_transform", transformJson},
		{"quad_width", p.quad_width},
		{"quad_height", p.quad_height},
		{"is_double_sided", p.is_double_sided},
		{"is_disabled", p.is_disabled},
		{"stencil_name", p.stencil_name},
		{"relative_transform", transformJson}
	};
}
inline void from_json(const nlohmann::json& j, MikanStencilQuad& p)
{
	from_json(j.at("relative_transform"), p.relative_transform);

	j.at("stencil_id").get_to(p.stencil_id);
	j.at("parent_anchor_id").get_to(p.parent_anchor_id);
	j.at("quad_width").get_to(p.quad_width);
	j.at("quad_height").get_to(p.quad_height);
	j.at("is_double_sided").get_to(p.is_double_sided);
	j.at("is_disabled").get_to(p.is_disabled);
	j.at("stencil_name").get_to(p.stencil_name);
}

// MikanStencilBox
inline void to_json(nlohmann::json& j, const MikanStencilBox& p)
{
	nlohmann::json transformJson;
	to_json(transformJson, p.relative_transform);

	j = nlohmann::json{
		{"stencil_id", p.stencil_id},
		{"parent_anchor_id", p.parent_anchor_id},
		{"relative_transform", transformJson},
		{"box_x_size", p.box_x_size},
		{"box_y_size", p.box_y_size},
		{"box_z_size", p.box_z_size},
		{"is_disabled", p.is_disabled},
		{"stencil_name", p.stencil_name},
		{"relative_transform", transformJson}
	};
}
inline void from_json(const nlohmann::json& j, MikanStencilBox& p)
{
	from_json(j.at("relative_transform"), p.relative_transform);

	j.at("stencil_id").get_to(p.stencil_id);
	j.at("parent_anchor_id").get_to(p.parent_anchor_id);
	j.at("box_x_size").get_to(p.box_x_size);
	j.at("box_y_size").get_to(p.box_y_size);
	j.at("box_z_size").get_to(p.box_z_size);
	j.at("is_disabled").get_to(p.is_disabled);
	j.at("stencil_name").get_to(p.stencil_name);
}

// MikanStencilModel
inline void to_json(nlohmann::json& j, const MikanStencilModel& p)
{
	nlohmann::json transformJson;
	to_json(transformJson, p.relative_transform);

	j = nlohmann::json{
		{"stencil_id", p.stencil_id},
		{"parent_anchor_id", p.parent_anchor_id},
		{"relative_transform", transformJson},
		{"is_disabled", p.is_disabled},
		{"stencil_name", p.stencil_name},
		{"relative_transform", transformJson}
	};
}
inline void from_json(const nlohmann::json& j, MikanStencilModel& p)
{
	from_json(j.at("relative_transform"), p.relative_transform);

	j.at("stencil_id").get_to(p.stencil_id);
	j.at("parent_anchor_id").get_to(p.parent_anchor_id);
	j.at("is_disabled").get_to(p.is_disabled);
	j.at("stencil_name").get_to(p.stencil_name);
}

// MikanSpatialAnchorInfo
inline void to_json(nlohmann::json& j, const MikanSpatialAnchorInfo& p)
{
	nlohmann::json transformJson;
	to_json(transformJson, p.world_transform);

	j = nlohmann::json{
		{"anchor_id", p.anchor_id},
		{"world_transform", transformJson},
		{"anchor_name", p.anchor_name}
	};
}
inline void from_json(const nlohmann::json& j, MikanSpatialAnchorInfo& p)
{
	from_json(j.at("world_transform"), p.world_transform);

	j.at("anchor_id").get_to(p.anchor_id);
	j.at("anchor_name").get_to(p.anchor_name);
}