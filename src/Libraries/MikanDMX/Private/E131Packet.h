#pragma once

#include <cstdint>
#include <cstring>

/// Full E1.31 (sACN) packet layout — 638 bytes, all fields big-endian.
/// Ref: ANSI E1.31-2018, Section 6.
#pragma pack(push, 1)
struct E131Packet
{
	// --- Root Layer (Section 5.1) ---
	uint16_t preamble_size;  // 0x0010
	uint16_t postamble_size; // 0x0000
	uint8_t acn_id[12];      // "ASC-E1.17\0\0\0"
	uint16_t root_flength;   // 0x7000 | (packet_size - 16)
	uint32_t root_vector;    // VECTOR_ROOT_E131_DATA = 0x00000004
	uint8_t cid[16];         // Sender CID (UUID)

	// --- Framing Layer (Section 6.2) ---
	uint16_t frame_flength;   // 0x7000 | (packet_size - 38)
	uint32_t frame_vector;    // VECTOR_E131_DATA_PACKET = 0x00000002
	uint8_t source_name[64];  // UTF-8, null-padded
	uint8_t priority;         // 0–200, default 100
	uint16_t synchronization; // 0 = no synchronization
	uint8_t sequence_number;  // Increments per universe per packet
	uint8_t options;          // 0 = normal
	uint16_t universe;        // E1.31 universe (big-endian)

	// --- DMP Layer (Section 7) ---
	uint16_t dmp_flength;         // 0x7000 | (packet_size - 115)
	uint8_t dmp_vector;           // VECTOR_DMP_SET_PROPERTY = 0x02
	uint8_t address_type;         // 0xa1 = absolute, range, single-octet
	uint16_t first_address;       // 0x0000
	uint16_t address_increment;   // 0x0001
	uint16_t property_count;      // 513 = 1 start-code + 512 slots
	uint8_t property_values[513]; // [0] = DMX512 start code (0x00), [1..512] = slot data
};
#pragma pack(pop)

static_assert(sizeof(E131Packet) == 638, "E131Packet must be exactly 638 bytes");

/// E1.31 multicast destination for a given universe:
///   239.255.X.Y  where X = (universe >> 8) & 0xFF, Y = universe & 0xFF
inline void e131_multicast_address(uint16_t universe, uint8_t out_ip[4])
{
	out_ip[0]= 239;
	out_ip[1]= 255;
	out_ip[2]= static_cast<uint8_t>((universe >> 8) & 0xFF);
	out_ip[3]= static_cast<uint8_t>(universe & 0xFF);
}

constexpr uint16_t E131_PORT= 5568;

/// Initialize an E131Packet with fixed header values.
/// Call this once; then update universe, sequence_number, and property_values[1..512] per send.
void e131_packet_init(E131Packet& pkt, const uint8_t cid[16], const char* sourceName, uint8_t priority);

/// Update the per-universe fields (universe + DMP property count) before sending.
void e131_packet_set_universe(E131Packet& pkt, uint16_t universe);

/// Write count slot values starting at slot startSlot (1-based) into property_values.
void e131_packet_set_slots(E131Packet& pkt, uint16_t startSlot, const uint8_t* values, uint16_t count);
