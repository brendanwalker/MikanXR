#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanLightTypes.h"
#include "SerializableList.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanLightRequests.rfkh.h"
#endif

// -- RGB Spot Light Requests --

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) CreateRGBSpotLight :
	public MikanRequest
{
	CreateRGBSpotLight()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(CreateRGBSpotLight)
	}

#ifdef MIKANAPI_REFLECTION_ENABLED
	CreateRGBSpotLight_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) DestroyRGBSpotLight :
	public MikanRequest
{
	DestroyRGBSpotLight()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(DestroyRGBSpotLight)
	}

	FIELD()
	MikanLightID light_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	DestroyRGBSpotLight_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) GetRGBSpotLightList :
	public MikanRequest
{
	GetRGBSpotLightList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetRGBSpotLightList)
	}

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetRGBSpotLightList_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) MikanRGBSpotLightListResponse :
	public MikanResponse
{
	MikanRGBSpotLightListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanRGBSpotLightListResponse)
	}

	FIELD()
	Serialization::List<MikanLightID> light_ids;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBSpotLightListResponse_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) GetRGBSpotLight :
	public MikanRequest
{
	GetRGBSpotLight()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetRGBSpotLight)
	}

	FIELD()
	MikanLightID light_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetRGBSpotLight_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) MikanRGBSpotLightResponse :
	public MikanResponse
{
	MikanRGBSpotLightResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanRGBSpotLightResponse)
	}

	FIELD()
	MikanRGBSpotLightComponentValues light_info;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBSpotLightResponse_GENERATED
#endif
};

// -- RGB Pixel Grid Requests --

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) CreateRGBPixelGrid :
	public MikanRequest
{
	CreateRGBPixelGrid()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(CreateRGBPixelGrid)
	}

#ifdef MIKANAPI_REFLECTION_ENABLED
	CreateRGBPixelGrid_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) DestroyRGBPixelGrid :
	public MikanRequest
{
	DestroyRGBPixelGrid()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(DestroyRGBPixelGrid)
	}

	FIELD()
	MikanLightID grid_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	DestroyRGBPixelGrid_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) GetRGBPixelGridList :
	public MikanRequest
{
	GetRGBPixelGridList()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetRGBPixelGridList)
	}

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetRGBPixelGridList_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) MikanRGBPixelGridListResponse :
	public MikanResponse
{
	MikanRGBPixelGridListResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanRGBPixelGridListResponse)
	}

	FIELD()
	Serialization::List<MikanLightID> grid_ids;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBPixelGridListResponse_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) GetRGBPixelGrid :
	public MikanRequest
{
	GetRGBPixelGrid()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetRGBPixelGrid)
	}

	FIELD()
	MikanLightID grid_id = INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetRGBPixelGrid_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) MikanRGBPixelGridResponse :
	public MikanResponse
{
	MikanRGBPixelGridResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(MikanRGBPixelGridResponse)
	}

	FIELD()
	MikanRGBPixelGridComponentValues grid_info;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBPixelGridResponse_GENERATED
#endif
};

/// Bulk pixel write — bypasses the property system for performance.
/// pixel_data is a flat array of R,G,B triples in row-major order.
/// The count must match grid_columns * grid_rows * 3.
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) SetRGBPixelGridData :
	public MikanRequest
{
	SetRGBPixelGridData()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(SetRGBPixelGridData)
	}

	FIELD()
	MikanLightID grid_id = INVALID_MIKAN_ID;

	FIELD()
	Serialization::List<uint8_t> pixel_data;

#ifdef MIKANAPI_REFLECTION_ENABLED
	SetRGBPixelGridData_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanLightRequests_GENERATED
#endif
