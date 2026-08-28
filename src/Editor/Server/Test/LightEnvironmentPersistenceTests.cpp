#include "LightEnvironmentPersistenceTests.h"
#include "unit_test.h"

#include "Light/LightEnvironmentComponent.h"

#include <assert.h>
#include <memory>
#include <set>
#include <math.h>
#include <stdio.h>

namespace
{
// MulticastDelegate binds member functions rather than lambdas, so the listener
// has to be an object instead of a capture.
struct PropertyChangeListener
{
	int notificationCount= 0;
	std::set<std::string> changedProperties;

	void onPropertyChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
	{
		notificationCount++;
		for (const std::string& propertyName : changedPropertySet.getSet())
			changedProperties.insert(propertyName);
	}
};

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
	UNIT_TEST_MODULE_CALL_TEST(light_environment_test_apply_notifies_listeners);
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

// Regression guard: writing the members is not enough. CommonConfig only arms
// the project auto-save, and only forwards a change to connected clients, when
// notifyPropertyChanged fires. A setter that quietly assigns leaves the value
// live in memory alone - the project file keeps the old values and no client is
// ever told. That is exactly how the SH coefficients first failed to persist.
bool light_environment_test_apply_notifies_listeners()
{
	UNIT_TEST_BEGIN("applying an environment notifies listeners")

	// Heap allocated on purpose: notifyPropertyChanged reaches for
	// shared_from_this(), which throws on a stack object once a listener exists.
	auto definition= std::make_shared<LightEnvironmentDefinition>();

	PropertyChangeListener listener;
	definition->OnPropertyChanged+= MakeDelegate(&listener, &PropertyChangeListener::onPropertyChanged);

	definition->setLightingEnvironment(makeTestEnvironment());

	// One logical update must produce exactly one notification, not three -
	// otherwise the auto-save is re-armed and clients are woken repeatedly for
	// a single capture.
	printf("      notifications=%d properties=%d\n", listener.notificationCount,
		   (int)listener.changedProperties.size());
	success= (listener.notificationCount == 1);
	assert(success);

	// ...and it must name every field it actually changed, or a client that
	// only watches one of them silently misses the update.
	success&= (listener.changedProperties.count(LightEnvironmentDefinition::k_shCoefficientsPropertyId) == 1);
	assert(success);
	success&= (listener.changedProperties.count(LightEnvironmentDefinition::k_directionalityPropertyId) == 1);
	assert(success);
	success&= (listener.changedProperties.count(LightEnvironmentDefinition::k_keyLightDirectionPropertyId) == 1);
	assert(success);

	// The individual setters must notify too - the remote setPropertyValue path
	// goes through them rather than through setLightingEnvironment.
	PropertyChangeListener exposureListener;
	auto exposureDefinition= std::make_shared<LightEnvironmentDefinition>();
	exposureDefinition->OnPropertyChanged+= MakeDelegate(&exposureListener, &PropertyChangeListener::onPropertyChanged);
	exposureDefinition->setExposureScale(3.f);

	success&= (exposureListener.notificationCount == 1);
	assert(success);
	success&= (exposureListener.changedProperties.count(LightEnvironmentDefinition::k_exposureScalePropertyId) == 1);
	assert(success);

	UNIT_TEST_COMPLETE()
}
