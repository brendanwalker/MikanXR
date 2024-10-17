//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "BinarySerializer.h"
#include "BinaryDeserializer.h"
#include "BinaryUtility.h"
#include "JsonSerializer.h"
#include "JsonDeserializer.h"
#include "MathUtility.h"
#include "SerializableList.h"
#include "SerializableMap.h"

#include "serialization_unit_tests.h"
#include "serialization_unit_tests.rfks.h"
#include "unit_test.h"

//-- public interface -----
bool run_serialization_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("serialization")
		UNIT_TEST_MODULE_CALL_TEST(serialization_utility_test_endian_swap);
		UNIT_TEST_MODULE_CALL_TEST(serialization_utility_test_reflection_from_json);
		UNIT_TEST_MODULE_CALL_TEST(serialization_utility_test_reflection_from_bytes);
	UNIT_TEST_MODULE_END()
}

//-- private functions -----
void test_endian_mode(Serialization::Endian endianMode)
{
	uint8_t scratch[32];

	int16_t expectedValue16 = 0x1234;
	Serialization::write_int16((uint8_t*)&scratch, expectedValue16, endianMode);
	int16_t actualValue16 = Serialization::read_int16((uint8_t*)&scratch, endianMode);
	assert(actualValue16 == expectedValue16);

	int32_t expectedValue24 = 0x123456;
	Serialization::write_int24((uint8_t*)&scratch, expectedValue24, endianMode);
	int32_t actualValue24 = Serialization::read_int24((uint8_t*)&scratch, endianMode);
	assert(actualValue24 == expectedValue24);

	int32_t expectedValue32 = 0x12345678;
	Serialization::write_int32((uint8_t*)&scratch, expectedValue32, endianMode);
	int32_t actualValue32 = Serialization::read_int32((uint8_t*)&scratch, endianMode);
	assert(actualValue32 == expectedValue32);

	int64_t expectedValue64 = 0x1234567890abcdef;
	Serialization::write_int64((uint8_t*)&scratch, expectedValue64, endianMode);
	int64_t actualValue64 = Serialization::read_int64((uint8_t*)&scratch, endianMode);
	assert(actualValue64 == expectedValue64);

	float expectedFloat = k_real_pi;
	Serialization::write_float((uint8_t*)&scratch, expectedFloat, endianMode);
	float actualFloat = Serialization::read_float((uint8_t*)&scratch, endianMode);
	assert(is_nearly_equal(expectedFloat, actualFloat, k_real_epsilon));

	double expectedDouble = k_real64_pi;
	Serialization::write_double((uint8_t*)&scratch, expectedDouble, endianMode);
	double actualDouble = Serialization::read_double((uint8_t*)&scratch, endianMode);
	assert(is_double_nearly_equal(expectedDouble, actualDouble, k_real64_epsilon));
}

bool serialization_utility_test_endian_swap()
{
	UNIT_TEST_BEGIN("endian swap")

		test_endian_mode(Serialization::Endian::Little);
		test_endian_mode(Serialization::Endian::Big);

	UNIT_TEST_COMPLETE()
}

void build_serialization_test_struct(SerializationTestStruct& outStruct)
{
	Serialization::List<bool> boolArray;
	boolArray.push_back(true);
	boolArray.push_back(false);
	boolArray.push_back(true);

	Serialization::List<SerializationPointStruct> pointArray;
	pointArray.push_back({1.2345f, 5.4321f});
	pointArray.push_back({5.4321f, 1.2345f});

	Serialization::Map<int, SerializationPointStruct> intPointMap;
	intPointMap.insert({1, {1.2345f, 5.4321f}});
	intPointMap.insert({2, {5.4321f, 1.2345f}});

	Serialization::Map<std::string, SerializationPointStruct> stringPointMap;
	stringPointMap.insert({"key1", {1.2345f, 5.4321f}});
	stringPointMap.insert({"key2", {5.4321f, 1.2345f}});

	outStruct = {
		true,
		-123,
		123,
		-1234,
		1234,
		-123456,
		123456,
		-123456789,
		123456789,
		1.2345f,
		1.23456789,
		"hello",
		SerializationTestEnum_Value2,
		{1.2345f, 5.4321f},
		boolArray,
		pointArray,
		intPointMap,
		stringPointMap
	};
}

void verify_serialization_test_struct(const SerializationTestStruct& actual, const SerializationTestStruct& expected)
{
	assert(actual.bool_field == expected.bool_field);
	assert(actual.byte_field == expected.byte_field);
	assert(actual.ubyte_field == expected.ubyte_field);
	assert(actual.short_field == expected.short_field);
	assert(actual.ushort_field == expected.ushort_field);
	assert(actual.int_field == expected.int_field);
	assert(actual.uint_field == expected.uint_field);
	assert(actual.long_field == expected.long_field);
	assert(actual.ulong_field == expected.ulong_field);
	assert(is_nearly_equal(actual.float_field, expected.float_field, k_real_epsilon));
	assert(is_double_nearly_equal(actual.double_field, expected.double_field, k_real64_epsilon));
	assert(actual.string_field == expected.string_field);
	assert(actual.enum_field == expected.enum_field);
	assert(is_nearly_equal(actual.point_field.x_field, expected.point_field.x_field, k_real_epsilon));
	assert(is_nearly_equal(actual.point_field.y_field, expected.point_field.y_field, k_real_epsilon));

	assert(actual.bool_array.size() == expected.bool_array.size());
	for (size_t i = 0; i < actual.bool_array.size(); ++i)
	{
		assert(actual.bool_array[i] == expected.bool_array[i]);
	}

	assert(actual.point_array.size() == expected.point_array.size());
	for (size_t i = 0; i < 2; ++i)
	{
		const auto& actualPoint = actual.point_array[i];
		const auto& expectedPoint = expected.point_array[i];

		assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, k_real_epsilon));
		assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, k_real_epsilon));
	}

	assert(actual.int_point_map.size() == expected.int_point_map.size());
	for (const auto& pair : actual.int_point_map)
	{
		const int key = pair.first;
		const auto& actualPoint = pair.second;
		const auto iter = expected.int_point_map.find(key);
		const auto& expectedPoint = iter->second;

		assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, k_real_epsilon));
		assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, k_real_epsilon));
	}

	assert(actual.string_point_map.size() == expected.string_point_map.size());
	for (const auto& pair : actual.string_point_map)
	{
		const std::string& key = pair.first;
		const auto& actualPoint = pair.second;
		const auto iter = expected.string_point_map.find(key);
		const auto& expectedPoint = iter->second;

		assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, k_real_epsilon));
		assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, k_real_epsilon));
	}
}

bool serialization_utility_test_reflection_from_json()
{
	UNIT_TEST_BEGIN("reflect from json")
		SerializationTestStruct expected = {};
		build_serialization_test_struct(expected);

		std::string jsonString;
		bool bCanSerialize= Serialization::serializeToJsonString(expected, jsonString);
		assert(bCanSerialize);

		SerializationTestStruct actual= {};
		bool bCanDeserialize = Serialization::deserializeFromJsonString(jsonString, actual);

		verify_serialization_test_struct(actual, expected);

	UNIT_TEST_COMPLETE()
}

bool serialization_utility_test_reflection_from_bytes()
{
	UNIT_TEST_BEGIN("reflect from json")
		SerializationTestStruct expected;
		build_serialization_test_struct(expected);

		std::vector<uint8_t> bytes;
		bool bCanSerialize= Serialization::serializeToBytes(expected, bytes);
		assert(bCanSerialize);

		SerializationTestStruct actual= {};
		bool bCanDeserialize = Serialization::deserializeFromBytes(bytes, actual);

		verify_serialization_test_struct(actual, expected);
	UNIT_TEST_COMPLETE()
}