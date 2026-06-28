#include "MikanLightTypes.h"
#include "SerializationProperty.h"

#include <assert.h>

// RLE wire format: a flat sequence of [count, value] byte pairs, where count is the
// number of consecutive bytes (1..255) holding the given value. A run longer than 255
// is split across multiple pairs. This compresses DMX universes well since channel
// data is typically dominated by long runs of identical values (e.g. zeros).
static const size_t k_maxRunLength= 255;

// Run-Length-Encode(RLE) a source DMX channel buffer into a MikanUniverseDMXData
size_t mikanRLEEncodeDMXUniverseBuffer(const size_t in_buffer_size, const uint8_t* in_buffer,
									   MikanUniverseDMXData* outUniverseData)
{
	if (outUniverseData == nullptr)
		return 0;

	Serialization::List<uint8_t>& encoded= outUniverseData->buffer_data;
	encoded.clear();
	outUniverseData->buffer_format= MikanDMXBufferFormat::DMXRLEEncoded;

	if (in_buffer == nullptr || in_buffer_size == 0)
		return 0;

	size_t srcIndex= 0;
	while (srcIndex < in_buffer_size)
	{
		const uint8_t value= in_buffer[srcIndex];

		// Count how many consecutive bytes match, capped at the max run length.
		size_t runLength= 1;
		while (srcIndex + runLength < in_buffer_size && runLength < k_maxRunLength
			   && in_buffer[srcIndex + runLength] == value)
		{
			++runLength;
		}

		encoded.push_back(static_cast<uint8_t>(runLength));
		encoded.push_back(value);

		srcIndex+= runLength;
	}

	// Pathological inputs (e.g. no repeated values) make RLE larger than the source.
	// In that case store the raw bytes instead so the buffer never exceeds in_buffer_size.
	if (encoded.size() >= in_buffer_size)
	{
		encoded.assign(in_buffer, in_buffer + in_buffer_size);
		outUniverseData->buffer_format= MikanDMXBufferFormat::DMXUncompressed;
	}

	return encoded.size();
}

// Extract the raw channel values from a RLE encoded MikanUniverseDMXData
size_t mikanRLEDecodeDMXUniverseBuffer(const MikanUniverseDMXData* universeData, const size_t out_buffer_max_size,
									   uint8_t* out_buffer)
{
	if (universeData == nullptr || out_buffer == nullptr || out_buffer_max_size == 0)
		return 0;

	const Serialization::List<uint8_t>& encoded= universeData->buffer_data;

	// A raw buffer is simply copied through (clamped to the destination size).
	if (universeData->buffer_format == MikanDMXBufferFormat::DMXUncompressed)
	{
		const size_t copyCount= encoded.size() < out_buffer_max_size ? encoded.size() : out_buffer_max_size;
		for (size_t i= 0; i < copyCount; ++i)
			out_buffer[i]= encoded[i];

		return copyCount;
	}

	// Otherwise expand the [count, value] pairs into the destination buffer.
	size_t outIndex= 0;
	const size_t pairCount= encoded.size() / 2;
	for (size_t pair= 0; pair < pairCount && outIndex < out_buffer_max_size; ++pair)
	{
		const uint8_t runLength= encoded[pair * 2];
		const uint8_t value= encoded[pair * 2 + 1];

		for (uint8_t i= 0; i < runLength && outIndex < out_buffer_max_size; ++i)
			out_buffer[outIndex++]= value;
	}

	return outIndex;
}
