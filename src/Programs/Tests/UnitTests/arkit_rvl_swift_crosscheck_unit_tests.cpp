//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdint>
#include <vector>

#include "RVLCodec.h"
#include "unit_test.h"

// This is ticket A9 (RVL cross-check), done the only way available in this
// environment: there is no macOS/Xcode toolchain here to compile and run the real
// Swift encoder (MikanARStreamer/MikanARStreamer/RVLCodec.swift) directly, so
// SwiftPort below is a manual, line-by-line C++ transliteration of that file as it
// exists on disk, used to cross-check the real Mikan-side decoder
// (RVLCodec.h/.cpp) against the real iPhone-side encoder's actual control flow -
// not just the RVLCodec.h spec comment, which turned out to omit a corner case the
// real Swift implementation has (see below). A pass here means "the decoder
// tolerates what the Swift source, read carefully, actually produces" - it is not
// a substitute for eventually running both encoders against identical inputs on
// real hardware/toolchains and diffing bytes once Track A is buildable end-to-end.
namespace SwiftPort
{
std::vector<uint8_t> encodeNibbles(uint32_t value)
{
	std::vector<uint8_t> nibbles;
	uint32_t remaining= value;
	do
	{
		uint8_t nibble= static_cast<uint8_t>(remaining & 0x7);
		remaining>>= 3;
		if (remaining > 0)
			nibble|= 0x8;
		nibbles.push_back(nibble);
	} while (remaining > 0);
	return nibbles;
}

uint32_t zigzagEncode(int32_t value)
{
	if (value >= 0)
		return static_cast<uint32_t>(value) * 2;
	else
		return static_cast<uint32_t>(-value - 1) * 2 + 1;
}

std::vector<uint8_t> packNibbles(const std::vector<uint8_t>& nibbles)
{
	std::vector<uint8_t> bytes;
	size_t i= 0;
	while (i < nibbles.size())
	{
		const uint8_t high= nibbles[i];
		const uint8_t low= (i + 1 < nibbles.size()) ? nibbles[i + 1] : 0;
		bytes.push_back(static_cast<uint8_t>((high << 4) | (low & 0xF)));
		i+= 2;
	}
	return bytes;
}

// Transliterated from RVLCodec.swift:110-154 (rvlEncode). NOTE the `guard i <
// count else { break }` right after the zero-run is written (Swift line 126):
// if a zero-run consumes the array all the way to its end, the trailing
// nonzero-run(0) marker that RVLCodec.h's decoder-spec comment describes as
// unconditional is NOT written. RVLCodec.cpp's rvlDecode() turns out to still
// handle this correctly (it also stops early once expectedPixelCount samples
// have been produced, before attempting to read that marker) - but that
// tolerance wasn't a deliberate design choice made with this asymmetry in
// mind, so it's exercised explicitly by the tests below rather than assumed.
std::vector<uint8_t> rvlEncode(const std::vector<uint16_t>& depth)
{
	const size_t count= depth.size();
	if (count == 0)
		return {};

	std::vector<uint8_t> nibbles;
	size_t i= 0;

	while (i < count)
	{
		uint32_t zeroRun= 0;
		while (i < count && depth[i] == 0)
		{
			++zeroRun;
			++i;
		}
		const std::vector<uint8_t> zeroNibbles= encodeNibbles(zeroRun);
		nibbles.insert(nibbles.end(), zeroNibbles.begin(), zeroNibbles.end());

		if (!(i < count))
			break;

		uint32_t nonZeroRun= 0;
		while (i < count && depth[i] != 0)
		{
			++nonZeroRun;
			++i;
		}
		const std::vector<uint8_t> runNibbles= encodeNibbles(nonZeroRun);
		nibbles.insert(nibbles.end(), runNibbles.begin(), runNibbles.end());

		int32_t prevValue= 0;
		for (uint32_t j= 0; j < nonZeroRun; ++j)
		{
			const int32_t value= static_cast<int32_t>(depth[i - nonZeroRun + j]);
			const int32_t delta= (j == 0) ? value : (value - prevValue);
			prevValue= value;

			const std::vector<uint8_t> valueNibbles= encodeNibbles(zigzagEncode(delta));
			nibbles.insert(nibbles.end(), valueNibbles.begin(), valueNibbles.end());
		}
	}

	return packNibbles(nibbles);
}

// Transliterated from RVLCodec.swift:216-236 (packConfidenceRLE).
std::vector<uint8_t> packConfidenceRLE(const std::vector<uint8_t>& confidence)
{
	std::vector<uint8_t> result;
	size_t i= 0;
	while (i < confidence.size())
	{
		const uint8_t value= confidence[i];
		uint8_t runLength= 0;
		while (i < confidence.size() && runLength < 255 && confidence[i] == value)
		{
			++runLength;
			++i;
		}
		result.push_back(value);
		result.push_back(runLength);
	}
	return result;
}
} // namespace SwiftPort

namespace
{
std::vector<uint16_t> makeAlternating(size_t count)
{
	std::vector<uint16_t> depth(count);
	for (size_t i= 0; i < count; ++i)
		depth[i]= (i % 2 == 0) ? 0 : static_cast<uint16_t>(i & 0xFFFF);
	return depth;
}

std::vector<uint16_t> makeMonotonic(size_t count)
{
	std::vector<uint16_t> depth(count);
	for (size_t i= 0; i < count; ++i)
		depth[i]= static_cast<uint16_t>(i < 65535 ? i : 65535);
	return depth;
}

std::vector<uint16_t> makeRealistic(size_t count, unsigned int seed)
{
	srand(seed);
	std::vector<uint16_t> depth(count);
	for (size_t i= 0; i < count; ++i)
	{
		if (i % 100 < 20)
		{
			depth[i]= 0;
		}
		else
		{
			const uint16_t base= static_cast<uint16_t>(500 + (i % 9500));
			const uint16_t variation= static_cast<uint16_t>(rand() % 50);
			const int sum= static_cast<int>(base) + static_cast<int>(variation);
			depth[i]= static_cast<uint16_t>(sum > 65535 ? 65535 : sum);
		}
	}
	return depth;
}
} // namespace

//-- private functions -----
static bool arkit_rvl_crosscheck_test_swift_port_matches_own_golden_vectors()
{
	UNIT_TEST_BEGIN("SwiftPort::rvlEncode matches the hand-derived golden vectors")

	// Sanity check the transliteration itself before trusting it as an oracle:
	// these are the same two vectors hand-derived in arkit_rvl_codec_unit_tests.cpp.
	{
		const std::vector<uint16_t> depth= {0, 0, 0, 0};
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		const std::vector<uint8_t> expected= {0x40};
		success= (encoded == expected);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_crosscheck_test_zero_run_reaches_array_end()
{
	UNIT_TEST_BEGIN("Mikan decoder handles Swift's omitted trailing marker (zero-run at array end)")

	// This is precisely the case where Swift's encoder omits the trailing
	// nonzero-run(0) marker that RVLCodec.h's spec comment describes as always
	// present. [0,5,3,0] ends on a single trailing zero.
	const std::vector<uint16_t> depth= {0, 5, 3, 0};
	std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);

	std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(depth.size()));
	success= (decoded == depth);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_crosscheck_test_nonzero_run_reaches_array_end()
{
	UNIT_TEST_BEGIN("Mikan decoder handles array ending on a nonzero run")

	const std::vector<uint16_t> depth= {0, 5, 3};
	std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);

	std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(depth.size()));
	success= (decoded == depth);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_crosscheck_test_swift_boundary_value_vectors()
{
	UNIT_TEST_BEGIN("Mikan decoder matches Swift RVLCodecTests boundary-value cases")

	// Mirrors RVLCodecTests.swift testRvlEncode_decode_boundaryValues and
	// testRvlEncode_decode_twoValueRun exactly (same inputs).
	{
		const std::vector<uint16_t> depth= {0, 1, 2, 1000, 65535, 0, 0};
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(depth.size()));
		success= (decoded == depth);
		assert(success);
	}
	{
		const std::vector<uint16_t> depth= {100, 65435, 0};
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(depth.size()));
		success= success && (decoded == depth);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_crosscheck_test_swift_fullsize_scenarios()
{
	UNIT_TEST_BEGIN("Mikan decoder matches Swift RVLCodecTests full-resolution scenarios")

	const size_t kPixelCount= 256 * 192;
	success= true;

	// Mirrors testRvlRoundTrip_allZeros / allMax / alternating / monotonic /
	// realisticDepth from RVLCodecTests.swift.
	{
		std::vector<uint16_t> depth(kPixelCount, 0);
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == depth);
		assert(success);
	}
	{
		std::vector<uint16_t> depth(kPixelCount, 0xFFFF);
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == depth);
		assert(success);
	}
	{
		std::vector<uint16_t> depth= makeAlternating(kPixelCount);
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == depth);
		assert(success);
	}
	{
		std::vector<uint16_t> depth= makeMonotonic(kPixelCount);
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == depth);
		assert(success);
	}
	for (unsigned int seed= 1; success && seed <= 4; ++seed)
	{
		std::vector<uint16_t> depth= makeRealistic(kPixelCount, seed);
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= (decoded == depth);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_crosscheck_test_single_element_arrays()
{
	UNIT_TEST_BEGIN("Mikan decoder matches Swift single-element edge cases")

	{
		const std::vector<uint16_t> depth= {0};
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), 1);
		success= (decoded == depth);
		assert(success);
	}
	{
		const std::vector<uint16_t> depth= {5000};
		std::vector<uint8_t> encoded= SwiftPort::rvlEncode(depth);
		std::vector<uint16_t> decoded= rvlDecode(encoded.data(), encoded.size(), 1);
		success= success && (decoded == depth);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_rvl_crosscheck_test_confidence_rle_matches_swift()
{
	UNIT_TEST_BEGIN("Mikan confidence RLE decoder matches Swift packConfidenceRLE")

	const size_t kPixelCount= 256 * 192;

	// Mirrors testConfidenceRLE_roundTrip_alternating and _longRun.
	{
		std::vector<uint8_t> confidence(kPixelCount);
		for (size_t i= 0; i < kPixelCount; ++i)
			confidence[i]= static_cast<uint8_t>(i % 3);

		std::vector<uint8_t> encoded= SwiftPort::packConfidenceRLE(confidence);
		std::vector<uint8_t> decoded=
			packConfidenceRLEDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= (decoded == confidence);
		assert(success);
	}
	{
		std::vector<uint8_t> confidence(500, 1);
		std::vector<uint8_t> encoded= SwiftPort::packConfidenceRLE(confidence);
		// Swift's test asserts this specific run splits into >1 byte-pair.
		success= success && (encoded.size() > 2);
		assert(success);

		std::vector<uint8_t> decoded= packConfidenceRLEDecode(encoded.data(), encoded.size(), 500);
		success= success && (decoded == confidence);
		assert(success);
	}
	{
		// Mirrors testConfidenceRLE_compressionRatio: 49152 uniform values -> 193
		// byte-pairs (386 bytes): 49152 / 255 = 192 remainder 252 -> 193 pairs.
		std::vector<uint8_t> confidence(kPixelCount, 2);
		std::vector<uint8_t> encoded= SwiftPort::packConfidenceRLE(confidence);
		success= success && (encoded.size() == 386);
		assert(success);

		std::vector<uint8_t> decoded=
			packConfidenceRLEDecode(encoded.data(), encoded.size(), static_cast<int>(kPixelCount));
		success= success && (decoded == confidence);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_rvl_swift_crosscheck_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_rvl_swift_crosscheck")
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_swift_port_matches_own_golden_vectors);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_zero_run_reaches_array_end);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_nonzero_run_reaches_array_end);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_swift_boundary_value_vectors);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_swift_fullsize_scenarios);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_single_element_arrays);
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_crosscheck_test_confidence_rle_matches_swift);
	UNIT_TEST_MODULE_END()
}
