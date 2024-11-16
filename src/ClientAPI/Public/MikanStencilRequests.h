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
	inline static const char* k_typeName = "GetQuadStencilList";
	GetQuadStencilList() : MikanRequest(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetQuadStencilList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetQuadStencil :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetQuadStencil";
	GetQuadStencil() : MikanRequest(k_typeName) {}

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
	inline static const char* k_typeName = "GetBoxStencilList";
	GetBoxStencilList() : MikanRequest(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetBoxStencilList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetBoxStencil :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetBoxStencil";
	GetBoxStencil() : MikanRequest(k_typeName) {}

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
	inline static const char* k_typeName = "GetModelStencilList";
	GetModelStencilList() : MikanRequest(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetModelStencilList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) GetModelStencil :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetModelStencil";
	GetModelStencil() : MikanRequest(k_typeName) {}

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
	inline static const char* k_typeName = "GetModelStencilRenderGeometry";
	GetModelStencilRenderGeometry() : MikanRequest(k_typeName) {}

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
	inline static const std::string k_typeName = "MikanStencilQuadInfoResponse";

	FIELD()
	MikanStencilQuadInfo quad_info; 

	MikanStencilQuadInfoResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilQuadInfoResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilBoxInfoResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilBoxInfoResponse";

	FIELD()
	MikanStencilBoxInfo box_info;

	MikanStencilBoxInfoResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilBoxInfoResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilModelInfoResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModelInfoResponse";

	FIELD()
	MikanStencilModelInfo model_info;

	MikanStencilModelInfoResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilModelInfoResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilListResponse 
	: public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilListResponse";

	FIELD()
	Serialization::List<MikanStencilID> stencil_id_list;

	MikanStencilListResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilListResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilRequest")) MikanStencilModelRenderGeometryResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModelRenderGeometryResponse";

	FIELD()
	MikanStencilModelRenderGeometry render_geometry;

	MikanStencilModelRenderGeometryResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilModelRenderGeometryResponse_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStencilRequests_GENERATED
#endif