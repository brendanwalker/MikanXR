#include "E131Packet.h"

#include <cstring>
#include <algorithm>

// Byte-order helpers
static uint16_t hton16(uint16_t v)
{
#if defined(_WIN32)
	return (v >> 8) | (v << 8);
#else
	return __builtin_bswap16(v);
#endif
}

static uint32_t hton32(uint32_t v)
{
#if defined(_WIN32)
	return ((v & 0xFF000000u) >> 24) | ((v & 0x00FF0000u) >> 8) | ((v & 0x0000FF00u) << 8) | ((v & 0x000000FFu) << 24);
#else
	return __builtin_bswap32(v);
#endif
}

// Packet size constants
constexpr uint16_t E131_PACKET_SIZE= 638;
constexpr uint16_t ROOT_FLENGTH_VALUE= 0x7000 | (E131_PACKET_SIZE - 16);
constexpr uint16_t FRAME_FLENGTH_VALUE= 0x7000 | (E131_PACKET_SIZE - 38);
constexpr uint16_t DMP_FLENGTH_VALUE= 0x7000 | (E131_PACKET_SIZE - 115);

// E1.31 vector constants
constexpr uint32_t VECTOR_ROOT_E131_DATA= 0x00000004;
constexpr uint32_t VECTOR_E131_DATA_PACKET= 0x00000002;
constexpr uint8_t VECTOR_DMP_SET_PROPERTY= 0x02;

static const uint8_t k_acnPacketIdentifier[12]= {0x41, 0x53, 0x43, 0x2D, 0x45, 0x31,
												 0x2E, 0x31, 0x37, 0x00, 0x00, 0x00}; // "ASC-E1.17\0\0\0"

void e131_packet_init(E131Packet& pkt, const uint8_t cid[16], const char* sourceName, uint8_t priority)
{
	std::memset(&pkt, 0, sizeof(pkt));

	// Root Layer
	pkt.preamble_size= hton16(0x0010);
	pkt.postamble_size= 0x0000;
	std::memcpy(pkt.acn_id, k_acnPacketIdentifier, 12);
	pkt.root_flength= hton16(ROOT_FLENGTH_VALUE);
	pkt.root_vector= hton32(VECTOR_ROOT_E131_DATA);
	std::memcpy(pkt.cid, cid, 16);

	// Framing Layer
	pkt.frame_flength= hton16(FRAME_FLENGTH_VALUE);
	pkt.frame_vector= hton32(VECTOR_E131_DATA_PACKET);
	if (sourceName)
	{
		std::strncpy(reinterpret_cast<char*>(pkt.source_name), sourceName, 63);
		pkt.source_name[63]= '\0';
	}
	pkt.priority= priority;
	pkt.synchronization= 0;
	pkt.sequence_number= 0;
	pkt.options= 0;
	pkt.universe= 0; // set per-universe via e131_packet_set_universe()

	// DMP Layer
	pkt.dmp_flength= hton16(DMP_FLENGTH_VALUE);
	pkt.dmp_vector= VECTOR_DMP_SET_PROPERTY;
	pkt.address_type= 0xa1;
	pkt.first_address= 0x0000;
	pkt.address_increment= hton16(0x0001);
	pkt.property_count= hton16(513); // 1 start code + 512 slots
	pkt.property_values[0]= 0x00;    // DMX512 NULL start code
}

void e131_packet_set_universe(E131Packet& pkt, uint16_t universe) { pkt.universe= hton16(universe); }

void e131_packet_set_slots(E131Packet& pkt, uint16_t startSlot, const uint8_t* values, uint16_t count)
{
	// property_values[0] is the DMX start code; slots are at [1..512]
	const uint16_t clampedCount= std::min<uint16_t>(count, static_cast<uint16_t>(512 - (startSlot - 1)));
	std::memcpy(&pkt.property_values[startSlot], values, clampedCount);
}
