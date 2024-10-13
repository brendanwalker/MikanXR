#pragma	once

#include "SerializableList.h"
#include "serialization_unit_tests.rfkh.h"

#include <string>
#include <stdint.h>

enum ENUM() SerializationTestEnum
{
	SerializationTestEnum_Value1,
	SerializationTestEnum_Value2,
	SerializationTestEnum_Value3,
};

struct STRUCT() SerializationPointStruct
{
	FIELD()
	float x_field;

	FIELD()
	float y_field;

	SerializationPointStruct_GENERATED
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
	std::string string_field;

	FIELD()
	SerializationTestEnum enum_field;

	FIELD()
	SerializationPointStruct point_field;

	FIELD()
	Serialization::List<bool> bool_array;

	FIELD()
	Serialization::List<SerializationPointStruct> point_array;

	SerializationTestStruct_GENERATED
};

File_serialization_unit_tests_GENERATED