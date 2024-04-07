#pragma once

#include "MikanSpatialAnchorTypes.h"
#include "MikanAPITypes_json.h"
#include "MikanMathTypes_json.h"

#include "nlohmann/json.hpp"

// MikanSpatialAnchorList
inline void to_json(nlohmann::json& j, const MikanSpatialAnchorList& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));
	j.update({
		{"spatial_anchor_id_list", p.spatial_anchor_id_list}
	});
}
inline void from_json(const nlohmann::json& j, MikanSpatialAnchorList& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
	j.at("spatial_anchor_id_list").get_to(p.spatial_anchor_id_list);
}

// MikanSpatialAnchorInfo
inline void to_json(nlohmann::json& j, const MikanSpatialAnchorInfo& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));

	nlohmann::json transformJson;
	to_json(transformJson, p.world_transform);

	j.update({
		{"anchor_id", p.anchor_id},
		{"world_transform", transformJson},
		{"anchor_name", p.anchor_name}
	 });
}
inline void from_json(const nlohmann::json& j, MikanSpatialAnchorInfo& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
	from_json(j.at("world_transform"), p.world_transform);
	j.at("anchor_id").get_to(p.anchor_id);
	j.at("anchor_name").get_to(p.anchor_name);
}