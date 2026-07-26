//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "ARKitDepthReceiver.h"
#include "ARKitWireProtocol.h"
#include "unit_test.h"

// Reference RVL/RLE encoders + fragmentation, used only to build synthetic wire
// payloads to feed ARKitDepthFragmentAssembler directly (bypassing sockets/threads
// entirely, per this ticket's verification requirement). Same corrected algorithm
// as arkit_rvl_codec_unit_tests.cpp / arkit_rvl_swift_crosscheck_unit_tests.cpp
// (previous value resets per nonzero run) - duplicated locally rather than shared,
// matching this codebase's per-file-self-contained unit test convention.
namespace
{
class DepthTestNibbleWriter
{
public:
	explicit DepthTestNibbleWriter(std::vector<uint8_t>& buffer)
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

uint32_t depthTestZigzagEncode(int32_t value)
{
	return (static_cast<uint32_t>(value) << 1) ^ static_cast<uint32_t>(value >> 31);
}

std::vector<uint8_t> depthTestReferenceRvlEncode(const std::vector<uint16_t>& depth)
{
	std::vector<uint8_t> out;
	DepthTestNibbleWriter writer(out);

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

		int32_t previous= 0;
		for (size_t j= nonzeroStart; j < i; ++j)
		{
			const int32_t delta= static_cast<int32_t>(depth[j]) - previous;
			writer.writeVLE(depthTestZigzagEncode(delta));
			previous= static_cast<int32_t>(depth[j]);
		}
	}

	writer.flush();
	return out;
}

std::vector<uint8_t> depthTestReferenceConfidenceRLEEncode(const std::vector<uint8_t>& confidence)
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

// Builds [uint32 rvlLen BE][rvlBytes][uint32 confLen BE][confBytes], matching
// DepthPacketFramer.swift's buildPayload / the Wire Protocol Reference.
std::vector<uint8_t> buildDepthPayload(const std::vector<uint16_t>& depth, const std::vector<uint8_t>& confidence)
{
	std::vector<uint8_t> rvlBytes= depthTestReferenceRvlEncode(depth);
	std::vector<uint8_t> confBytes= depthTestReferenceConfidenceRLEEncode(confidence);

	std::vector<uint8_t> payload;
	payload.resize(4);
	writeU32BE(payload.data(), static_cast<uint32_t>(rvlBytes.size()));
	payload.insert(payload.end(), rvlBytes.begin(), rvlBytes.end());

	const size_t confLenOffset= payload.size();
	payload.resize(payload.size() + 4);
	writeU32BE(payload.data() + confLenOffset, static_cast<uint32_t>(confBytes.size()));
	payload.insert(payload.end(), confBytes.begin(), confBytes.end());

	return payload;
}

// Splits a depth payload into wire-format UDP fragments (20-byte header + up to
// maxPayloadPerFragment bytes), matching DepthPacketFramer.swift's
// fragmentPayload. Returns each fragment as a complete raw datagram buffer.
std::vector<std::vector<uint8_t>> buildDepthFragments(uint32_t frameSeq, uint64_t timestampUs,
													  const std::vector<uint16_t>& depth,
													  const std::vector<uint8_t>& confidence,
													  size_t maxPayloadPerFragment= 1200)
{
	std::vector<uint8_t> payload= buildDepthPayload(depth, confidence);

	const size_t totalChunks= std::max<size_t>(1, (payload.size() + maxPayloadPerFragment - 1) / maxPayloadPerFragment);

	std::vector<std::vector<uint8_t>> fragments;
	size_t offset= 0;
	uint16_t fragIndex= 0;
	while (offset < payload.size() || fragments.empty())
	{
		const size_t end= std::min(offset + maxPayloadPerFragment, payload.size());

		ARKitDepthFragmentHeader header;
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

// v2 payload: appends [uint32 segLen BE][seg RLE bytes] (person matte) after the
// confidence section, matching DepthPacketFramer.swift's v2 buildPayload. The matte is
// RLE-encoded with the same codec as confidence (0/1 stencil).
std::vector<uint8_t> buildDepthPayloadV2(const std::vector<uint16_t>& depth, const std::vector<uint8_t>& confidence,
										 const std::vector<uint8_t>& matte)
{
	std::vector<uint8_t> payload= buildDepthPayload(depth, confidence);
	std::vector<uint8_t> segBytes= depthTestReferenceConfidenceRLEEncode(matte);

	const size_t segLenOffset= payload.size();
	payload.resize(payload.size() + 4);
	writeU32BE(payload.data() + segLenOffset, static_cast<uint32_t>(segBytes.size()));
	payload.insert(payload.end(), segBytes.begin(), segBytes.end());

	return payload;
}

// Splits a v2 payload into wire fragments tagged with kARKitDepthWireVersion (2). Same
// fragmentation as buildDepthFragments, but for the matte-carrying v2 payload/version.
std::vector<std::vector<uint8_t>> buildDepthFragmentsV2(uint32_t frameSeq, uint64_t timestampUs,
														const std::vector<uint16_t>& depth,
														const std::vector<uint8_t>& confidence,
														const std::vector<uint8_t>& matte,
														size_t maxPayloadPerFragment= 1200)
{
	std::vector<uint8_t> payload= buildDepthPayloadV2(depth, confidence, matte);

	const size_t totalChunks= std::max<size_t>(1, (payload.size() + maxPayloadPerFragment - 1) / maxPayloadPerFragment);

	std::vector<std::vector<uint8_t>> fragments;
	size_t offset= 0;
	uint16_t fragIndex= 0;
	while (offset < payload.size() || fragments.empty())
	{
		const size_t end= std::min(offset + maxPayloadPerFragment, payload.size());

		ARKitDepthFragmentHeader header;
		header.version= kARKitDepthWireVersion;
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

// A person matte with a single vertical silhouette (left half background/0, right half
// person/1) - a realistic, RLE-friendly stencil with a real boundary.
std::vector<uint8_t> makeSilhouetteMatte()
{
	std::vector<uint8_t> matte(kARKitDepthPixelCount);
	for (int y= 0; y < kARKitDepthHeight; ++y)
		for (int x= 0; x < kARKitDepthWidth; ++x)
			matte[y * kARKitDepthWidth + x]= static_cast<uint8_t>(x >= kARKitDepthWidth / 2 ? 1 : 0);
	return matte;
}

std::vector<uint16_t> makeRealisticDepth(unsigned int seed)
{
	std::mt19937 rng(seed);
	std::vector<uint16_t> depth(kARKitDepthPixelCount);
	for (int i= 0; i < kARKitDepthPixelCount; ++i)
	{
		if (i % 100 < 20)
			depth[i]= 0;
		else
			depth[i]= static_cast<uint16_t>(500 + (rng() % 9500));
	}
	return depth;
}

std::vector<uint8_t> makeUniformConfidence(uint8_t value) { return std::vector<uint8_t>(kARKitDepthPixelCount, value); }

#if defined(_WIN32)
void depthTestEnsureWinsockInitialized()
{
	static const int result= []
	{
		WSADATA wsaData;
		return ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	}();
	(void)result;
}

bool depthTestSendLoopbackDatagram(uint16_t port, const uint8_t* data, size_t length)
{
	depthTestEnsureWinsockInitialized();

	SOCKET sock= ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
		return false;

	sockaddr_in dest{};
	dest.sin_family= AF_INET;
	dest.sin_port= htons(port);
	::inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

	const int sent= ::sendto(sock, reinterpret_cast<const char*>(data), static_cast<int>(length), 0,
							 reinterpret_cast<const sockaddr*>(&dest), sizeof(dest));
	::closesocket(sock);

	return sent == static_cast<int>(length);
}
#endif
} // namespace

//-- private functions -----
static bool arkit_depth_receiver_test_single_fragment_completes_immediately()
{
	UNIT_TEST_BEGIN("single-fragment frame (all-zero, highly compressible) completes on first fragment")

	std::vector<uint16_t> depth(kARKitDepthPixelCount, 0);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);

	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(1, 1000, depth, confidence);
	success= (fragments.size() == 1);
	assert(success);

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	ARKitDepthFrame received;
	assembler.setFrameCallback(
		[&](ARKitDepthFrame frame)
		{
			++callbackCount;
			received= std::move(frame);
		});

	const auto now= std::chrono::steady_clock::now();
	success= success && assembler.processFragment(fragments[0].data(), fragments[0].size(), now);
	assert(success);

	success= success && (callbackCount == 1);
	assert(success);
	success= success && (assembler.getPendingFrameCount() == 0);
	assert(success);
	success= success && (received.frameSeq == 1 && received.captureTimestampUs == 1000);
	assert(success);
	success= success && (received.depthMM == depth);
	assert(success);
	success= success && (received.confidence == confidence);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_multi_fragment_in_order()
{
	UNIT_TEST_BEGIN("multi-fragment realistic frame completes when fed in order")

	std::vector<uint16_t> depth= makeRealisticDepth(1);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(7, 5000, depth, confidence);
	success= (fragments.size() > 1); // realistic data should span multiple fragments
	assert(success);

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	ARKitDepthFrame received;
	assembler.setFrameCallback(
		[&](ARKitDepthFrame frame)
		{
			++callbackCount;
			received= std::move(frame);
		});

	const auto now= std::chrono::steady_clock::now();
	for (size_t i= 0; success && i + 1 < fragments.size(); ++i)
	{
		success= assembler.processFragment(fragments[i].data(), fragments[i].size(), now);
		assert(success);
		success= (callbackCount == 0); // must not complete before the last fragment
		assert(success);
	}

	success= success && assembler.processFragment(fragments.back().data(), fragments.back().size(), now);
	assert(success);
	success= success && (callbackCount == 1);
	assert(success);
	success= success && (received.depthMM == depth);
	assert(success);
	success= success && (received.confidence == confidence);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_out_of_order_and_reversed()
{
	UNIT_TEST_BEGIN("multi-fragment frame completes regardless of arrival order")

	std::vector<uint16_t> depth= makeRealisticDepth(2);
	std::vector<uint8_t> confidence= makeUniformConfidence(1);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(42, 9999, depth, confidence);
	success= (fragments.size() > 3);
	assert(success);

	// Reversed order.
	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	assembler.setFrameCallback([&](ARKitDepthFrame) { ++callbackCount; });

	const auto now= std::chrono::steady_clock::now();
	for (auto rit= fragments.rbegin(); success && rit != fragments.rend(); ++rit)
		success= assembler.processFragment(rit->data(), rit->size(), now);
	assert(success);
	success= success && (callbackCount == 1);
	assert(success);

	// Shuffled order.
	ARKitDepthFragmentAssembler assembler2;
	int callbackCount2= 0;
	ARKitDepthFrame received2;
	assembler2.setFrameCallback(
		[&](ARKitDepthFrame frame)
		{
			++callbackCount2;
			received2= std::move(frame);
		});

	std::vector<size_t> order(fragments.size());
	for (size_t i= 0; i < order.size(); ++i)
		order[i]= i;
	std::mt19937 rng(123);
	std::shuffle(order.begin(), order.end(), rng);

	for (size_t idx : order)
	{
		success= success && assembler2.processFragment(fragments[idx].data(), fragments[idx].size(), now);
		assert(success);
	}
	success= success && (callbackCount2 == 1);
	assert(success);
	success= success && (received2.depthMM == depth);
	assert(success);
	success= success && (received2.confidence == confidence);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_gaps_do_not_complete_prematurely()
{
	UNIT_TEST_BEGIN("frame with a missing fragment stays pending until the gap is filled")

	std::vector<uint16_t> depth= makeRealisticDepth(3);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(100, 42, depth, confidence);
	success= (fragments.size() >= 4);
	assert(success);

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	assembler.setFrameCallback([&](ARKitDepthFrame) { ++callbackCount; });

	const auto now= std::chrono::steady_clock::now();
	const size_t gapIndex= fragments.size() / 2;

	for (size_t i= 0; success && i < fragments.size(); ++i)
	{
		if (i == gapIndex)
			continue;
		success= assembler.processFragment(fragments[i].data(), fragments[i].size(), now);
		assert(success);
	}
	success= success && (callbackCount == 0);
	assert(success);
	success= success && (assembler.getPendingFrameCount() == 1);
	assert(success);

	success= success && assembler.processFragment(fragments[gapIndex].data(), fragments[gapIndex].size(), now);
	assert(success);
	success= success && (callbackCount == 1);
	assert(success);
	success= success && (assembler.getPendingFrameCount() == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_duplicate_fragments_do_not_double_count()
{
	UNIT_TEST_BEGIN("duplicate fragments are accepted as no-ops, frame completes exactly once")

	std::vector<uint16_t> depth= makeRealisticDepth(4);
	std::vector<uint8_t> confidence= makeUniformConfidence(0);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(9, 1, depth, confidence);
	success= (fragments.size() >= 2);
	assert(success);

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	assembler.setFrameCallback([&](ARKitDepthFrame) { ++callbackCount; });

	const auto now= std::chrono::steady_clock::now();

	// Send fragment 0 three times before ever sending the rest.
	for (int i= 0; success && i < 3; ++i)
		success= assembler.processFragment(fragments[0].data(), fragments[0].size(), now);
	assert(success);
	success= success && (callbackCount == 0);
	assert(success);

	for (size_t i= 1; success && i < fragments.size(); ++i)
		success= assembler.processFragment(fragments[i].data(), fragments[i].size(), now);
	assert(success);

	// Re-send the last fragment again after completion - should be a harmless no-op
	// against a *new* pending entry (the old one was already erased on completion),
	// not a second callback firing for the original frame.
	success= success && assembler.processFragment(fragments.back().data(), fragments.back().size(), now);
	assert(success);

	success= success && (callbackCount == 1);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_stale_frame_eviction()
{
	UNIT_TEST_BEGIN("incomplete frames are evicted after the stale timeout, but not before")

	std::vector<uint16_t> depth= makeRealisticDepth(5);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(55, 1, depth, confidence);
	success= (fragments.size() >= 2);
	assert(success);

	const auto staleTimeout= std::chrono::milliseconds(500);
	ARKitDepthFragmentAssembler assembler(staleTimeout);
	int callbackCount= 0;
	assembler.setFrameCallback([&](ARKitDepthFrame) { ++callbackCount; });

	const auto start= std::chrono::steady_clock::now();
	success= assembler.processFragment(fragments[0].data(), fragments[0].size(), start);
	assert(success);
	success= success && (assembler.getPendingFrameCount() == 1);
	assert(success);

	// Not yet stale.
	assembler.sweepStaleFrames(start + staleTimeout - std::chrono::milliseconds(1));
	success= success && (assembler.getPendingFrameCount() == 1);
	assert(success);
	success= success && (assembler.getDroppedStaleFrameCount() == 0);
	assert(success);

	// Now stale.
	assembler.sweepStaleFrames(start + staleTimeout + std::chrono::milliseconds(1));
	success= success && (assembler.getPendingFrameCount() == 0);
	assert(success);
	success= success && (assembler.getDroppedStaleFrameCount() == 1);
	assert(success);
	success= success && (callbackCount == 0); // never completed - must not fire the callback
	assert(success);

	// The remaining, now-late fragments must not crash and must not resurrect the
	// evicted frame - they start a fresh (still-incomplete) pending entry.
	for (size_t i= 1; success && i < fragments.size(); ++i)
		success= assembler.processFragment(fragments[i].data(), fragments[i].size(), start + staleTimeout * 10);
	assert(success);
	success= success && (callbackCount == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_rejects_malformed_header()
{
	UNIT_TEST_BEGIN("malformed/adversarial headers are rejected without crashing")

	ARKitDepthFragmentAssembler assembler;
	const auto now= std::chrono::steady_clock::now();

	// Too short to even contain a header.
	const uint8_t tooShort[]= {0x01, 0x02};
	success= (assembler.processFragment(tooShort, sizeof(tooShort), now) == false);
	assert(success);

	// Well-formed Pose packet bytes fed to the depth assembler - wrong type byte.
	ARKitPosePacket posePacket;
	uint8_t poseBuffer[ARKitPosePacket::kWireSize]= {};
	writeARKitPosePacket(poseBuffer, sizeof(poseBuffer), posePacket);
	success= success && (assembler.processFragment(poseBuffer, sizeof(poseBuffer), now) == false);
	assert(success);

	// fragCount == 0.
	ARKitDepthFragmentHeader zeroFragCount;
	zeroFragCount.frameSeq= 1;
	zeroFragCount.fragIndex= 0;
	zeroFragCount.fragCount= 0;
	uint8_t zeroFragCountBuffer[ARKitDepthFragmentHeader::kWireSize]= {};
	writeARKitDepthFragmentHeader(zeroFragCountBuffer, sizeof(zeroFragCountBuffer), zeroFragCount);
	success= success && (assembler.processFragment(zeroFragCountBuffer, sizeof(zeroFragCountBuffer), now) == false);
	assert(success);

	// fragIndex >= fragCount.
	ARKitDepthFragmentHeader outOfRange;
	outOfRange.frameSeq= 2;
	outOfRange.fragIndex= 5;
	outOfRange.fragCount= 3;
	uint8_t outOfRangeBuffer[ARKitDepthFragmentHeader::kWireSize]= {};
	writeARKitDepthFragmentHeader(outOfRangeBuffer, sizeof(outOfRangeBuffer), outOfRange);
	success= success && (assembler.processFragment(outOfRangeBuffer, sizeof(outOfRangeBuffer), now) == false);
	assert(success);

	success= success && (assembler.getPendingFrameCount() == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_rejects_fragcount_mismatch()
{
	UNIT_TEST_BEGIN("a fragCount mismatch for an in-progress frameSeq is rejected, existing state untouched")

	ARKitDepthFragmentAssembler assembler;
	const auto now= std::chrono::steady_clock::now();

	ARKitDepthFragmentHeader first;
	first.frameSeq= 10;
	first.fragIndex= 0;
	first.fragCount= 3;
	uint8_t firstBuffer[ARKitDepthFragmentHeader::kWireSize + 4]= {};
	writeARKitDepthFragmentHeader(firstBuffer, sizeof(firstBuffer), first);
	success= assembler.processFragment(firstBuffer, sizeof(firstBuffer), now);
	assert(success);
	success= success && (assembler.getPendingFrameCount() == 1);
	assert(success);

	ARKitDepthFragmentHeader mismatched;
	mismatched.frameSeq= 10;
	mismatched.fragIndex= 1;
	mismatched.fragCount= 7; // different from the 3 already recorded for frameSeq 10
	uint8_t mismatchedBuffer[ARKitDepthFragmentHeader::kWireSize + 4]= {};
	writeARKitDepthFragmentHeader(mismatchedBuffer, sizeof(mismatchedBuffer), mismatched);
	success= success && (assembler.processFragment(mismatchedBuffer, sizeof(mismatchedBuffer), now) == false);
	assert(success);

	// The original in-progress assembly must be unaffected by the rejected fragment.
	success= success && (assembler.getPendingFrameCount() == 1);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_rejects_corrupted_length_fields_without_crashing()
{
	UNIT_TEST_BEGIN("corrupted rvlLen/confLen fields are rejected without an out-of-bounds read")

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	assembler.setFrameCallback([&](ARKitDepthFrame) { ++callbackCount; });
	const auto now= std::chrono::steady_clock::now();

	// A single-fragment "frame" whose rvlLen claims far more bytes than actually
	// present in the payload.
	ARKitDepthFragmentHeader header;
	header.frameSeq= 77;
	header.fragIndex= 0;
	header.fragCount= 1;

	std::vector<uint8_t> payload(8, 0);
	writeU32BE(payload.data(), 0xFFFFFF00); // absurd rvlLen

	std::vector<uint8_t> datagram(ARKitDepthFragmentHeader::kWireSize + payload.size());
	writeARKitDepthFragmentHeader(datagram.data(), datagram.size(), header);
	std::copy(payload.begin(), payload.end(),
			  datagram.begin() + static_cast<ptrdiff_t>(ARKitDepthFragmentHeader::kWireSize));

	success= assembler.processFragment(datagram.data(), datagram.size(), now);
	assert(success); // the fragment itself is well-formed; only the reassembled payload is bad
	success= success && (callbackCount == 0);
	assert(success);
	success= success && (assembler.getDroppedMalformedFrameCount() == 1);
	assert(success);
	success= success && (assembler.getPendingFrameCount() == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_fuzz_corrupted_fragments_no_crash()
{
	UNIT_TEST_BEGIN("fuzzed fragment bytes never crash the assembler")

	std::vector<uint16_t> depth= makeRealisticDepth(6);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(200, 1, depth, confidence);

	std::mt19937 rng(999);
	const auto now= std::chrono::steady_clock::now();

	success= true;
	for (int trial= 0; success && trial < 300; ++trial)
	{
		ARKitDepthFragmentAssembler assembler;
		int crashCanary= 0;
		assembler.setFrameCallback([&](ARKitDepthFrame) { ++crashCanary; });

		for (auto fragment : fragments) // copy - mutate freely per trial
		{
			const int numFlips= 1 + static_cast<int>(rng() % 4);
			for (int f= 0; f < numFlips; ++f)
			{
				const size_t idx= rng() % fragment.size();
				fragment[idx]^= static_cast<uint8_t>(1 << (rng() % 8));
			}
			assembler.processFragment(fragment.data(), fragment.size(), now);
			assembler.sweepStaleFrames(now);
		}

		(void)crashCanary; // reaching here at all (no crash/hang) is the pass condition
	}
	assert(success);

	UNIT_TEST_COMPLETE()
}

#if defined(_WIN32)
static bool arkit_depth_receiver_test_end_to_end_over_real_socket()
{
	UNIT_TEST_BEGIN("ARKitDepthReceiver end-to-end: real socket + worker thread + assembler")

	std::vector<uint16_t> depth= makeRealisticDepth(11);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(321, 8675309, depth, confidence);
	success= (fragments.size() > 1);
	assert(success);

	const uint16_t port= 41200;

	ARKitDepthReceiver receiver;
	std::mutex resultMutex;
	std::atomic<int> callbackCount{0};
	ARKitDepthFrame received;
	receiver.setFrameCallback(
		[&](ARKitDepthFrame frame)
		{
			std::lock_guard<std::mutex> lock(resultMutex);
			received= std::move(frame);
			++callbackCount;
		});

	success= success && receiver.start(port);
	assert(success);

	// Send out of order, mirroring real UDP delivery.
	std::vector<size_t> order(fragments.size());
	for (size_t i= 0; i < order.size(); ++i)
		order[i]= i;
	std::mt19937 rng(55);
	std::shuffle(order.begin(), order.end(), rng);

	for (size_t idx : order)
		success= success && depthTestSendLoopbackDatagram(port, fragments[idx].data(), fragments[idx].size());
	assert(success);

	// The worker thread polls on UdpReceiveSocket's internal ~100ms timeout; give it
	// a generous window to receive and reassemble all fragments.
	for (int attempt= 0; callbackCount.load() == 0 && attempt < 50; ++attempt)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	receiver.stop();

	success= success && (callbackCount.load() == 1);
	assert(success);

	std::lock_guard<std::mutex> lock(resultMutex);
	success= success && (received.frameSeq == 321 && received.captureTimestampUs == 8675309);
	assert(success);
	success= success && (received.depthMM == depth);
	assert(success);
	success= success && (received.confidence == confidence);
	assert(success);

	UNIT_TEST_COMPLETE()
}
#endif

static bool arkit_depth_receiver_test_v2_matte_roundtrips()
{
	UNIT_TEST_BEGIN("a v2 (matte-carrying) frame decodes the person matte into ARKitDepthFrame.segmentation")

	std::vector<uint16_t> depth= makeRealisticDepth(3);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	std::vector<uint8_t> matte= makeSilhouetteMatte();

	std::vector<std::vector<uint8_t>> fragments= buildDepthFragmentsV2(9, 6000, depth, confidence, matte);

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	ARKitDepthFrame received;
	assembler.setFrameCallback(
		[&](ARKitDepthFrame frame)
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
	success= success && (received.depthMM == depth);
	assert(success);
	success= success && (received.confidence == confidence);
	assert(success);
	// The whole point of S2: the matte survives the v2 wire round-trip intact.
	success= success && (received.segmentation == matte);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_depth_receiver_test_v1_leaves_matte_empty()
{
	UNIT_TEST_BEGIN("a v1 (no-matte) frame leaves ARKitDepthFrame.segmentation empty")

	std::vector<uint16_t> depth= makeRealisticDepth(4);
	std::vector<uint8_t> confidence= makeUniformConfidence(2);
	// buildDepthFragments produces the v1 payload/version (no seg section).
	std::vector<std::vector<uint8_t>> fragments= buildDepthFragments(11, 7000, depth, confidence);

	ARKitDepthFragmentAssembler assembler;
	int callbackCount= 0;
	ARKitDepthFrame received;
	assembler.setFrameCallback(
		[&](ARKitDepthFrame frame)
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
	success= success && (received.depthMM == depth);
	assert(success);
	success= success && received.segmentation.empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_depth_receiver_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_depth_receiver")
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_single_fragment_completes_immediately);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_multi_fragment_in_order);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_out_of_order_and_reversed);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_gaps_do_not_complete_prematurely);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_duplicate_fragments_do_not_double_count);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_stale_frame_eviction);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_rejects_malformed_header);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_rejects_fragcount_mismatch);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_rejects_corrupted_length_fields_without_crashing);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_fuzz_corrupted_fragments_no_crash);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_v2_matte_roundtrips);
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_v1_leaves_matte_empty);
#if defined(_WIN32)
	UNIT_TEST_MODULE_CALL_TEST(arkit_depth_receiver_test_end_to_end_over_real_socket);
#endif
	UNIT_TEST_MODULE_END()
}
