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

/// The format of the DMX buffer stored in universe's channel buffer
enum class ENUM(Serialization::CodeGenModule("MikanLightTypes")) MikanDMXBufferFormat : int
{
	DMXUncompressed ENUMVALUE_STRING("DMXUncompressed")= 0,
	DMXRLEEncoded ENUMVALUE_STRING("DMXRLEEncoded")= 1,
};

/// DMX Channel Data for a single DMX Universe
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanUniverseDMXData
{
	// The DMX Universe ID for this channel data
	FIELD() uint16_t dmx_universe_id= 1;

	// The format of buffer (i.e. compressed or raw)
	FIELD() MikanDMXBufferFormat buffer_format= MikanDMXBufferFormat::DMXUncompressed;

	// The dmx channel data buffer
	FIELD() Serialization::List<uint8_t> buffer_data;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanUniverseDMXData_GENERATED
#endif
};

// Run-Length-Encode(RLE) a source DMX channel buffer into a MikanUniverseDMXData.
// Fills outUniverseData->buffer_data with [count(1..255), value] pairs and sets
// buffer_format to RLEEncoded. Returns the number of encoded bytes written.
MIKAN_API_FUNC(size_t) mikanRLEEncodeDMXUniverseBuffer(const size_t in_buffer_size, const uint8_t* in_buffer,
													   MikanUniverseDMXData* outUniverseData);

// Extract the raw channel values from a MikanUniverseDMXData. Honors buffer_format:
// an RLEEncoded buffer is expanded, a Raw buffer is copied directly. Writes at most
// out_buffer_max_size bytes and returns the number of bytes written to out_buffer.
MIKAN_API_FUNC(size_t) mikanRLEDecodeDMXUniverseBuffer(const MikanUniverseDMXData* universeData,
													   const size_t out_buffer_max_size, uint8_t* out_buffer);

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightTypes")) MikanDMXData
{
	// The server timestamp for the universe data
	FIELD() double server_time_seconds;

	// DMX data for all requested universes
	FIELD() Serialization::List<MikanUniverseDMXData> universes;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDMXData_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanLightTypes_GENERATED
#endif
