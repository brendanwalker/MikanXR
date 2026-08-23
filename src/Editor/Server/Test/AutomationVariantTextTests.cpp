#include "AutomationVariantTextTests.h"
#include "AutomationVariantText.h"
#include "AutomationProtocol.h"

#include "unit_test.h"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

namespace
{
// Parse the text form of a variant back into a variant of the same type and
// verify the second text form matches the first (text -> variant -> text).
static bool textRoundTrips(MikanVariantType dataType, const std::string& valueText)
{
	std::vector<std::string> tokens;
	std::string error;
	if (!AutomationProtocol::tokenizeCommandLine(valueText, tokens, error))
	{
		fprintf(stdout, "    textRoundTrips FAILED: tokenize '%s': %s\n", valueText.c_str(), error.c_str());
		return false;
	}

	MikanVariant value;
	if (!AutomationVariantText::textToVariant(dataType, tokens, value, error))
	{
		fprintf(stdout, "    textRoundTrips FAILED: parse '%s': %s\n", valueText.c_str(), error.c_str());
		return false;
	}

	if (value.value_type != dataType)
	{
		fprintf(stdout, "    textRoundTrips FAILED: '%s' parsed to wrong variant type\n", valueText.c_str());
		return false;
	}

	const std::string reprinted= AutomationVariantText::variantToText(value);
	if (reprinted != valueText)
	{
		fprintf(stdout, "    textRoundTrips FAILED: '%s' reprinted as '%s'\n", valueText.c_str(), reprinted.c_str());
		return false;
	}

	return true;
}

static bool parseFails(MikanVariantType dataType, const std::vector<std::string>& tokens)
{
	MikanVariant value;
	std::string error;
	if (AutomationVariantText::textToVariant(dataType, tokens, value, error))
	{
		fprintf(stdout, "    parseFails FAILED: parse unexpectedly succeeded\n");
		return false;
	}

	return !error.empty();
}
} // namespace

bool automation_variant_test_scalars_round_trip()
{
	UNIT_TEST_BEGIN("scalar types round-trip through text")

	success&= textRoundTrips(MikanVariantType::BOOL, "true");
	success&= textRoundTrips(MikanVariantType::BOOL, "false");
	success&= textRoundTrips(MikanVariantType::UBYTE, "255");
	success&= textRoundTrips(MikanVariantType::USHORT, "65535");
	success&= textRoundTrips(MikanVariantType::INT, "-123456");
	success&= textRoundTrips(MikanVariantType::LONG, "987654321");
	success&= textRoundTrips(MikanVariantType::FLOAT, "1.5");
	success&= textRoundTrips(MikanVariantType::FLOAT, "-0.25");
	success&= textRoundTrips(MikanVariantType::DOUBLE, "3.0625");
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_variant_test_strings_round_trip()
{
	UNIT_TEST_BEGIN("strings round-trip, quoted spans keeping their spaces")

	success&= textRoundTrips(MikanVariantType::STRING, "hello");

	// A quoted string with spaces arrives as one token and prints back raw
	std::vector<std::string> tokens;
	std::string error;
	MikanVariant value;
	success&= AutomationProtocol::tokenizeCommandLine("\"two words\"", tokens, error);
	success&= AutomationVariantText::textToVariant(MikanVariantType::STRING, tokens, value, error);
	success&= (AutomationVariantText::variantToText(value) == "two words");
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_variant_test_math_types_round_trip()
{
	UNIT_TEST_BEGIN("math types round-trip with one token per component")

	success&= textRoundTrips(MikanVariantType::VECTOR2F, "1 2");
	success&= textRoundTrips(MikanVariantType::VECTOR3F, "1.5 -2.5 3");
	success&= textRoundTrips(MikanVariantType::VECTOR4F, "1 2 3 4");
	success&= textRoundTrips(MikanVariantType::QUATERNIONF, "1 0 0 0");
	success&= textRoundTrips(MikanVariantType::MATRIX4F, "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
	success&= textRoundTrips(MikanVariantType::VECTOR3D, "0.125 -0.25 0.5");
	success&= textRoundTrips(MikanVariantType::QUATERNIOND, "0.5 0.5 0.5 0.5");
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_variant_test_quaternion_field_order()
{
	UNIT_TEST_BEGIN("quaternions parse in w x y z field order")

	std::vector<std::string> tokens= {"0.5", "0.25", "0.125", "0.0625"};
	MikanVariant value;
	std::string error;
	success&= AutomationVariantText::textToVariant(MikanVariantType::QUATERNIONF, tokens, value, error);
	if (success)
	{
		const MikanQuatf& q= value.getQuaternionfValue();
		success&= (q.w == 0.5f && q.x == 0.25f && q.y == 0.125f && q.z == 0.0625f);
	}
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_variant_test_parse_errors()
{
	UNIT_TEST_BEGIN("malformed values report parse errors")

	success&= parseFails(MikanVariantType::INT, {"notanumber"});
	success&= parseFails(MikanVariantType::UBYTE, {"256"});
	success&= parseFails(MikanVariantType::BOOL, {"maybe"});
	success&= parseFails(MikanVariantType::FLOAT, {"1.5", "2.5"});
	success&= parseFails(MikanVariantType::VECTOR3F, {"1", "2"});
	success&= parseFails(MikanVariantType::MATRIX4F, {"1", "2", "3"});
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_variant_test_array_set_unsupported()
{
	UNIT_TEST_BEGIN("array types refuse a set as unsupported")

	success&= parseFails(MikanVariantType::FLOAT_ARRAY, {"1", "2", "3"});
	success&= parseFails(MikanVariantType::STRING_MAP, {"key=value"});
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool run_automation_variant_text_tests()
{
	UNIT_TEST_MODULE_BEGIN("automation_variant_text")
	UNIT_TEST_MODULE_CALL_TEST(automation_variant_test_scalars_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(automation_variant_test_strings_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(automation_variant_test_math_types_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(automation_variant_test_quaternion_field_order);
	UNIT_TEST_MODULE_CALL_TEST(automation_variant_test_parse_errors);
	UNIT_TEST_MODULE_CALL_TEST(automation_variant_test_array_set_unsupported);
	UNIT_TEST_MODULE_END()
}
