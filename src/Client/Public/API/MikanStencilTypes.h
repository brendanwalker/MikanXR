#pragma once

#include "MikanAPITypes.h"
#include "MikanMathTypes.h"

#include <vector>

struct MikanStencilQuadInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilQuadInfo";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	float quad_width;
	float quad_height;
	bool is_double_sided;
	bool is_disabled;
	std::string stencil_name;

	MikanStencilQuadInfo() : MikanResponse(k_typeName) {}
};

struct MikanStencilBoxInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilBoxInfo";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	float box_x_size;
	float box_y_size;
	float box_z_size;
	bool is_disabled;
	std::string stencil_name;

	MikanStencilBoxInfo() : MikanResponse(k_typeName) {}
};

struct MikanStencilModelInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModelInfo";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	bool is_disabled;
	std::string stencil_name;

	MikanStencilModelInfo() : MikanResponse(k_typeName) {}
};

struct MikanStencilList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilList";

	std::vector<MikanStencilID> stencil_id_list;

	MikanStencilList() : MikanResponse(k_typeName) {}
};

struct MikanTriagulatedMesh
{
	std::vector<MikanVector3f> vertices;
	std::vector<MikanVector3f> normals;
	std::vector<MikanVector2f> texels;
	std::vector<int> indices;
};

struct MikanStencilModelRenderGeometry : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModelRenderGeometry";

	std::vector<MikanTriagulatedMesh> meshes;

	MikanStencilModelRenderGeometry() : MikanResponse(k_typeName) {}
};