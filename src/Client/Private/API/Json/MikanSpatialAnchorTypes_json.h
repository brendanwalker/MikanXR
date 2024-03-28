#pragma once

#include "MikanSpatialAnchorTypes.h"
#include "MikanMathTypes_json.h"

#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanSpatialAnchorList,
								   spatial_anchor_id_list
)

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