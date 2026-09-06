#include "DMXPresetPersistenceTests.h"
#include "unit_test.h"

#include "CommonConfig.h"
#include "Light/DMXPresetComponent.h"
#include "MikanVariantTypes.h"
#include "MulticastDelegate.h"

#include <assert.h>
#include <memory>
#include <set>
#include <stdio.h>
#include <string>
#include <vector>

namespace
{
// MulticastDelegate binds member functions rather than lambdas, so the listener
// has to be an object instead of a capture.
struct DMXPresetChangeListener
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

DMXPresetDefinition::FixtureValueMap makeTestValues()
{
	DMXPresetDefinition::FixtureValueMap values;
	values[1223]= {255, 0, 128};
	values[1091]= {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	return values;
}

DMXPresetComponentPtr makePresetComponent()
{
	auto component= std::make_shared<DMXPresetComponent>(MikanObjectWeakPtr());
	component->setDefinition(std::make_shared<DMXPresetDefinition>());
	return component;
}
} // namespace

bool run_dmx_preset_persistence_tests()
{
	UNIT_TEST_MODULE_BEGIN("dmx_preset_persistence")
	UNIT_TEST_MODULE_CALL_TEST(dmx_preset_test_json_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(dmx_preset_test_property_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(dmx_preset_test_channel_range_writes);
	UNIT_TEST_MODULE_END()
}

bool dmx_preset_test_json_round_trip()
{
	UNIT_TEST_BEGIN("definition round-trips fixtures of different lengths through JSON")

	DMXPresetDefinition source;
	source.setGroupId(1257);
	source.setAllFixtureValues(makeTestValues());

	DMXPresetDefinition restored;
	restored.readFromJSON(source.writeToJSON());

	success= (restored.getGroupId() == 1257);
	assert(success);
	success&= (restored.getFixtureValues() == makeTestValues());
	assert(success);

	// A definition with no fixtures reads back empty
	DMXPresetDefinition bare;
	DMXPresetDefinition bareRestored;
	bareRestored.setAllFixtureValues(makeTestValues());
	bareRestored.readFromJSON(bare.writeToJSON());
	success&= bareRestored.getFixtureValues().empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool dmx_preset_test_property_round_trip()
{
	UNIT_TEST_BEGIN("preset_data property re-applies its own text")

	DMXPresetComponentPtr source= makePresetComponent();
	source->getDMXPresetDefinition()->setAllFixtureValues(makeTestValues());

	MikanVariant text;
	success= source->getPropertyValue(DMXPresetDefinition::k_presetDataPropertyId, text);
	assert(success);
	success&= (text.value_type == MikanVariantType::STRING);
	assert(success);
	success&= (std::string(text.getUtf8Value()).find('\n') == std::string::npos);
	assert(success);

	DMXPresetComponentPtr target= makePresetComponent();
	success&= target->setPropertyValue(DMXPresetDefinition::k_presetDataPropertyId, text);
	assert(success);
	success&= (target->getDMXPresetDefinition()->getFixtureValues() == makeTestValues());
	assert(success);

	// The flat client views line up: ids in map order, one count per id, bytes concatenated
	MikanVariant fixtureIds, channelCounts, channelData;
	success&= target->getPropertyValue(DMXPresetComponent::k_fixtureIdsPropertyId, fixtureIds)
			  && target->getPropertyValue(DMXPresetComponent::k_channelCountsPropertyId, channelCounts)
			  && target->getPropertyValue(DMXPresetComponent::k_channelDataPropertyId, channelData);
	assert(success);
	success&= (fixtureIds.getIntArrayValue().size() == 2 && fixtureIds.getIntArrayValue()[0] == 1091);
	assert(success);
	success&= (channelCounts.getIntArrayValue()[0] == 12 && channelCounts.getIntArrayValue()[1] == 3);
	assert(success);
	success&= (channelData.getUByteArrayValue().size() == 15 && channelData.getUByteArrayValue()[12] == 255);
	assert(success);

	// Malformed text and a non-string value are refused without touching the table
	success&= !target->setPropertyValue(DMXPresetDefinition::k_presetDataPropertyId,
										MikanVariant(std::string("{\"fixtures\":[]}_guard")));
	assert(success);
	success&= !target->setPropertyValue(DMXPresetDefinition::k_presetDataPropertyId, MikanVariant(3));
	assert(success);
	success&= (target->getDMXPresetDefinition()->getFixtureValues() == makeTestValues());
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool dmx_preset_test_channel_range_writes()
{
	UNIT_TEST_BEGIN("channel range writes grow slices and notify exactly preset_data")

	DMXPresetComponentPtr component= makePresetComponent();
	DMXPresetDefinitionPtr definition= component->getDMXPresetDefinition();

	DMXPresetChangeListener listener;
	definition->OnPropertyChanged+= MakeDelegate(&listener, &DMXPresetChangeListener::onPropertyChanged);

	// A write past the end grows the slice with zeros
	definition->setFixtureChannelRange(1223, 3, {7, 8, 9});
	std::vector<uint8_t> values;
	success= definition->getFixtureValues(1223, values) && values == std::vector<uint8_t>{0, 0, 0, 7, 8, 9};
	assert(success);
	success&= (listener.notificationCount == 1);
	assert(success);
	success&= (listener.changedProperties == std::set<std::string>{DMXPresetDefinition::k_presetDataPropertyId});
	assert(success);

	// The same bytes again are a no-op
	definition->setFixtureChannelRange(1223, 3, {7, 8, 9});
	success&= (listener.notificationCount == 1);
	assert(success);

	// A write inside the slice keeps its length
	definition->setFixtureChannelRange(1223, 0, {255});
	success&= definition->getFixtureValues(1223, values) && values == std::vector<uint8_t>{255, 0, 0, 7, 8, 9};
	assert(success);
	success&= (listener.notificationCount == 2);
	assert(success);

	// Removing an absent fixture does not notify; removing a present one does
	success&= !definition->removeFixtureValues(42);
	assert(success);
	success&= definition->removeFixtureValues(1223) && (listener.notificationCount == 3);
	assert(success);

	definition->OnPropertyChanged-= MakeDelegate(&listener, &DMXPresetChangeListener::onPropertyChanged);

	UNIT_TEST_COMPLETE()
}
