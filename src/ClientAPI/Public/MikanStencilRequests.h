#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanStencilTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStencilRequests.rfkh.h"
#endif

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

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStencilRequests_GENERATED
#endif