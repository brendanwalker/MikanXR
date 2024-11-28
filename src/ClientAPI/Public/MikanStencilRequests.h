#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanStencilTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStencilRequests.rfkh.h"
#endif

// Stencil Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetQuadStencilList :
	public MikanRequest
{
public:
	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetQuadStencilList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetQuadStencil :
	public MikanRequest
{
public:
	FIELD()
	MikanStencilID stencilId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetQuadStencil_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetBoxStencilList :
	public MikanRequest
{
public:
	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetBoxStencilList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetBoxStencil :
	public MikanRequest
{
public:
	FIELD()
	MikanStencilID stencilId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetBoxStencil_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetModelStencilList :
	public MikanRequest
{
public:
	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetModelStencilList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetModelStencil :
	public MikanRequest
{
public:
	FIELD()
	MikanStencilID stencilId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetModelStencil_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetModelStencilRenderGeometry :
	public MikanRequest
{
public:
	FIELD()
	MikanStencilID stencilId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetModelStencilRenderGeometry_GENERATED
	#endif
};

// Stencil Response Types
// ------

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilQuadInfoResponse : 
	public MikanResponse
{
	FIELD()
	MikanStencilQuadInfo quad_info; 

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilQuadInfoResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilBoxInfoResponse : 
	public MikanResponse
{
	FIELD()
	MikanStencilBoxInfo box_info;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilBoxInfoResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilModelInfoResponse : 
	public MikanResponse
{
	FIELD()
	MikanStencilModelInfo model_info;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilModelInfoResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilListResponse 
	: public MikanResponse
{
	FIELD()
	Serialization::List<MikanStencilID> stencil_id_list;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilListResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilModelRenderGeometryResponse : 
	public MikanResponse
{
	FIELD()
	MikanStencilModelRenderGeometry render_geometry;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilModelRenderGeometryResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStencilRequests_GENERATED
#endif