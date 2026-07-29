#pragma once

// -- includes -----
#include <cstddef>
#include <cstdint>
#include <cstring>

// The ARKit streaming wire protocol is big-endian (network byte order) on every
// multi-byte field, but x86/x64 is little-endian and struct layout/padding is not
// portable across compilers. So these structs are plain native-endian in-memory
// representations only - never memcpy/reinterpret_cast them onto socket bytes.
// Always go through the read*/write* helpers below, which serialize field-by-field
// at explicit byte offsets.

// -- protocol constants -----
constexpr uint16_t kARKitWireProtocolMagic= 0xAD01;
constexpr uint8_t kARKitWireProtocolVersion= 1;

enum class eARKitPacketType : uint8_t
{
	Pose= 2,
};

// -- big-endian primitive helpers -----

inline void writeU16BE(uint8_t* buffer, uint16_t value)
{
	buffer[0]= static_cast<uint8_t>((value >> 8) & 0xFF);
	buffer[1]= static_cast<uint8_t>(value & 0xFF);
}

inline uint16_t readU16BE(const uint8_t* buffer)
{
	return static_cast<uint16_t>((static_cast<uint16_t>(buffer[0]) << 8) | static_cast<uint16_t>(buffer[1]));
}

inline void writeU32BE(uint8_t* buffer, uint32_t value)
{
	buffer[0]= static_cast<uint8_t>((value >> 24) & 0xFF);
	buffer[1]= static_cast<uint8_t>((value >> 16) & 0xFF);
	buffer[2]= static_cast<uint8_t>((value >> 8) & 0xFF);
	buffer[3]= static_cast<uint8_t>(value & 0xFF);
}

inline uint32_t readU32BE(const uint8_t* buffer)
{
	return (static_cast<uint32_t>(buffer[0]) << 24) | (static_cast<uint32_t>(buffer[1]) << 16)
		   | (static_cast<uint32_t>(buffer[2]) << 8) | static_cast<uint32_t>(buffer[3]);
}

inline void writeU64BE(uint8_t* buffer, uint64_t value)
{
	for (int i= 0; i < 8; ++i)
	{
		buffer[i]= static_cast<uint8_t>((value >> (56 - i * 8)) & 0xFF);
	}
}

inline uint64_t readU64BE(const uint8_t* buffer)
{
	uint64_t value= 0;
	for (int i= 0; i < 8; ++i)
	{
		value= (value << 8) | static_cast<uint64_t>(buffer[i]);
	}
	return value;
}

inline void writeF32BE(uint8_t* buffer, float value)
{
	uint32_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	writeU32BE(buffer, bits);
}

inline float readF32BE(const uint8_t* buffer)
{
	const uint32_t bits= readU32BE(buffer);
	float value;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

// -- wire structs -----
// kWireSize is the hand-verified serialized size and is intentionally not checked
// against sizeof(struct) - these are native structs with compiler-dependent
// alignment/padding, not a layout of the wire format (see note above).

// Pose channel (basePort+2) packet. Single UDP datagram, never fragmented.
struct ARKitPosePacket
{
	static constexpr size_t kWireSize= 104;

	uint16_t magic= kARKitWireProtocolMagic;
	uint8_t version= kARKitWireProtocolVersion;
	eARKitPacketType type= eARKitPacketType::Pose;
	uint32_t frameSeq= 0;
	uint64_t captureTimestampUs= 0;
	float transform[16]= {}; // row-major 4x4, camera-to-world
	float fx= 0.f;
	float fy= 0.f;
	float cx= 0.f;
	float cy= 0.f;
	float imageWidth= 0.f;
	float imageHeight= 0.f;
};

// RTP header extension (RFC 5285) payload attached to every video-stream (basePort+0)
// RTP packet, correlating a decoded video frame back to a frameSeq.
struct ARKitRTPExtensionPayload
{
	static constexpr size_t kWireSize= 12;

	uint32_t frameSeq= 0;
	uint64_t captureTimestampUs= 0;
};

// -- (de)serialization -----

inline size_t writeARKitPosePacket(uint8_t* buffer, size_t bufferSize, const ARKitPosePacket& packet)
{
	if (buffer == nullptr || bufferSize < ARKitPosePacket::kWireSize)
		return 0;

	size_t offset= 0;
	writeU16BE(buffer + offset, packet.magic);
	offset+= sizeof(uint16_t);
	buffer[offset]= packet.version;
	offset+= sizeof(uint8_t);
	buffer[offset]= static_cast<uint8_t>(packet.type);
	offset+= sizeof(uint8_t);
	writeU32BE(buffer + offset, packet.frameSeq);
	offset+= sizeof(uint32_t);
	writeU64BE(buffer + offset, packet.captureTimestampUs);
	offset+= sizeof(uint64_t);

	for (int i= 0; i < 16; ++i)
	{
		writeF32BE(buffer + offset, packet.transform[i]);
		offset+= sizeof(float);
	}

	writeF32BE(buffer + offset, packet.fx);
	offset+= sizeof(float);
	writeF32BE(buffer + offset, packet.fy);
	offset+= sizeof(float);
	writeF32BE(buffer + offset, packet.cx);
	offset+= sizeof(float);
	writeF32BE(buffer + offset, packet.cy);
	offset+= sizeof(float);
	writeF32BE(buffer + offset, packet.imageWidth);
	offset+= sizeof(float);
	writeF32BE(buffer + offset, packet.imageHeight);
	offset+= sizeof(float);

	return offset;
}

inline bool readARKitPosePacket(const uint8_t* buffer, size_t bufferSize, ARKitPosePacket& outPacket)
{
	if (buffer == nullptr || bufferSize < ARKitPosePacket::kWireSize)
		return false;

	size_t offset= 0;
	outPacket.magic= readU16BE(buffer + offset);
	offset+= sizeof(uint16_t);
	outPacket.version= buffer[offset];
	offset+= sizeof(uint8_t);
	outPacket.type= static_cast<eARKitPacketType>(buffer[offset]);
	offset+= sizeof(uint8_t);
	outPacket.frameSeq= readU32BE(buffer + offset);
	offset+= sizeof(uint32_t);
	outPacket.captureTimestampUs= readU64BE(buffer + offset);
	offset+= sizeof(uint64_t);

	for (int i= 0; i < 16; ++i)
	{
		outPacket.transform[i]= readF32BE(buffer + offset);
		offset+= sizeof(float);
	}

	outPacket.fx= readF32BE(buffer + offset);
	offset+= sizeof(float);
	outPacket.fy= readF32BE(buffer + offset);
	offset+= sizeof(float);
	outPacket.cx= readF32BE(buffer + offset);
	offset+= sizeof(float);
	outPacket.cy= readF32BE(buffer + offset);
	offset+= sizeof(float);
	outPacket.imageWidth= readF32BE(buffer + offset);
	offset+= sizeof(float);
	outPacket.imageHeight= readF32BE(buffer + offset);
	offset+= sizeof(float);

	return outPacket.magic == kARKitWireProtocolMagic && outPacket.version == kARKitWireProtocolVersion
		   && outPacket.type == eARKitPacketType::Pose;
}

inline size_t writeARKitRTPExtensionPayload(uint8_t* buffer, size_t bufferSize, const ARKitRTPExtensionPayload& payload)
{
	if (buffer == nullptr || bufferSize < ARKitRTPExtensionPayload::kWireSize)
		return 0;

	size_t offset= 0;
	writeU32BE(buffer + offset, payload.frameSeq);
	offset+= sizeof(uint32_t);
	writeU64BE(buffer + offset, payload.captureTimestampUs);
	offset+= sizeof(uint64_t);

	return offset;
}

inline bool readARKitRTPExtensionPayload(const uint8_t* buffer, size_t bufferSize, ARKitRTPExtensionPayload& outPayload)
{
	if (buffer == nullptr || bufferSize < ARKitRTPExtensionPayload::kWireSize)
		return false;

	size_t offset= 0;
	outPayload.frameSeq= readU32BE(buffer + offset);
	offset+= sizeof(uint32_t);
	outPayload.captureTimestampUs= readU64BE(buffer + offset);
	offset+= sizeof(uint64_t);

	return true;
}
