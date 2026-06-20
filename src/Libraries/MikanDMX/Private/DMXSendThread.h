#pragma once

#include "E131Packet.h"
#include "UdpMulticastSocket.h"

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <thread>

/// Per-universe transmit buffer with a dirty flag.
struct UniverseBuffer
{
	uint8_t slots[512]= {}; // DMX slot data (slots 1–512)
	uint8_t sequenceNumber= 0;
	bool dirty= false;
};

/// Background thread that transmits dirty E1.31 universe buffers at a fixed rate.
/// Thread-safe: setChannels / setUniverseData may be called from any thread.
class DMXSendThread
{
public:
	DMXSendThread();
	~DMXSendThread();

	// Non-copyable
	DMXSendThread(const DMXSendThread&)= delete;
	DMXSendThread& operator=(const DMXSendThread&)= delete;

	bool start(
		const std::string& bindIP,
		const std::string& sourceName,
		uint8_t priority,
		float transmitRateHz);

	void stop();

	bool isRunning() const { return m_running.load(); }

	/// Thread-safe: overwrite a contiguous channel range in a universe buffer.
	void setChannels(uint16_t universe, uint16_t startChannel, const uint8_t* values, uint16_t count);

	/// Thread-safe: overwrite all 512 slots in a universe buffer.
	void setUniverseData(uint16_t universe, const uint8_t* slotData, uint16_t slotCount);

private:
	void threadFunc();

	void transmitUniverse(uint16_t universe, UniverseBuffer& buf);

	// Config (set at start(), read-only after that)
	float m_transmitRateHz= 44.0f;
	uint8_t m_cid[16]= {};
	uint8_t m_priority= 100;
	std::string m_sourceName;

	// Socket
	UdpMulticastSocket m_socket;

	// Packet template (header is identical for every universe; universe + seq vary)
	E131Packet m_packetTemplate;

	// Universe buffers — protected by m_bufferMutex
	std::mutex m_bufferMutex;
	std::map<uint16_t, UniverseBuffer> m_universeBuffers;

	// Thread control
	std::atomic<bool> m_running{false};
	std::atomic<bool> m_stopRequested{false};
	std::thread m_thread;
};
