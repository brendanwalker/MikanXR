#pragma	once

#include "SerializableList.h"
#include "SerializableMap.h"
#include "SerializableObjectPtr.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifndef KODGEN_PARSING
#include "serialization_unit_tests.rfkh.h"
#endif

#include <string>
#include <stdint.h>

enum ENUM() SerializationTestEnum
{
	SerializationTestEnum_Value1 ENUMVALUE(Serialization::EnumStringValue("Value1")),
	SerializationTestEnum_Value2 ENUMVALUE(Serialization::EnumStringValue("Value2")),
	SerializationTestEnum_Value3 ENUMVALUE(Serialization::EnumStringValue("Value3")),
};

struct STRUCT() SerializationPointStruct
{
	virtual ~SerializationPointStruct() = default;

	#ifndef KODGEN_PARSING
	SerializationPointStruct_GENERATED
	#endif
};

struct STRUCT() SerializationPoint2dStruct : public SerializationPointStruct
{
	SerializationPoint2dStruct() : x_field{0.0f}, y_field{0.0f} {}
	SerializationPoint2dStruct(float x, float y) : x_field{x}, y_field{y} {}
	virtual ~SerializationPoint2dStruct() = default;
	
	FIELD()
	float x_field;

	FIELD()
	float y_field;

	#ifndef KODGEN_PARSING
	SerializationPoint2dStruct_GENERATED
	#endif
};

struct STRUCT() SerializationPoint3dStruct: public SerializationPointStruct
{
	SerializationPoint3dStruct() : x_field{0.0f}, y_field{0.0f}, z_field{0.0f} {}
	SerializationPoint3dStruct(float x, float y, float z) : x_field{x}, y_field{y}, z_field{z} {}
	virtual ~SerializationPoint3dStruct() = default;

	FIELD()
	float x_field;

	FIELD()
	float y_field;

	FIELD()
	float z_field;

	#ifndef KODGEN_PARSING
	SerializationPoint3dStruct_GENERATED
	#endif
};

struct STRUCT() SerializationTestStruct
{
	FIELD()
	bool bool_field;

	FIELD()
	int8_t byte_field;

	FIELD()
	uint8_t ubyte_field;

	FIELD()
	int16_t short_field;

	FIELD()
	uint16_t ushort_field;

	FIELD()
	int32_t int_field;

	FIELD()
	uint32_t uint_field;

	FIELD()
	int64_t long_field;

	FIELD()
	uint64_t ulong_field;

	FIELD()
	float float_field;

	FIELD()
	double double_field;

	FIELD()
	Serialization::String string_field;

	FIELD()
	SerializationTestEnum enum_field;

	FIELD()
	SerializationPoint2dStruct point2d_field;

	FIELD()
	Serialization::ObjectPtr<SerializationPointStruct> point_ptr_field;

	FIELD()
	Serialization::BoolList bool_array;

	FIELD()
	Serialization::List<int> int_array;

	FIELD()
	Serialization::List<SerializationPoint2dStruct> point2d_array;

	FIELD()
	Serialization::Map<int, SerializationPoint2dStruct> int_point_map;

	FIELD()
	Serialization::Map<std::string, SerializationPoint2dStruct> string_point_map;

	#ifndef KODGEN_PARSING
	SerializationTestStruct_GENERATED
	#endif
};

#ifndef KODGEN_PARSING
File_serialization_unit_tests_GENERATED
#endif