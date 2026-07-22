//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
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

#include "ARKitPoseReceiver.h"
#include "ARKitWireProtocol.h"
#include "unit_test.h"

namespace
{
ARKitPosePacket makeSamplePosePacket(uint32_t frameSeq)
{
	ARKitPosePacket packet;
	packet.frameSeq= frameSeq;
	packet.captureTimestampUs= 1000000ULL + frameSeq;
	for (int i= 0; i < 16; ++i)
		packet.transform[i]= (i == 0 || i == 5 || i == 10 || i == 15) ? 1.0f : static_cast<float>(i) * 0.1f;
	packet.fx= 1428.5f;
	packet.fy= 1428.5f;
	packet.cx= 960.25f;
	packet.cy= 540.75f;
	packet.imageWidth= 1920.0f;
	packet.imageHeight= 1080.0f;
	return packet;
}

// Builds a sample ARKitPoseFrame directly by round-tripping through the real
// wire-format write/parse path, rather than hand-filling an ARKitPoseFrame, so
// ring buffer tests exercise the same code the receiver actually uses.
ARKitPoseFrame parseSampleFrame(uint32_t frameSeq)
{
	ARKitPosePacket packet= makeSamplePosePacket(frameSeq);
	uint8_t buffer[ARKitPosePacket::kWireSize]= {};
	writeARKitPosePacket(buffer, sizeof(buffer), packet);

	ARKitPoseFrame frame;
	parseARKitPoseDatagram(buffer, sizeof(buffer), frame);
	return frame;
}

#if defined(_WIN32)
void poseTestEnsureWinsockInitialized()
{
	static const int result= []
	{
		WSADATA wsaData;
		return ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	}();
	(void)result;
}

bool poseTestSendLoopbackDatagram(uint16_t port, const uint8_t* data, size_t length)
{
	poseTestEnsureWinsockInitialized();

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
static bool arkit_pose_receiver_test_parse_roundtrip()
{
	UNIT_TEST_BEGIN("parseARKitPoseDatagram roundtrip")

	ARKitPosePacket packet= makeSamplePosePacket(123);
	uint8_t buffer[ARKitPosePacket::kWireSize]= {};
	success= (writeARKitPosePacket(buffer, sizeof(buffer), packet) == ARKitPosePacket::kWireSize);
	assert(success);

	ARKitPoseFrame frame;
	success= success && parseARKitPoseDatagram(buffer, sizeof(buffer), frame);
	assert(success);

	success= success && (frame.frameSeq == packet.frameSeq);
	success= success && (frame.captureTimestampUs == packet.captureTimestampUs);
	for (int i= 0; success && i < 16; ++i)
		success= (frame.transform[i] == packet.transform[i]);
	assert(success);
	success= success && frame.fx == packet.fx && frame.fy == packet.fy;
	success= success && frame.cx == packet.cx && frame.cy == packet.cy;
	success= success && frame.imageWidth == packet.imageWidth && frame.imageHeight == packet.imageHeight;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_pose_receiver_test_parse_rejects_malformed_input()
{
	UNIT_TEST_BEGIN("parseARKitPoseDatagram rejects malformed/truncated input")

	ARKitPoseFrame frame;

	// Too short to contain even the fixed header.
	const uint8_t tooShort[]= {0x01, 0x02, 0x03};
	success= (parseARKitPoseDatagram(tooShort, sizeof(tooShort), frame) == false);
	assert(success);

	// Truncated one byte short of a full, otherwise-valid packet.
	ARKitPosePacket packet= makeSamplePosePacket(5);
	uint8_t buffer[ARKitPosePacket::kWireSize]= {};
	writeARKitPosePacket(buffer, sizeof(buffer), packet);
	success= success && (parseARKitPoseDatagram(buffer, sizeof(buffer) - 1, frame) == false);
	assert(success);

	// Wrong packet type - a depth fragment header fed to the pose parser.
	ARKitDepthFragmentHeader depthHeader;
	depthHeader.frameSeq= 1;
	depthHeader.fragIndex= 0;
	depthHeader.fragCount= 1;
	uint8_t depthBuffer[ARKitDepthFragmentHeader::kWireSize]= {};
	writeARKitDepthFragmentHeader(depthBuffer, sizeof(depthBuffer), depthHeader);
	success= success && (parseARKitPoseDatagram(depthBuffer, sizeof(depthBuffer), frame) == false);
	assert(success);

	// Corrupted magic byte in an otherwise well-formed pose packet.
	uint8_t corruptedMagic[ARKitPosePacket::kWireSize];
	std::memcpy(corruptedMagic, buffer, sizeof(corruptedMagic));
	corruptedMagic[0]^= 0xFF;
	success= success && (parseARKitPoseDatagram(corruptedMagic, sizeof(corruptedMagic), frame) == false);
	assert(success);

	// Null buffer.
	success= success && (parseARKitPoseDatagram(nullptr, 0, frame) == false);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_pose_receiver_test_ring_buffer_basic_push_and_get()
{
	UNIT_TEST_BEGIN("ring buffer push/tryGet round trip")

	ARKitPoseRingBuffer ring(30);

	ARKitPoseFrame in= parseSampleFrame(7);
	ring.push(in);

	ARKitPoseFrame out;
	success= ring.tryGet(7, out);
	assert(success);
	success= success && (out.frameSeq == 7);
	assert(success);

	// Never-pushed frameSeq.
	ARKitPoseFrame missing;
	success= success && (ring.tryGet(999, missing) == false);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_pose_receiver_test_ring_buffer_fifo_eviction()
{
	UNIT_TEST_BEGIN("ring buffer evicts oldest entries once over capacity")

	const size_t kCapacity= 5;
	ARKitPoseRingBuffer ring(kCapacity);

	for (uint32_t seq= 0; seq < 10; ++seq)
	{
		ARKitPoseFrame frame= parseSampleFrame(seq);
		ring.push(frame);
	}

	success= (ring.size() == kCapacity);
	assert(success);

	// The oldest 5 (0-4) should be evicted; the newest 5 (5-9) should remain.
	ARKitPoseFrame out;
	for (uint32_t seq= 0; success && seq < 5; ++seq)
		success= (ring.tryGet(seq, out) == false);
	assert(success);

	for (uint32_t seq= 5; success && seq < 10; ++seq)
		success= ring.tryGet(seq, out) && out.frameSeq == seq;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_pose_receiver_test_ring_buffer_duplicate_update_does_not_evict()
{
	UNIT_TEST_BEGIN("re-pushing an already-tracked frameSeq updates in place, no extra eviction")

	const size_t kCapacity= 3;
	ARKitPoseRingBuffer ring(kCapacity);

	ring.push(parseSampleFrame(1));
	ring.push(parseSampleFrame(2));
	ring.push(parseSampleFrame(3));
	success= (ring.size() == 3);
	assert(success);

	// Re-push frameSeq 1 with different timestamp data - should update, not evict
	// frameSeq 2 to make room.
	ARKitPoseFrame updated= parseSampleFrame(1);
	updated.captureTimestampUs= 999999;
	ring.push(updated);

	success= success && (ring.size() == 3);
	assert(success);

	ARKitPoseFrame out;
	success= success && ring.tryGet(1, out) && out.captureTimestampUs == 999999;
	assert(success);
	success= success && ring.tryGet(2, out);
	assert(success);
	success= success && ring.tryGet(3, out);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_pose_receiver_test_ring_buffer_thread_safety()
{
	UNIT_TEST_BEGIN("concurrent push/tryGet from multiple threads does not crash or corrupt")

	ARKitPoseRingBuffer ring(50);
	std::atomic<bool> stop{false};

	std::thread writer(
		[&]
		{
			for (uint32_t seq= 0; seq < 5000; ++seq)
				ring.push(parseSampleFrame(seq));
			stop.store(true);
		});

	std::thread reader(
		[&]
		{
			ARKitPoseFrame out;
			while (!stop.load())
			{
				ring.tryGet(0, out); // may or may not still be present - just must not crash
				ring.size();
			}
		});

	writer.join();
	reader.join();

	success= true; // reaching here without crashing/hanging is the pass condition

	UNIT_TEST_COMPLETE()
}

#if defined(_WIN32)
static bool arkit_pose_receiver_test_end_to_end_over_real_socket()
{
	UNIT_TEST_BEGIN("ARKitPoseReceiver end-to-end: real socket + worker thread")

	const uint16_t port= 41300;

	ARKitPoseReceiver receiver;
	std::atomic<int> callbackCount{0};
	receiver.setFrameCallback([&](ARKitPoseFrame) { ++callbackCount; });

	success= receiver.start(port);
	assert(success);

	ARKitPosePacket packet= makeSamplePosePacket(55);
	uint8_t buffer[ARKitPosePacket::kWireSize]= {};
	writeARKitPosePacket(buffer, sizeof(buffer), packet);
	success= success && poseTestSendLoopbackDatagram(port, buffer, sizeof(buffer));
	assert(success);

	// Also send one malformed datagram - must not crash the receiver or land in
	// the ring buffer.
	const uint8_t garbage[]= {0xFF, 0xFF, 0xFF, 0xFF};
	success= success && poseTestSendLoopbackDatagram(port, garbage, sizeof(garbage));
	assert(success);

	for (int attempt= 0; callbackCount.load() == 0 && attempt < 50; ++attempt)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	ARKitPoseFrame retrieved;
	const bool found= receiver.tryGetPose(55, retrieved);

	receiver.stop();

	success= success && (callbackCount.load() == 1);
	assert(success);
	success= success && found;
	assert(success);
	success= success && (retrieved.frameSeq == 55);
	assert(success);
	success= success && (receiver.getRejectedMalformedCount() == 1);
	assert(success);

	UNIT_TEST_COMPLETE()
}
#endif

//-- public interface -----
bool run_arkit_pose_receiver_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_pose_receiver")
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_parse_roundtrip);
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_parse_rejects_malformed_input);
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_ring_buffer_basic_push_and_get);
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_ring_buffer_fifo_eviction);
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_ring_buffer_duplicate_update_does_not_evict);
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_ring_buffer_thread_safety);
#if defined(_WIN32)
	UNIT_TEST_MODULE_CALL_TEST(arkit_pose_receiver_test_end_to_end_over_real_socket);
#endif
	UNIT_TEST_MODULE_END()
}
