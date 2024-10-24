#pragma once

#include "MikanAPITypes.h"
#include "MikanMathTypes.h"

#include "SerializableList.h"

#ifdef REFLECTION_CODE_BUILT
#include "MikanStencilTypes.rfkh.h"
#endif

struct STRUCT() MikanStencilQuadInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilQuadInfo";

	FIELD()
	MikanStencilID stencil_id; // filled in on allocation
	FIELD()
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	FIELD()
	MikanTransform relative_transform; // transform relative to parent anchor
	FIELD()
	float quad_width;
	FIELD()
	float quad_height;
	FIELD()
	bool is_double_sided;
	FIELD()
	bool is_disabled;
	FIELD()
	Serialization::String stencil_name;

	MikanStencilQuadInfo() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanStencilQuadInfo_GENERATED
	#endif
};

struct STRUCT() MikanStencilBoxInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilBoxInfo";

	FIELD()
	MikanStencilID stencil_id; // filled in on allocation
	FIELD()
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	FIELD()
	MikanTransform relative_transform; // transform relative to parent anchor
	FIELD()
	float box_x_size;
	FIELD()
	float box_y_size;
	FIELD()
	float box_z_size;
	FIELD()
	bool is_disabled;
	FIELD()
	Serialization::String stencil_name;

	MikanStencilBoxInfo() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanStencilBoxInfo_GENERATED
	#endif
};

struct STRUCT() MikanStencilModelInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModelInfo";

	FIELD()
	MikanStencilID stencil_id; // filled in on allocation
	FIELD()
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	FIELD()
	MikanTransform relative_transform; // transform relative to parent anchor
	FIELD()
	bool is_disabled;
	FIELD()
	Serialization::String stencil_name;

	MikanStencilModelInfo() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanStencilModelInfo_GENERATED
	#endif
};

struct STRUCT() MikanStencilList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilList";

	FIELD()
	Serialization::List<MikanStencilID> stencil_id_list;

	MikanStencilList() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanStencilList_GENERATED
	#endif
};

struct STRUCT() MikanTriagulatedMesh
{
	FIELD()
	Serialization::List<MikanVector3f> vertices;
	FIELD()
	Serialization::List<MikanVector3f> normals;
	FIELD()
	Serialization::List<MikanVector2f> texels;
	FIELD()
	Serialization::List<int> indices;

	#ifdef REFLECTION_CODE_BUILT
	MikanTriagulatedMesh_GENERATED
	#endif
};

struct STRUCT() MikanStencilModelRenderGeometry : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModelRenderGeometry";

	FIELD()
	Serialization::List<MikanTriagulatedMesh> meshes;

	MikanStencilModelRenderGeometry() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanStencilModelRenderGeometry_GENERATED
	#endif
};

#ifdef REFLECTION_CODE_BUILT
File_MikanStencilTypes_GENERATED
#endif