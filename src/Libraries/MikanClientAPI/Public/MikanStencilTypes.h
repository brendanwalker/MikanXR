#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanPropertyTypes.h"
#include "MikanTransformTypes.h"
#include "SerializableList.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStencilTypes.rfkh.h"
#endif

enum ENUM(Serialization::CodeGenModule("MikanStencilTypes")) MikanStencilCullMode
{
	MikanStencilCullMode_NONE ENUMVALUE_STRING("NONE"),
	MikanStencilCullMode_Z_AXIS ENUMVALUE_STRING("Z_Axis"),
	MikanStencilCullMode_Y_AXIS ENUMVALUE_STRING("Y_Axis"),
	MikanStencilCullMode_X_AXIS ENUMVALUE_STRING("X_Axis"),
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanQuadStencilSystemValues :
	public MikanSystemValues
{
	static const char* k_systemName;

	FIELD()
	bool render_stencils;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanQuadStencilSystemValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanBoxStencilSystemValues :
	public MikanSystemValues
{
	static const char* k_systemName;

	FIELD()
	bool render_stencils;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanBoxStencilSystemValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanModelStencilSystemValues :
	public MikanSystemValues
{
	static const char* k_systemName;

	FIELD()
	bool render_stencils;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanModelStencilSystemValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanStencilComponentValues :
	public MikanTransformComponentValues
{
	FIELD()
	MikanSpatialAnchorID parent_anchor_id;
	FIELD()
	bool is_disabled;
	FIELD()
	MikanStencilCullMode cull_mode;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilComponentValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanQuadStencilComponentValues :
	public MikanStencilComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	float quad_width;
	FIELD()
	float quad_height;
	FIELD()
	bool is_double_sided;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanQuadStencilComponentValues_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanBoxStencilComponentValues :
	public MikanStencilComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	float box_x_size;
	FIELD()
	float box_y_size;
	FIELD()
	float box_z_size;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanBoxStencilComponentValues_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanModelStencilComponentValues :
	public MikanStencilComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	Serialization::String model_path;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanModelStencilComponentValues_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanTriagulatedMesh
{
	FIELD()
	Serialization::List<MikanVector3f> vertices;
	FIELD()
	Serialization::List<MikanVector3f> normals;
	FIELD()
	Serialization::List<MikanVector2f> texels;
	FIELD()
	Serialization::List<int> indices;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanTriagulatedMesh_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilTypes")) MikanStencilModelRenderGeometry
{
	FIELD()
	Serialization::List<MikanTriagulatedMesh> meshes;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilModelRenderGeometry_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStencilTypes_GENERATED
#endif