//-- includes -----
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

#include "unit_test.h"

#include "ARKitMatteReceiver.h"
#include "ARKitWireProtocol.h"

namespace
{
// Reference (value, runLength) RLE encoder matching the iPhone's packConfidenceRLE (the
// codec the matte channel reuses) - runLength capped at 255, longer runs split.
std::vector<uint8_t> matteTestRLEEncode(const std::vector<uint8_t>& matte)
{
	std::vector<uint8_t> out;
	size_t i= 0;
	while (i < matte.size())
	{
		const uint8_t value= matte[i];
		size_t run= 0;
		while (i < matte.size() && matte[i] == value && run < 255)
		{
			++i;
			++run;
		}
		out.push_back(value);
		out.push_back(static_cast<uint8_t>(run));
	}
	return out;
}

// Builds the matte payload [uint32 width BE][uint32 height BE][uint32 rleLen BE][rle],
// matching MattePacketFramer.swift / ARKitWireProtocol.h.
std::vector<uint8_t> buildMattePayload(int width, int height, const std::vector<uint8_t>& matte)
{
	std::vector<uint8_t> rle= matteTestRLEEncode(matte);

	std::vector<uint8_t> payload;
	payload.resize(12);
	writeU32BE(payload.data() + 0, static_cast<uint32_t>(width));
	writeU32BE(payload.data() + 4, static_cast<uint32_t>(height));
	writeU32BE(payload.data() + 8, static_cast<uint32_t>(rle.size()));
	payload.insert(payload.end(), rle.begin(), rle.end());
	return payload;
}

// Splits a matte payload into wire fragments tagged with the matte header (version 1,
// type 3), matching MattePacketFramer's fragmentation.
std::vector<std::vector<uint8_t>> buildMatteFragments(uint32_t frameSeq, uint64_t timestampUs, int width, int height,
													  const std::vector<uint8_t>& matte,
													  size_t maxPayloadPerFragment= 1200)
{
	std::vector<uint8_t> payload= buildMattePayload(width, height, matte);

	const size_t totalChunks= std::max<size_t>(1, (payload.size() + maxPayloadPerFragment - 1) / maxPayloadPerFragment);

	std::vector<std::vector<uint8_t>> fragments;
	size_t offset= 0;
	uint16_t fragIndex= 0;
	while (offset < payload.size() || fragments.empty())
	{
		const size_t end= std::min(offset + maxPayloadPerFragment, payload.size());

		ARKitDepthFragmentHeader header; // generic 20-byte fragment header
		header.version= kARKitMatteWireVersion;
		header.type= eARKitPacketType::Matte;
		header.frameSeq= frameSeq;
		header.captureTimestampUs= timestampUs;
		header.fragIndex= fragIndex;
		header.fragCount= static_cast<uint16_t>(totalChunks);

		std::vector<uint8_t> datagram(ARKitDepthFragmentHeader::kWireSize + (end - offset));
		writeARKitDepthFragmentHeader(datagram.data(), datagram.size(), header);
		std::copy(payload.begin() + static_cast<ptrdiff_t>(offset), payload.begin() + static_cast<ptrdiff_t>(end),
				  datagram.begin() + static_cast<ptrdiff_t>(ARKitDepthFragmentHeader::kWireSize));

		fragments.push_back(std::move(datagram));

		offset= end;
		++fragIndex;
	}

	return fragments;
}

// A silhouette stencil (left half 0/background, right half 1/person) at arbitrary dims -
// realistic and RLE-friendly with a real boundary.
std::vector<uint8_t> makeSilhouette(int width, int height)
{
	std::vector<uint8_t> matte(static_cast<size_t>(width) * height);
	for (int y= 0; y < height; ++y)
		for (int x= 0; x < width; ++x)
			matte[y * width + x]= static_cast<uint8_t>(x >= width / 2 ? 1 : 0);
	return matte;
}
} // namespace

//-- private functions -----
static bool arkit_matte_receiver_test_native_res_roundtrips()
{
	UNIT_TEST_BEGIN("a native-resolution matte reassembles with correct dims and labels")

	// A resolution larger than the 256x192 depth grid, spanning multiple fragments.
	const int width= 640, height= 480;
	std::vector<uint8_t> matte= makeSilhouette(width, height);
	std::vector<std::vector<uint8_t>> fragments= buildMatteFragments(42, 1234, width, height, matte);
	success= (fragments.size() > 1);
	assert(success);

	ARKitMatteFragmentAssembler assembler;
	int callbackCount= 0;
	ARKitMatteFrame received;
	assembler.setFrameCallback(
		[&](ARKitMatteFrame frame)
		{
			++callbackCount;
			received= std::move(frame);
		});

	const auto now= std::chrono::steady_clock::now();
	for (const auto& fragment : fragments)
	{
		success= success && assembler.processFragment(fragment.data(), fragment.size(), now);
		assert(success);
	}

	success= success && (callbackCount == 1);
	assert(success);
	success= success && (received.frameSeq == 42 && received.captureTimestampUs == 1234);
	assert(success);
	success= success && (received.width == width && received.height == height);
	assert(success);
	success= success && (received.matte == matte);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_matte_receiver_test_rejects_wrong_type_and_version()
{
	UNIT_TEST_BEGIN("matte assembler rejects a fragment whose header isn't a matte (v1/type3) header")

	ARKitMatteFragmentAssembler assembler;
	const auto now= std::chrono::steady_clock::now();

	// Too short for a header.
	const uint8_t tooShort[]= {0x01, 0x02, 0x03};
	success= (assembler.processFragment(tooShort, sizeof(tooShort), now) == false);
	assert(success);

	// A well-formed DEPTH fragment header (type 1, version 2) fed to the matte assembler.
	std::vector<uint8_t> matte= makeSilhouette(8, 8);
	std::vector<uint8_t> payload= buildMattePayload(8, 8, matte);
	ARKitDepthFragmentHeader depthHeader;
	depthHeader.version= kARKitDepthWireVersion;
	depthHeader.type= eARKitPacketType::Depth;
	depthHeader.frameSeq= 1;
	depthHeader.fragIndex= 0;
	depthHeader.fragCount= 1;
	std::vector<uint8_t> datagram(ARKitDepthFragmentHeader::kWireSize + payload.size());
	writeARKitDepthFragmentHeader(datagram.data(), datagram.size(), depthHeader);
	std::copy(payload.begin(), payload.end(), datagram.begin() + ARKitDepthFragmentHeader::kWireSize);
	success= success && (assembler.processFragment(datagram.data(), datagram.size(), now) == false);
	assert(success);

	success= success && (assembler.getPendingFrameCount() == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_matte_receiver_test_rejects_implausible_dimensions()
{
	UNIT_TEST_BEGIN("matte assembler drops a frame with zero/oversized dimensions without crashing")

	ARKitMatteFragmentAssembler assembler;
	int callbackCount= 0;
	assembler.setFrameCallback([&](ARKitMatteFrame) { ++callbackCount; });
	const auto now= std::chrono::steady_clock::now();

	// A single well-formed matte fragment, but with width forced to an absurd value that
	// exceeds kARKitMatteMaxDimension - the assembler must drop it, not allocate for it.
	std::vector<uint8_t> matte= makeSilhouette(8, 8);
	std::vector<uint8_t> payload= buildMattePayload(8, 8, matte);
	writeU32BE(payload.data() + 0, 999999u); // corrupt width field

	ARKitDepthFragmentHeader header;
	header.version= kARKitMatteWireVersion;
	header.type= eARKitPacketType::Matte;
	header.frameSeq= 7;
	header.fragIndex= 0;
	header.fragCount= 1;
	std::vector<uint8_t> datagram(ARKitDepthFragmentHeader::kWireSize + payload.size());
	writeARKitDepthFragmentHeader(datagram.data(), datagram.size(), header);
	std::copy(payload.begin(), payload.end(), datagram.begin() + ARKitDepthFragmentHeader::kWireSize);

	// The fragment itself is well-formed (accepted), but completing the frame fails the
	// dimension bounds check -> no callback, counted as malformed.
	success= assembler.processFragment(datagram.data(), datagram.size(), now);
	assert(success);
	success= success && (callbackCount == 0);
	assert(success);
	success= success && (assembler.getDroppedMalformedFrameCount() == 1);
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_matte_receiver_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_matte_receiver")
	UNIT_TEST_MODULE_CALL_TEST(arkit_matte_receiver_test_native_res_roundtrips);
	UNIT_TEST_MODULE_CALL_TEST(arkit_matte_receiver_test_rejects_wrong_type_and_version);
	UNIT_TEST_MODULE_CALL_TEST(arkit_matte_receiver_test_rejects_implausible_dimensions);
	UNIT_TEST_MODULE_END()
}
