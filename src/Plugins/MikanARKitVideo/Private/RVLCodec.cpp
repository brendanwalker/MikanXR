#include "RVLCodec.h"

namespace
{
// Reads 4-bit nibbles (high nibble of each byte first) and VLE-encoded unsigned
// integers from a bounded byte buffer. Never reads past m_data[0, m_length).
class NibbleReader
{
public:
	NibbleReader(const uint8_t* data, size_t length)
		: m_data(data)
		, m_length(length)
		, m_nibbleIndex(0)
	{
	}

	bool readNibble(uint8_t& outNibble)
	{
		const size_t byteIndex= m_nibbleIndex / 2;
		if (byteIndex >= m_length)
			return false;

		const uint8_t byte= m_data[byteIndex];
		outNibble= (m_nibbleIndex % 2 == 0) ? static_cast<uint8_t>(byte >> 4) : static_cast<uint8_t>(byte & 0x0F);
		++m_nibbleIndex;
		return true;
	}

	// Returns false if the stream runs out of nibbles before a terminating
	// (non-continuation) nibble, or if a corrupted stream never terminates within
	// a plausible value width (guards against shifting UB / spinning forever).
	bool readVLE(uint32_t& outValue)
	{
		outValue= 0;
		int shift= 0;
		uint8_t nibble= 0;
		bool more;
		do
		{
			if (shift >= 32 || !readNibble(nibble))
				return false;

			outValue|= static_cast<uint32_t>(nibble & 0x7) << shift;
			shift+= 3;
			more= (nibble & 0x8) != 0;
		} while (more);

		return true;
	}

private:
	const uint8_t* m_data;
	size_t m_length;
	size_t m_nibbleIndex;
};

int32_t unzigzag(uint32_t value) { return static_cast<int32_t>(value >> 1) ^ -static_cast<int32_t>(value & 1); }
} // namespace

std::vector<uint16_t> rvlDecode(const uint8_t* data, size_t length, int expectedPixelCount)
{
	if (data == nullptr || length == 0 || expectedPixelCount <= 0)
		return {};

	std::vector<uint16_t> output;
	output.reserve(static_cast<size_t>(expectedPixelCount));

	NibbleReader reader(data, length);

	while (static_cast<int>(output.size()) < expectedPixelCount)
	{
		const uint32_t remainingBudget= static_cast<uint32_t>(expectedPixelCount - static_cast<int>(output.size()));

		uint32_t zeroRun= 0;
		if (!reader.readVLE(zeroRun) || zeroRun > remainingBudget)
			return {};

		for (uint32_t i= 0; i < zeroRun; ++i)
			output.push_back(0);

		if (static_cast<int>(output.size()) >= expectedPixelCount)
			break;

		const uint32_t remainingAfterZeros= static_cast<uint32_t>(expectedPixelCount - static_cast<int>(output.size()));

		uint32_t nonzeroRun= 0;
		if (!reader.readVLE(nonzeroRun) || nonzeroRun > remainingAfterZeros)
			return {};

		// previous resets to 0 at the start of every nonzero run (not carried across
		// zero-run gaps) - each run's first value is its own zigzag(value), not a
		// delta from whatever nonzero value preceded the last zero-run.
		int32_t previous= 0;
		for (uint32_t i= 0; i < nonzeroRun; ++i)
		{
			uint32_t zigzag= 0;
			if (!reader.readVLE(zigzag))
				return {};

			const int32_t delta= unzigzag(zigzag);
			const int32_t current= previous + delta;
			if (current < 0 || current > 0xFFFF)
				return {};

			output.push_back(static_cast<uint16_t>(current));
			previous= current;
		}
	}

	if (static_cast<int>(output.size()) != expectedPixelCount)
		return {};

	return output;
}

std::vector<uint8_t> packConfidenceRLEDecode(const uint8_t* data, size_t length, int expectedPixelCount)
{
	if (data == nullptr || expectedPixelCount <= 0)
		return {};

	std::vector<uint8_t> output;
	output.reserve(static_cast<size_t>(expectedPixelCount));

	size_t offset= 0;
	while (static_cast<int>(output.size()) < expectedPixelCount)
	{
		if (offset + 2 > length)
			return {};

		const uint8_t value= data[offset];
		const uint8_t runLength= data[offset + 1];
		offset+= 2;

		if (runLength == 0 || static_cast<int>(output.size()) + runLength > expectedPixelCount)
			return {};

		for (uint8_t i= 0; i < runLength; ++i)
			output.push_back(value);
	}

	return output;
}
