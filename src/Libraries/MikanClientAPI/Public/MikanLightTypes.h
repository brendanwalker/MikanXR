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
#include "MikanLightTypes.rfkh.h"
#endif

// -- System Values --

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanDMXObjectSystemValues
	: public MikanSystemValues
{
	static const char* k_systemName;

	FIELD() Serialization::String network_interface_ip;

	FIELD() uint8_t dmx_priority= 100;

	FIELD() float transmit_rate_hz= 44.0f;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDMXObjectSystemValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanRGBSpotLightSystemValues
	: public MikanSystemValues
{
	static const char* k_systemName;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBSpotLightSystemValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanRGBPixelGridSystemValues
	: public MikanSystemValues
{
	static const char* k_systemName;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBPixelGridSystemValues_GENERATED
#endif
};

// -- Component Values --

/// Base values shared by all DMX fixture types.
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanDMXFixtureComponentValues
	: public MikanTransformComponentValues
{
	FIELD() MikanStageID stage_id= INVALID_MIKAN_ID;

	FIELD() uint16_t dmx_universe= 1;

	FIELD() uint16_t dmx_start_channel= 1;

	FIELD() uint16_t dmx_channel_count= 3;

	FIELD() bool is_disabled= false;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDMXFixtureComponentValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanRGBSpotLightComponentValues
	: public MikanDMXFixtureComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD() uint8_t red= 0;

	FIELD() uint8_t green= 0;

	FIELD() uint8_t blue= 0;

	FIELD() float cone_angle_degrees= 0.f;

	FIELD() float cone_range_meters= 0.f;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBSpotLightComponentValues_GENERATED
#endif
};

/// Pixel grid values — pixel data is NOT included (use SetLightDMXData request for bulk writes).
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanRGBPixelGridComponentValues
	: public MikanDMXFixtureComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD() int grid_columns= 8;

	FIELD() int grid_rows= 8;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanRGBPixelGridComponentValues_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanDMXData
{
	FIELD() Serialization::List<uint8_t> channel_data;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDMXData_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanLightTypes_GENERATED
#endif
