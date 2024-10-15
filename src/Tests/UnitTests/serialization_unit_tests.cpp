//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "MathUtility.h"
#include "JsonSerializer.h"
#include "JsonDeserializer.h"
#include "serialization_unit_tests.h"
#include "serialization_unit_tests.rfks.h"
#include "unit_test.h"

//-- public interface -----
bool run_serialization_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("serialization")
		UNIT_TEST_MODULE_CALL_TEST(serialization_utility_test_reflection_from_json);
	UNIT_TEST_MODULE_END()
}

//-- private functions -----
bool serialization_utility_test_reflection_from_json()
{
	UNIT_TEST_BEGIN("reflect from json")
		Serialization::List<bool> expected_bool_array;
		expected_bool_array.push_back(true);
		expected_bool_array.push_back(false);
		expected_bool_array.push_back(true);

		Serialization::List<SerializationPointStruct> expected_point_array; 
		expected_point_array.push_back({1.2345f, 5.4321f});
		expected_point_array.push_back({5.4321f, 1.2345f});

		Serialization::Map<int, SerializationPointStruct> expected_int_point_map; 
		expected_int_point_map.insert({1, {1.2345f, 5.4321f}});
		expected_int_point_map.insert({2, {5.4321f, 1.2345f}});

		Serialization::Map<std::string, SerializationPointStruct> expected_string_point_map;
		expected_string_point_map.insert({"key1", {1.2345f, 5.4321f}});
		expected_string_point_map.insert({"key2", {5.4321f, 1.2345f}});

		SerializationTestStruct expected= {
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
			expected_bool_array,
			expected_point_array,
			expected_int_point_map,
			expected_string_point_map
		};

		std::string jsonString;
		bool bCanSerialize= Serialization::serializeToJsonString(expected, jsonString);
		assert(bCanSerialize);

		SerializationTestStruct actual= {};
		bool bCanDeserialize = Serialization::deserializeFromJsonString(jsonString, actual);

		assert(bCanDeserialize);
		assert(actual.bool_field == expected.bool_field);
		assert(actual.byte_field == expected.byte_field);
		assert(actual.ubyte_field == expected.ubyte_field);
		assert(actual.short_field == expected.short_field);
		assert(actual.ushort_field == expected.ushort_field);
		assert(actual.int_field == expected.int_field);
		assert(actual.uint_field == expected.uint_field);
		assert(actual.long_field == expected.long_field);
		assert(actual.ulong_field == expected.ulong_field);
		assert(is_nearly_equal(actual.float_field, expected.float_field, 0.0001f));
		assert(is_double_nearly_equal(actual.double_field, expected.double_field, 0.0000001));
		assert(actual.string_field == expected.string_field);
		assert(actual.enum_field == expected.enum_field);
		assert(is_nearly_equal(actual.point_field.x_field, expected.point_field.x_field, 0.0001f));
		assert(is_nearly_equal(actual.point_field.y_field, expected.point_field.y_field, 0.0001f));

		assert(actual.bool_array.size() == expected.bool_array.size());
		for (size_t i = 0; i < actual.bool_array.size(); ++i)
		{
			assert(actual.bool_array[i] == expected.bool_array[i]);
		}

		assert(actual.point_array.size() == expected.point_array.size());
		for (size_t i = 0; i < 2; ++i)
		{
			const auto& actualPoint= actual.point_array[i];
			const auto& expectedPoint= expected.point_array[i];

			assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, 0.0001f));
			assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, 0.0001f));
		}

		assert(actual.int_point_map.size() == expected.int_point_map.size());
		for (const auto& pair : actual.int_point_map)
		{
			const auto& actualPoint= pair.second;
			const auto& expectedPoint= expected.int_point_map[pair.first];

			assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, 0.0001f));
			assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, 0.0001f));
		}

		assert(actual.string_point_map.size() == expected.string_point_map.size());
		for (const auto& pair : actual.string_point_map)
		{
			const auto& actualPoint= pair.second;
			const auto& expectedPoint= expected.string_point_map[pair.first];

			assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, 0.0001f));
			assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, 0.0001f));
		}


	UNIT_TEST_COMPLETE()
}