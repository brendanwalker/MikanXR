//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "MathUtility.h"
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
		const char* jsonString = R""""(
		{
			"bool_field": true,
			"byte_field": -123,
			"ubyte_field": 123,
			"short_field": -1234,
			"ushort_field": 1234,
			"int_field": -123456,
			"uint_field": 123456,
			"long_field": -123456789,
			"ulong_field": 123456789,
			"float_field": 1.2345,
			"double_field": 1.23456789,
			"string_field": "hello",
			"enum_field": "SerializationTestEnum_Value2",
			"point_field": {
				"x_field": 1.2345,
				"y_field": 5.4321
			},
			"bool_array": [true, false, true],
			"point_array": [
				{"x_field": 1.2345, "y_field": 5.4321},
				{"x_field": 5.4321, "y_field": 1.2345}
			],
			"int_point_map": [
				{
					"key": 1,
					"value": {"x_field": 1.2345, "y_field": 5.4321}
				},
				{
					"key": 2,
					"value": {"x_field": 5.4321, "y_field": 1.2345}
				}
			],
			"string_point_map": [
				{
					"key": "key1",
					"value": {"x_field": 1.2345, "y_field": 5.4321}
				},
				{
					"key": "key2",
					"value": {"x_field": 5.4321, "y_field": 1.2345}
				}
			]
		}
		)"""";

	FIELD()
		Serialization::Map<int, SerializationPointStruct> int_point_map;

	FIELD()
		Serialization::Map<std::string, SerializationPointStruct> string_point_map;

		SerializationTestStruct instance= {};
		bool bSuccess = Serialization::deserializefromJsonString(jsonString, instance);

		assert(bSuccess);
		assert(instance.bool_field == true);
		assert(instance.byte_field == -123);
		assert(instance.ubyte_field == 123);
		assert(instance.short_field == -1234);
		assert(instance.ushort_field == 1234);
		assert(instance.int_field == -123456);
		assert(instance.uint_field == 123456);
		assert(instance.long_field == -123456789);
		assert(instance.ulong_field == 123456789);
		assert(is_nearly_equal(instance.float_field, 1.2345f, 0.0001f));
		assert(is_double_nearly_equal(instance.double_field, 1.23456789, 0.0000001));
		assert(instance.string_field == "hello");
		assert(instance.enum_field == SerializationTestEnum_Value2);
		assert(is_nearly_equal(instance.point_field.x_field, 1.2345f, 0.0001f));
		assert(is_nearly_equal(instance.point_field.y_field, 5.4321, 0.0001f));

		bool expextedBoolArray[3] = {true, false, true};
		assert(instance.bool_array.size() == 3);
		for (size_t i = 0; i < 3; ++i)
		{
			assert(instance.bool_array[i] == expextedBoolArray[i]);
		}

		SerializationPointStruct expectedPointArray[2] = {
			{1.2345f, 5.4321f},
			{5.4321f, 1.2345f}
		};
		assert(instance.point_array.size() == 2);
		for (size_t i = 0; i < 2; ++i)
		{
			const auto& actualPoint= instance.point_array[i];
			const auto& expectedPoint= expectedPointArray[i];

			assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, 0.0001f));
			assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, 0.0001f));
		}

		std::map<int, SerializationPointStruct> expectedIntPointMap = {
			{1, {1.2345f, 5.4321f}},
			{2, {5.4321f, 1.2345f}}
		};
		assert(instance.int_point_map.size() == 2);
		for (const auto& pair : instance.int_point_map)
		{
			const auto& actualPoint= pair.second;
			const auto& expectedPoint= expectedIntPointMap[pair.first];

			assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, 0.0001f));
			assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, 0.0001f));
		}

		std::map<std::string, SerializationPointStruct> expectedStringPointMap = {
			{"key1", {1.2345f, 5.4321f}},
			{"key2", {5.4321f, 1.2345f}}
		};
		assert(instance.string_point_map.size() == 2);
		for (const auto& pair : instance.string_point_map)
		{
			const auto& actualPoint= pair.second;
			const auto& expectedPoint= expectedStringPointMap[pair.first];

			assert(is_nearly_equal(actualPoint.x_field, expectedPoint.x_field, 0.0001f));
			assert(is_nearly_equal(actualPoint.y_field, expectedPoint.y_field, 0.0001f));
		}


	UNIT_TEST_COMPLETE()
}