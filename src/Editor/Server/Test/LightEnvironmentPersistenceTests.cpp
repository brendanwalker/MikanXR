#include "LightEnvironmentPersistenceTests.h"
#include "unit_test.h"

#include "Light/LightEnvironmentComponent.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

namespace
{
SHLightingEnvironment makeTestEnvironment()
{
	SHLightingEnvironment environment;
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		const float base= 0.1f * (float)(i + 1);
		environment.coefficients[i]= glm::vec3(base, base * 0.5f, -base * 0.25f);
	}
	return environment;
}
} // namespace

bool run_light_environment_persistence_tests()
{
	UNIT_TEST_MODULE_BEGIN("light_environment_persistence")
	UNIT_TEST_MODULE_CALL_TEST(light_environment_test_json_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(light_environment_test_coefficient_list_is_normalized);
	UNIT_TEST_MODULE_CALL_TEST(light_environment_test_environment_conversion);
	UNIT_TEST_MODULE_END()
}

bool light_environment_test_json_round_trip()
{
	UNIT_TEST_BEGIN("definition round-trips through JSON")

	LightEnvironmentDefinition source;
	source.setLightingEnvironment(makeTestEnvironment());
	source.setExposureScale(2.5f);

	const configuru::Config serialized= source.writeToJSON();

	LightEnvironmentDefinition restored;
	restored.readFromJSON(serialized);

	success= (restored.getSHCoefficients().size() == source.getSHCoefficients().size());
	assert(success);

	if (success)
	{
		for (size_t i= 0; i < source.getSHCoefficients().size() && success; ++i)
		{
			success= fabsf(restored.getSHCoefficients()[i] - source.getSHCoefficients()[i]) < 1e-5f;
			assert(success);
		}
	}

	success&= fabsf(restored.getExposureScale() - 2.5f) < 1e-5f;
	assert(success);

	// Directionality and key direction are derived, so they must survive too -
	// a client that only reads those would otherwise silently see zeros.
	success&= fabsf(restored.getDirectionality() - source.getDirectionality()) < 1e-5f;
	assert(success);
	success&= fabsf(restored.getKeyLightDirection().x - source.getKeyLightDirection().x) < 1e-5f;
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool light_environment_test_coefficient_list_is_normalized()
{
	UNIT_TEST_BEGIN("coefficient list is always 27 entries")

	const size_t expectedCount= (size_t)k_shCoefficientCount * 3;

	// A fresh definition must already be the right length.
	LightEnvironmentDefinition fresh;
	success= (fresh.getSHCoefficients().size() == expectedCount);
	assert(success);

	// Too short: the tail must be zero filled rather than left missing.
	LightEnvironmentDefinition tooShort;
	tooShort.setSHCoefficients({1.f, 2.f, 3.f});
	success&= (tooShort.getSHCoefficients().size() == expectedCount);
	assert(success);
	success&= (tooShort.getSHCoefficients()[0] == 1.f);
	assert(success);
	success&= (tooShort.getSHCoefficients()[expectedCount - 1] == 0.f);
	assert(success);

	// Too long: the excess must be dropped, not overflow the buffer.
	LightEnvironmentDefinition tooLong;
	tooLong.setSHCoefficients(std::vector<float>(expectedCount + 17, 4.f));
	success&= (tooLong.getSHCoefficients().size() == expectedCount);
	assert(success);

	// A config with no coefficient key at all must still yield 27 entries.
	configuru::Config empty= configuru::Config::object();
	LightEnvironmentDefinition fromEmpty;
	fromEmpty.setSHCoefficients(std::vector<float>(expectedCount, 9.f));
	fromEmpty.readFromJSON(empty);
	success&= (fromEmpty.getSHCoefficients().size() == expectedCount);
	assert(success);
	success&= (fromEmpty.getSHCoefficients()[0] == 0.f);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool light_environment_test_environment_conversion()
{
	UNIT_TEST_BEGIN("environment conversion and exposure scaling")

	const SHLightingEnvironment truth= makeTestEnvironment();

	LightEnvironmentDefinition definition;
	definition.setLightingEnvironment(truth);

	const SHLightingEnvironment restored= definition.getLightingEnvironment();
	for (int i= 0; i < k_shCoefficientCount && success; ++i)
	{
		success= glm::length(restored.coefficients[i] - truth.coefficients[i]) < 1e-5f;
		assert(success);
	}

	// Derived values must match what the math type reports directly.
	success&= fabsf(definition.getDirectionality() - truth.getDirectionality()) < 1e-5f;
	assert(success);

	printf("      directionality = %.4f\n", definition.getDirectionality());

	UNIT_TEST_COMPLETE()
}
