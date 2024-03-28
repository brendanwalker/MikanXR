#pragma once

#include "MikanAPITypes.h"
#include "MikanMathTypes.h"

#include <vector>

struct MikanStencilQuad : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilQuad";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	float quad_width;
	float quad_height;
	bool is_double_sided;
	bool is_disabled;
	std::string stencil_name;

	MikanStencilQuad() : MikanResponse(k_typeName) {}
};

struct MikanStencilBox : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilBox";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	float box_x_size;
	float box_y_size;
	float box_z_size;
	bool is_disabled;
	std::string stencil_name;

	MikanStencilBox() : MikanResponse(k_typeName) {}
};

struct MikanStencilModel : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModel";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	bool is_disabled;
	std::string stencil_name;

	MikanStencilModel() : MikanResponse(k_typeName) {}
};

struct MikanStencilList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilList";

	std::vector<MikanStencilID> stencil_id_list;

	MikanStencilList() : MikanResponse(k_typeName) {}
};
