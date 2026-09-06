#include "DMXSequenceTests.h"
#include "unit_test.h"

#include "Light/DMXSequenceComponent.h"

#include <assert.h>
#include <math.h>
#include <memory>
#include <stdio.h>
#include <vector>

namespace
{
DMXSequenceComponentPtr makeSequenceComponent()
{
	auto component= std::make_shared<DMXSequenceComponent>(MikanObjectWeakPtr());
	component->setDefinition(std::make_shared<DMXSequenceDefinition>());
	return component;
}
} // namespace

bool run_dmx_sequence_tests()
{
	UNIT_TEST_MODULE_BEGIN("dmx_sequence")
	UNIT_TEST_MODULE_CALL_TEST(dmx_sequence_test_json_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(dmx_sequence_test_advance_time);
	UNIT_TEST_MODULE_CALL_TEST(dmx_sequence_test_frame_buffer_writers);
	UNIT_TEST_MODULE_END()
}

bool dmx_sequence_test_json_round_trip()
{
	UNIT_TEST_BEGIN("definition round-trips through JSON")

	DMXSequenceDefinition source;
	source.setGroupId(1263);
	source.setSequenceName("chase");
	source.setDurationSeconds(2.5f);
	source.setLoop(false);

	DMXSequenceDefinition restored;
	restored.readFromJSON(source.writeToJSON());

	success= (restored.getGroupId() == 1263);
	assert(success);
	success&= (restored.getSequenceName() == "chase");
	assert(success);
	success&= fabsf(restored.getDurationSeconds() - 2.5f) < 1e-5f;
	assert(success);
	success&= !restored.getLoop();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool dmx_sequence_test_advance_time()
{
	UNIT_TEST_BEGIN("advanceTime wraps, finishes, or runs unbounded")

	bool bWrapped= false;
	bool bFinished= false;

	// Looping wraps past the duration
	float time= 1.8f;
	DMXSequenceComponent::advanceTime(time, 0.5f, 2.f, true, bWrapped, bFinished);
	success= bWrapped && !bFinished && fabsf(time - 0.3f) < 1e-5f;
	assert(success);

	// Not looping clamps to the duration and finishes
	time= 1.8f;
	DMXSequenceComponent::advanceTime(time, 0.5f, 2.f, false, bWrapped, bFinished);
	success&= !bWrapped && bFinished && time == 2.f;
	assert(success);

	// Inside the duration nothing happens
	time= 0.5f;
	DMXSequenceComponent::advanceTime(time, 0.25f, 2.f, false, bWrapped, bFinished);
	success&= !bWrapped && !bFinished && fabsf(time - 0.75f) < 1e-5f;
	assert(success);

	// Zero duration is unbounded
	time= 100.f;
	DMXSequenceComponent::advanceTime(time, 1.f, 0.f, false, bWrapped, bFinished);
	success&= !bWrapped && !bFinished && time == 101.f;
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool dmx_sequence_test_frame_buffer_writers()
{
	UNIT_TEST_BEGIN("frame buffer writers grow and overwrite slices")

	DMXSequenceComponentPtr sequence= makeSequenceComponent();

	std::vector<uint8_t> values;
	success= !sequence->getFrameBufferValues(1223, values);
	assert(success);

	// A color write creates a three-byte slice
	sequence->setFixtureColor(1223, 255, 40, 0);
	success&= sequence->getFrameBufferValues(1223, values) && values == std::vector<uint8_t>{255, 40, 0};
	assert(success);

	// A channel write replaces the slice at any length
	sequence->setFixtureChannels(1223, {1, 2, 3, 4, 5, 6});
	success&= sequence->getFrameBufferValues(1223, values) && values.size() == 6 && values[5] == 6;
	assert(success);

	// A color write onto a longer slice keeps the tail
	sequence->setFixtureColor(1223, 9, 9, 9);
	success&= sequence->getFrameBufferValues(1223, values) && values == std::vector<uint8_t>{9, 9, 9, 4, 5, 6};
	assert(success);

	// Without a group a pixel write is ignored rather than guessing a layout
	sequence->setPixel(1091, 0, 0, 1, 2, 3);
	success&= !sequence->getFrameBufferValues(1091, values);
	assert(success);

	UNIT_TEST_COMPLETE()
}
