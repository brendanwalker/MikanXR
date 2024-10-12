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
			}
		}
		)"""";

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

	UNIT_TEST_COMPLETE()
}