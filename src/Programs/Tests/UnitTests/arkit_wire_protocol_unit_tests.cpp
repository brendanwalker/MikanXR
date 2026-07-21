//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>

#include "ARKitWireProtocol.h"
#include "unit_test.h"

//-- private functions -----
static bool arkit_wire_protocol_test_depth_fragment_header_roundtrip()
{
	UNIT_TEST_BEGIN("depth fragment header roundtrip")

	ARKitDepthFragmentHeader header;
	header.frameSeq= 0xDEADBEEF;
	header.captureTimestampUs= 0x0123456789ABCDEFULL;
	header.fragIndex= 3;
	header.fragCount= 7;

	uint8_t buffer[ARKitDepthFragmentHeader::kWireSize]= {};
	size_t written= writeARKitDepthFragmentHeader(buffer, sizeof(buffer), header);
	success= (written == ARKitDepthFragmentHeader::kWireSize);
	assert(success);

	// Spot-check big-endian byte order directly, not just the round-trip, since a
	// symmetric little-endian bug would otherwise round-trip cleanly and hide itself.
	success= success && (buffer[0] == 0xAD && buffer[1] == 0x01); // magic
	assert(success);
	success= success && (buffer[4] == 0xDE && buffer[5] == 0xAD && buffer[6] == 0xBE && buffer[7] == 0xEF); // frameSeq
	assert(success);

	ARKitDepthFragmentHeader roundTripped;
	success= success && readARKitDepthFragmentHeader(buffer, sizeof(buffer), roundTripped);
	assert(success);

	success= success && roundTripped.magic == header.magic;
	success= success && roundTripped.version == header.version;
	success= success && roundTripped.type == header.type;
	success= success && roundTripped.frameSeq == header.frameSeq;
	success= success && roundTripped.captureTimestampUs == header.captureTimestampUs;
	success= success && roundTripped.fragIndex == header.fragIndex;
	success= success && roundTripped.fragCount == header.fragCount;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_wire_protocol_test_depth_fragment_header_rejects_short_buffer()
{
	UNIT_TEST_BEGIN("depth fragment header rejects undersized buffer")

	ARKitDepthFragmentHeader header;
	uint8_t buffer[ARKitDepthFragmentHeader::kWireSize]= {};

	success= (writeARKitDepthFragmentHeader(buffer, ARKitDepthFragmentHeader::kWireSize - 1, header) == 0);
	assert(success);

	ARKitDepthFragmentHeader outHeader;
	success=
		success && (readARKitDepthFragmentHeader(buffer, ARKitDepthFragmentHeader::kWireSize - 1, outHeader) == false);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_wire_protocol_test_depth_fragment_header_rejects_wrong_type()
{
	UNIT_TEST_BEGIN("depth fragment header rejects mismatched packet type")

	// Fabricate a buffer whose type byte claims Pose, and confirm the depth reader
	// (which validates eARKitPacketType::Depth) rejects it rather than parsing garbage.
	ARKitPosePacket posePacket;
	uint8_t buffer[ARKitPosePacket::kWireSize]= {};
	writeARKitPosePacket(buffer, sizeof(buffer), posePacket);

	ARKitDepthFragmentHeader outHeader;
	success= (readARKitDepthFragmentHeader(buffer, ARKitDepthFragmentHeader::kWireSize, outHeader) == false);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_wire_protocol_test_pose_packet_roundtrip()
{
	UNIT_TEST_BEGIN("pose packet roundtrip")

	ARKitPosePacket packet;
	packet.frameSeq= 42;
	packet.captureTimestampUs= 1234567890123ULL;
	for (int i= 0; i < 16; ++i)
	{
		packet.transform[i]= (i == 0 || i == 5 || i == 10 || i == 15) ? 1.0f : 0.0f; // identity
	}
	packet.fx= 1428.5f;
	packet.fy= 1428.5f;
	packet.cx= 960.25f;
	packet.cy= 540.75f;
	packet.imageWidth= 1920.0f;
	packet.imageHeight= 1080.0f;

	uint8_t buffer[ARKitPosePacket::kWireSize]= {};
	size_t written= writeARKitPosePacket(buffer, sizeof(buffer), packet);
	success= (written == ARKitPosePacket::kWireSize);
	assert(success);

	ARKitPosePacket roundTripped;
	success= success && readARKitPosePacket(buffer, sizeof(buffer), roundTripped);
	assert(success);

	success= success && roundTripped.frameSeq == packet.frameSeq;
	success= success && roundTripped.captureTimestampUs == packet.captureTimestampUs;
	for (int i= 0; success && i < 16; ++i)
	{
		success= (roundTripped.transform[i] == packet.transform[i]);
	}
	assert(success);
	success= success && roundTripped.fx == packet.fx;
	success= success && roundTripped.fy == packet.fy;
	success= success && roundTripped.cx == packet.cx;
	success= success && roundTripped.cy == packet.cy;
	success= success && roundTripped.imageWidth == packet.imageWidth;
	success= success && roundTripped.imageHeight == packet.imageHeight;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_wire_protocol_test_rtp_extension_payload_roundtrip()
{
	UNIT_TEST_BEGIN("RTP extension payload roundtrip")

	ARKitRTPExtensionPayload payload;
	payload.frameSeq= 0x00112233;
	payload.captureTimestampUs= 0x0011223344556677ULL;

	uint8_t buffer[ARKitRTPExtensionPayload::kWireSize]= {};
	size_t written= writeARKitRTPExtensionPayload(buffer, sizeof(buffer), payload);
	success= (written == ARKitRTPExtensionPayload::kWireSize);
	assert(success);

	ARKitRTPExtensionPayload roundTripped;
	success= success && readARKitRTPExtensionPayload(buffer, sizeof(buffer), roundTripped);
	assert(success);

	success= success && roundTripped.frameSeq == payload.frameSeq;
	success= success && roundTripped.captureTimestampUs == payload.captureTimestampUs;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_wire_protocol_test_be_primitive_helpers()
{
	UNIT_TEST_BEGIN("big-endian primitive helpers")

	uint8_t buffer[8]= {};

	writeU16BE(buffer, 0xABCD);
	success= (buffer[0] == 0xAB && buffer[1] == 0xCD);
	success= success && (readU16BE(buffer) == 0xABCD);
	assert(success);

	writeU32BE(buffer, 0x11223344);
	success= success && (buffer[0] == 0x11 && buffer[1] == 0x22 && buffer[2] == 0x33 && buffer[3] == 0x44);
	success= success && (readU32BE(buffer) == 0x11223344);
	assert(success);

	writeU64BE(buffer, 0x0102030405060708ULL);
	success= success && (buffer[0] == 0x01 && buffer[7] == 0x08);
	success= success && (readU64BE(buffer) == 0x0102030405060708ULL);
	assert(success);

	writeF32BE(buffer, -12345.678f);
	success= success && (readF32BE(buffer) == -12345.678f);
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_wire_protocol_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_wire_protocol")
	UNIT_TEST_MODULE_CALL_TEST(arkit_wire_protocol_test_be_primitive_helpers);
	UNIT_TEST_MODULE_CALL_TEST(arkit_wire_protocol_test_depth_fragment_header_roundtrip);
	UNIT_TEST_MODULE_CALL_TEST(arkit_wire_protocol_test_depth_fragment_header_rejects_short_buffer);
	UNIT_TEST_MODULE_CALL_TEST(arkit_wire_protocol_test_depth_fragment_header_rejects_wrong_type);
	UNIT_TEST_MODULE_CALL_TEST(arkit_wire_protocol_test_pose_packet_roundtrip);
	UNIT_TEST_MODULE_CALL_TEST(arkit_wire_protocol_test_rtp_extension_payload_roundtrip);
	UNIT_TEST_MODULE_END()
}
