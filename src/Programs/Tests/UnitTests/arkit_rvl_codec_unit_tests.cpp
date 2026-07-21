//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>
#include <vector>

#include "RVLCodec.h"
#include "unit_test.h"

//-- reference (test-only) encoder -----
// Mirrors the exact bitstream layout documented in RVLCodec.h so we can round-trip
// test the decoder against many generated inputs, in addition to the hand-derived
// golden byte vectors below (which are the primary correctness signal, since they
// don't depend on this encoder also being correct). This encoder is not shipped -
// Mikan only ever decodes; the iPhone app is the real encoder.
namespace
{
class NibbleWriter
{
public:
	explicit NibbleWriter(std::vector<uint8_t>& buffer)
		: m_out(buffer)
		, m_pendingHighNibble(-1)
	{
	}

	void writeNibble(uint8_t nibble)
	{
		if (m_pendingHighNibble < 0)
		{
			m_pendingHighNibble= nibble & 0x0F;
		}
		else
		{
			m_out.push_back(static_cast<uint8_t>((m_pendingHighNibble << 4) | (nibble & 0x0F)));
			m_pendingHighNibble= -1;
		}
	}

	void writeVLE(uint32_t value)
	{
		do
		{
			uint8_t nibble= static_cast<uint8_t>(value & 0x7);
			value>>= 3;
			if (value != 0)
				nibble|= 0x8;
			writeNibble(nibble);
		} while (value != 0);
	}

	void flush()
	{
		if (m_pendingHighNibble >= 0)
		{
			m_out.push_back(static_cast<uint8_t>(m_pendingHighNibble << 4));
			m_pendingHighNibble= -1;
		}
	}

private:
	std::vector<uint8_t>& m_out;
	int m_pendingHighNibble;
};

uint32_t zigzagEncode(int32_t value)
{
	return (static_cast<uint32_t>(value) << 1) ^ static_cast<uint32_t>(value >> 31);
}

std::vector<uint8_t> referenceRvlEncode(const std::vector<uint16_t>& depth)
{
	std::vector<uint8_t> out;
	NibbleWriter writer(out);

	size_t i= 0;
	while (i < depth.size())
	{
		const size_t zeroStart= i;
		while (i < depth.size() && depth[i] == 0)
			++i;
		writer.writeVLE(static_cast<uint32_t>(i - zeroStart));

		const size_t nonzeroStart= i;
		while (i < depth.size() && depth[i] != 0)
			++i;
		writer.writeVLE(static_cast<uint32_t>(i - nonzeroStart));

		// previous resets to 0 at the start of every nonzero run - see the matching
		// note in RVLCodec.h. Sharing this reset scope with rvlDecode() is what this
		// reference encoder is for: an earlier version kept `previous` across the
		// whole array (matching a bug that was also in rvlDecode() at the time), so
		// this round-trip test passed while being wrong - caught only by cross-
		// checking against the real iPhone Swift encoder (see
		// arkit_rvl_swift_crosscheck_unit_tests.cpp).
		int32_t previous= 0;
		for (size_t j= nonzeroStart; j < i; ++j)
		{
			const int32_t delta= static_cast<int32_t>(depth[j]) - previous;
			writer.writeVLE(zigzagEncode(delta));
			previous= static_cast<int32_t>(depth[j]);
		}
	}

	writer.flush();
	return out;
}

std::vector<uint8_t> referenceConfidenceRLEEncode(const std::vector<uint8_t>& confidence)
{
	std::vector<uint8_t> out;
	size_t i= 0;
	while (i < confidence.size())
	{
		const uint8_t value= confidence[i];
		const size_t runStart= i;
		while (i < confidence.size() && confidence[i] == value && (i - runStart) < 255)
			++i;
		out.push_back(value);
		out.push_back(static_cast<uint8_t>(i - runStart));
	}
	return out;
}

std::vector<uint16_t> makeRandomDepth(size_t count, unsigned int seed)
{
	srand(seed);
	std::vector<uint16_t> depth(count);
	for (size_t i= 0; i < count; ++i)
	{
		// Bias toward zero to mimic realistic clustered-invalid-region depth data.
		depth[i]= (rand() % 4 == 0) ? static_cast<uint16_t>(rand() % 6000) : 0;
	}
	return depth;
}

std::vector<uint8_t> makeRandomConfidence(size_t count, unsigned int seed)
{
	srand(seed);
	std::vector<uint8_t> confidence(count);
	for (size_t i= 0; i < count; ++i)
		confidence[i]= static_cast<uint8_t>(rand() % 3);
	return confidence;
}
} // namespace

//-- private functions -----
static bool arkit_rvl_test_decode_golden_all_zero()
{
	UNIT_TEST_BEGIN("rvl decode golden vector: all-zero")

	// Hand-derived from the spec in RVLCodec.h: 4 zero pixels -> VLE(4), VLE(0) ->
	// nibbles [0x4, 0x0] -> single byte 0x40.
	const uint8_t compressed[]= {0x40};
	std::vector<uint16_t> result= rvlDecode(compressed, sizeof(compressed), 4);

	success= (result.size() == 4);
	assert(success);
	for (int i= 0; success && i < 4; ++i)
		success= (result[i] == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_decode_golden_mixed()
{
	UNIT_TEST_BEGIN("rvl decode golden vector: mixed zero/nonzero run")

	// Hand-derived for input [0, 5, 3, 0]: zeroRun=1, nonzeroRun=2, zigzag(5-0)=10,
	// zigzag(3-5)=3, zeroRun=1, nonzeroRun=0 -> nibbles [1,2,A,1,3,1,0,pad0] ->
	// bytes [0x12, 0xA1, 0x31, 0x00].
	const uint8_t compressed[]= {0x12, 0xA1, 0x31, 0x00};
	std::vector<uint16_t> result= rvlDecode(compressed, sizeof(compressed), 4);

	success= (result.size() == 4);
	assert(success);
	success= success && result[0] == 0 && result[1] == 5 && result[2] == 3 && result[3] == 0;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_decode_roundtrip_random()
{
	UNIT_TEST_BEGIN("rvl decode roundtrip against reference encoder")

	const size_t kPixelCount= 256 * 192;
	success= true;

	for (unsigned int seed= 1; success && seed <= 8; ++seed)
	{
		std::vector<uint16_t> original= makeRandomDepth(kPixelCount, seed);
		std::vector<uint8_t> compressed= referenceRvlEncode(original);

		std::vector<uint16_t> decoded= rvlDecode(compressed.data(), compressed.size(), static_cast<int>(kPixelCount));

		success= (decoded.size() == original.size());
		assert(success);
		for (size_t i= 0; success && i < original.size(); ++i)
			success= (decoded[i] == original[i]);
		assert(success);
	}

	// Degenerate all-zero and all-nonzero (worst case for run-length assumptions)
	// full-resolution frames.
	{
		std::vector<uint16_t> allZero(kPixelCount, 0);
		std::vector<uint8_t> compressed= referenceRvlEncode(allZero);
		std::vector<uint16_t> decoded= rvlDecode(compressed.data(), compressed.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == allZero);
		assert(success);
	}
	{
		std::vector<uint16_t> allMax(kPixelCount, 0xFFFE);
		std::vector<uint8_t> compressed= referenceRvlEncode(allMax);
		std::vector<uint16_t> decoded= rvlDecode(compressed.data(), compressed.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == allMax);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_decode_rejects_invalid_input()
{
	UNIT_TEST_BEGIN("rvl decode rejects invalid/degenerate input")

	success= (rvlDecode(nullptr, 0, 4).empty());
	assert(success);

	const uint8_t nonEmpty[]= {0x40};
	success= success && rvlDecode(nonEmpty, sizeof(nonEmpty), 0).empty();
	assert(success);
	success= success && rvlDecode(nonEmpty, sizeof(nonEmpty), -1).empty();
	assert(success);

	// A stream too short to have produced expectedPixelCount samples.
	std::vector<uint16_t> original= makeRandomDepth(256 * 192, 42);
	std::vector<uint8_t> compressed= referenceRvlEncode(original);
	success= success && rvlDecode(compressed.data(), compressed.size() / 2, 256 * 192).empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_decode_fuzz_no_crash()
{
	UNIT_TEST_BEGIN("rvl decode fuzz: truncation and bit-flips never crash")

	std::vector<uint16_t> original= makeRandomDepth(256 * 192, 7);
	std::vector<uint8_t> compressed= referenceRvlEncode(original);

	success= true;

	// Truncate to every length from 0 up to the full buffer; decoder must either
	// fail gracefully (empty result) or - if it happens to still succeed - produce
	// a result of exactly the requested pixel count (never a partial/garbage one).
	for (size_t len= 0; success && len <= compressed.size(); len+= (compressed.size() / 64) + 1)
	{
		std::vector<uint16_t> result= rvlDecode(compressed.data(), len, 256 * 192);
		success= result.empty() || result.size() == static_cast<size_t>(256 * 192);
	}
	assert(success);

	// Bit-flip fuzzing: corrupt random bytes of a copy of the valid stream and
	// confirm decode never crashes/hangs and always either fails gracefully or
	// returns a fully-sized result.
	srand(1234);
	for (int trial= 0; success && trial < 200; ++trial)
	{
		std::vector<uint8_t> corrupted= compressed;
		const int numFlips= 1 + (rand() % 4);
		for (int f= 0; f < numFlips; ++f)
		{
			if (corrupted.empty())
				break;
			const size_t idx= static_cast<size_t>(rand()) % corrupted.size();
			corrupted[idx]^= static_cast<uint8_t>(1 << (rand() % 8));
		}

		std::vector<uint16_t> result= rvlDecode(corrupted.data(), corrupted.size(), 256 * 192);
		success= result.empty() || result.size() == static_cast<size_t>(256 * 192);
	}
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_confidence_rle_golden()
{
	UNIT_TEST_BEGIN("confidence RLE decode golden vector")

	// [2,2,2,0,0,1] -> pairs (2,3) (0,2) (1,1) -> bytes [0x02,0x03, 0x00,0x02, 0x01,0x01]
	const uint8_t compressed[]= {0x02, 0x03, 0x00, 0x02, 0x01, 0x01};
	std::vector<uint8_t> result= packConfidenceRLEDecode(compressed, sizeof(compressed), 6);

	const uint8_t expected[]= {2, 2, 2, 0, 0, 1};
	success= (result.size() == 6);
	assert(success);
	for (int i= 0; success && i < 6; ++i)
		success= (result[i] == expected[i]);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_confidence_rle_roundtrip_random()
{
	UNIT_TEST_BEGIN("confidence RLE decode roundtrip against reference encoder")

	const size_t kPixelCount= 256 * 192;
	success= true;

	for (unsigned int seed= 1; success && seed <= 4; ++seed)
	{
		std::vector<uint8_t> original= makeRandomConfidence(kPixelCount, seed);
		std::vector<uint8_t> compressed= referenceConfidenceRLEEncode(original);

		std::vector<uint8_t> decoded=
			packConfidenceRLEDecode(compressed.data(), compressed.size(), static_cast<int>(kPixelCount));

		success= (decoded == original);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_test_confidence_rle_rejects_invalid_input()
{
	UNIT_TEST_BEGIN("confidence RLE decode rejects invalid/malformed input")

	success= packConfidenceRLEDecode(nullptr, 0, 4).empty();
	assert(success);

	// Truncated mid-pair (odd trailing byte with no runLength).
	const uint8_t truncated[]= {0x02};
	success= success && packConfidenceRLEDecode(truncated, sizeof(truncated), 4).empty();
	assert(success);

	// runLength == 0 is not a valid encoding (encoder never emits it).
	const uint8_t zeroRun[]= {0x02, 0x00};
	success= success && packConfidenceRLEDecode(zeroRun, sizeof(zeroRun), 1).empty();
	assert(success);

	// Declared runs overshoot expectedPixelCount.
	const uint8_t overshoot[]= {0x02, 0x05};
	success= success && packConfidenceRLEDecode(overshoot, sizeof(overshoot), 3).empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_rvl_codec_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_rvl_codec")
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_decode_golden_all_zero);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_decode_golden_mixed);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_decode_roundtrip_random);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_decode_rejects_invalid_input);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_decode_fuzz_no_crash);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_confidence_rle_golden);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_confidence_rle_roundtrip_random);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_test_confidence_rle_rejects_invalid_input);
	UNIT_TEST_MODULE_END()
}
