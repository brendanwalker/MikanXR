#pragma once

#include "MikanStencilTypes.h"
#include "MikanAPITypes_json.h"
#include "MikanMathTypes_json.h"

#include "nlohmann/json.hpp"

// MikanStencilList
inline void to_json(nlohmann::json& j, const MikanStencilList& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));
	j.update({
		{"stencil_id_list", p.stencil_id_list}
	});
}
inline void from_json(const nlohmann::json& j, MikanStencilList& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
	j.at("stencil_id_list").get_to(p.stencil_id_list);
}

// MikanStencilQuad
inline void to_json(nlohmann::json& j, const MikanStencilQuadInfo& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));

	nlohmann::json transformJson;
	to_json(transformJson, p.relative_transform);

	j.update({
		{"stencil_id", p.stencil_id},
		{"parent_anchor_id", p.parent_anchor_id},
		{"relative_transform", transformJson},
		{"quad_width", p.quad_width},
		{"quad_height", p.quad_height},
		{"is_double_sided", p.is_double_sided},
		{"is_disabled", p.is_disabled},
		{"stencil_name", p.stencil_name},
		{"relative_transform", transformJson}
	});
}
inline void from_json(const nlohmann::json& j, MikanStencilQuadInfo& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
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
inline void to_json(nlohmann::json& j, const MikanStencilBoxInfo& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));

	nlohmann::json transformJson;
	to_json(transformJson, p.relative_transform);

	j.update({
		{"stencil_id", p.stencil_id},
		{"parent_anchor_id", p.parent_anchor_id},
		{"relative_transform", transformJson},
		{"box_x_size", p.box_x_size},
		{"box_y_size", p.box_y_size},
		{"box_z_size", p.box_z_size},
		{"is_disabled", p.is_disabled},
		{"stencil_name", p.stencil_name},
		{"relative_transform", transformJson}
	});
}
inline void from_json(const nlohmann::json& j, MikanStencilBoxInfo& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
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
inline void to_json(nlohmann::json& j, const MikanStencilModelInfo& p)
{
	nlohmann::to_json(j, static_cast<MikanResponse>(p));

	nlohmann::json transformJson;
	to_json(transformJson, p.relative_transform);

	j.update({
		{"stencil_id", p.stencil_id},
		{"parent_anchor_id", p.parent_anchor_id},
		{"relative_transform", transformJson},
		{"is_disabled", p.is_disabled},
		{"stencil_name", p.stencil_name},
		{"relative_transform", transformJson}
	});
}
inline void from_json(const nlohmann::json& j, MikanStencilModelInfo& p)
{
	from_json(j, static_cast<MikanResponse&>(p));
	from_json(j.at("relative_transform"), p.relative_transform);

	j.at("stencil_id").get_to(p.stencil_id);
	j.at("parent_anchor_id").get_to(p.parent_anchor_id);
	j.at("is_disabled").get_to(p.is_disabled);
	j.at("stencil_name").get_to(p.stencil_name);
}
