#include "DMXUniverseRLETests.h"
#include "MikanLightTypes.h"
#include "unit_test.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace
{
// A standard DMX512 universe is 512 channels.
static const size_t k_dmxUniverseChannelCount= 512;

// Encode the source, decode it back, and verify the result matches byte-for-byte.
// Returns false (and prints a diagnostic) on any mismatch.
static bool roundTripMatches(const std::vector<uint8_t>& source)
{
	MikanUniverseDMXData universeData;
	const size_t encodedSize= mikanRLEEncodeDMXUniverseBuffer(source.size(), source.data(), &universeData);

	if (encodedSize != universeData.buffer_data.size())
	{
		fprintf(stdout, "    roundTripMatches FAILED: returned size %zu != buffer_data size %zu\n", encodedSize,
				universeData.buffer_data.size());
		return false;
	}

	// Decode into a buffer large enough to hold the full source.
	std::vector<uint8_t> decoded(source.size() + 16, 0xCD);
	const size_t decodedSize= mikanRLEDecodeDMXUniverseBuffer(&universeData, decoded.size(), decoded.data());

	if (decodedSize != source.size())
	{
		fprintf(stdout, "    roundTripMatches FAILED: decoded size %zu != source size %zu\n", decodedSize,
				source.size());
		return false;
	}

	for (size_t i= 0; i < source.size(); ++i)
	{
		if (decoded[i] != source[i])
		{
			fprintf(stdout, "    roundTripMatches FAILED: byte %zu decoded=%u source=%u\n", i, decoded[i], source[i]);
			return false;
		}
	}

	return true;
}
} // namespace

// A realistic DMX universe (mostly zeros with a few lit channels) must round-trip
// and the RLE form must be smaller than the raw 512 bytes.
bool dmx_rle_test_typical_universe_compresses()
{
	UNIT_TEST_BEGIN("typical DMX universe round-trips and compresses")

	std::vector<uint8_t> source(k_dmxUniverseChannelCount, 0);
	source[0]= 255; // a single RGB fixture lit at full white on channels 1-3
	source[1]= 255;
	source[2]= 255;
	source[100]= 64;

	MikanUniverseDMXData universeData;
	const size_t encodedSize= mikanRLEEncodeDMXUniverseBuffer(source.size(), source.data(), &universeData);

	success&= (universeData.buffer_format == MikanDMXBufferFormat::DMXRLEEncoded);
	success&= (encodedSize < source.size());
	success&= roundTripMatches(source);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// A run longer than the 255-byte max run length must be split across pairs and
// still decode back to the original (512 identical bytes -> three runs).
bool dmx_rle_test_long_run_splits()
{
	UNIT_TEST_BEGIN("run longer than 255 splits and round-trips")

	std::vector<uint8_t> source(k_dmxUniverseChannelCount, 7);

	MikanUniverseDMXData universeData;
	mikanRLEEncodeDMXUniverseBuffer(source.size(), source.data(), &universeData);

	// 512 = 255 + 255 + 2 -> three [count, value] pairs -> 6 encoded bytes.
	success&= (universeData.buffer_format == MikanDMXBufferFormat::DMXRLEEncoded);
	success&= (universeData.buffer_data.size() == 6);
	success&= roundTripMatches(source);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Incompressible input (every adjacent byte differs) must fall back to a Raw
// buffer that is no larger than the source, and still round-trip.
bool dmx_rle_test_incompressible_falls_back_to_raw()
{
	UNIT_TEST_BEGIN("incompressible input falls back to DMXUncompressed")

	std::vector<uint8_t> source(k_dmxUniverseChannelCount, 0);
	for (size_t i= 0; i < source.size(); ++i)
		source[i]= static_cast<uint8_t>(i & 1 ? 0xAA : 0x55);

	MikanUniverseDMXData universeData;
	const size_t encodedSize= mikanRLEEncodeDMXUniverseBuffer(source.size(), source.data(), &universeData);

	success&= (universeData.buffer_format == MikanDMXBufferFormat::DMXUncompressed);
	success&= (encodedSize == source.size());
	success&= roundTripMatches(source);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Decode must honor a manually-constructed Raw buffer (copy straight through).
bool dmx_rle_test_decode_honors_raw_format()
{
	UNIT_TEST_BEGIN("decode copies a DMXUncompressed buffer straight through")

	MikanUniverseDMXData universeData;
	universeData.buffer_format= MikanDMXBufferFormat::DMXUncompressed;
	const uint8_t rawBytes[]= {10, 20, 30, 40, 50};
	universeData.buffer_data.assign(rawBytes, rawBytes + 5);

	uint8_t decoded[8]= {0};
	const size_t decodedSize= mikanRLEDecodeDMXUniverseBuffer(&universeData, sizeof(decoded), decoded);

	success&= (decodedSize == 5);
	for (size_t i= 0; i < 5; ++i)
		success&= (decoded[i] == rawBytes[i]);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Decode must never write past out_buffer_max_size, even mid-run.
bool dmx_rle_test_decode_clamps_to_max_size()
{
	UNIT_TEST_BEGIN("decode clamps output to out_buffer_max_size")

	// Encode 300 identical bytes, then decode into a buffer that only holds 10.
	std::vector<uint8_t> source(300, 42);
	MikanUniverseDMXData universeData;
	mikanRLEEncodeDMXUniverseBuffer(source.size(), source.data(), &universeData);

	uint8_t decoded[10]= {0};
	const size_t decodedSize= mikanRLEDecodeDMXUniverseBuffer(&universeData, sizeof(decoded), decoded);

	success&= (decodedSize == sizeof(decoded));
	for (size_t i= 0; i < sizeof(decoded); ++i)
		success&= (decoded[i] == 42);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Empty / null input must produce an empty buffer and decode to nothing.
bool dmx_rle_test_empty_input()
{
	UNIT_TEST_BEGIN("empty input encodes and decodes to nothing")

	MikanUniverseDMXData universeData;
	const size_t encodedSize= mikanRLEEncodeDMXUniverseBuffer(0, nullptr, &universeData);

	success&= (encodedSize == 0);
	success&= (universeData.buffer_data.size() == 0);

	uint8_t decoded[4]= {0};
	const size_t decodedSize= mikanRLEDecodeDMXUniverseBuffer(&universeData, sizeof(decoded), decoded);
	success&= (decodedSize == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool run_dmx_universe_rle_tests()
{
	UNIT_TEST_MODULE_BEGIN("dmx_universe_rle")
	UNIT_TEST_MODULE_CALL_TEST(dmx_rle_test_typical_universe_compresses);
	UNIT_TEST_MODULE_CALL_TEST(dmx_rle_test_long_run_splits);
	UNIT_TEST_MODULE_CALL_TEST(dmx_rle_test_incompressible_falls_back_to_raw);
	UNIT_TEST_MODULE_CALL_TEST(dmx_rle_test_decode_honors_raw_format);
	UNIT_TEST_MODULE_CALL_TEST(dmx_rle_test_decode_clamps_to_max_size);
	UNIT_TEST_MODULE_CALL_TEST(dmx_rle_test_empty_input);
	UNIT_TEST_MODULE_END()
}
