#include "DMXSendThread.h"
#include "E131Packet.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <random>
#include <sstream>

static void generateRandomCID(uint8_t cid[16])
{
	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<uint16_t> dist(0, 255);
	for (int i= 0; i < 16; ++i)
		cid[i]= static_cast<uint8_t>(dist(rng));
	// Mark as RFC 4122 version 4
	cid[6]= (cid[6] & 0x0F) | 0x40;
	cid[8]= (cid[8] & 0x3F) | 0x80;
}

DMXSendThread::DMXSendThread() { generateRandomCID(m_cid); }

DMXSendThread::~DMXSendThread() { stop(); }

bool DMXSendThread::start(const std::string& bindIP, const std::string& sourceName, uint8_t priority,
						  float transmitRateHz)
{
	if (m_running.load())
		return true;

	m_sourceName= sourceName;
	m_priority= priority;
	m_transmitRateHz= transmitRateHz > 0.0f ? transmitRateHz : 44.0f;

	if (!m_socket.open(bindIP))
		return false;

	e131_packet_init(m_packetTemplate, m_cid, sourceName.c_str(), priority);

	m_stopRequested.store(false);
	m_thread= std::thread(&DMXSendThread::threadFunc, this);
	m_running.store(true);
	return true;
}

void DMXSendThread::stop()
{
	if (!m_running.load())
		return;

	m_stopRequested.store(true);
	if (m_thread.joinable())
		m_thread.join();

	m_socket.close();
	m_running.store(false);
}

void DMXSendThread::setChannels(uint16_t universe, uint16_t startChannel, const uint8_t* values, uint16_t count)
{
	assert(startChannel >= 1 && startChannel <= 512);

	const uint16_t maxCount= static_cast<uint16_t>(512 - (startChannel - 1));
	const uint16_t clampedCount= std::min(count, maxCount);

	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);

		UniverseBuffer& buf= m_universeBuffers[universe];
		std::memcpy(&buf.slots[startChannel - 1], values, clampedCount);
	}
}

void DMXSendThread::setUniverseData(uint16_t universe, const uint8_t* slotData, uint16_t slotCount)
{
	const uint16_t clampedCount= std::min<uint16_t>(slotCount, 512);

	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);

		UniverseBuffer& buf= m_universeBuffers[universe];
		std::memcpy(buf.slots, slotData, clampedCount);
		if (clampedCount < 512)
			std::memset(&buf.slots[clampedCount], 0, 512 - clampedCount);
	}
}

void DMXSendThread::transmitUniverse(uint16_t universe, UniverseBuffer& buf)
{
	// Fill the packet template with this universe's data
	E131Packet pkt= m_packetTemplate;
	e131_packet_set_universe(pkt, universe);
	pkt.sequence_number= buf.sequenceNumber++;

	// Copy slot data into property_values[1..512]
	e131_packet_set_slots(pkt, 1, buf.slots, 512);

	// Compute multicast destination IP: 239.255.X.Y
	uint8_t destIPBytes[4];
	e131_multicast_address(universe, destIPBytes);
	char destIP[16];
	std::snprintf(destIP, sizeof(destIP), "%u.%u.%u.%u", destIPBytes[0], destIPBytes[1], destIPBytes[2],
				  destIPBytes[3]);

	m_socket.sendTo(destIP, E131_PORT, &pkt, sizeof(pkt));
}

void DMXSendThread::threadFunc()
{
	using Clock= std::chrono::steady_clock;
	using Duration= std::chrono::duration<double>;

	// Held in the clock's own duration so the deadline and Clock::now() share a type
	const Clock::duration interval= std::chrono::duration_cast<Clock::duration>(Duration(1.0 / m_transmitRateHz));
	Clock::time_point nextWakeUp= Clock::now() + interval;

	while (!m_stopRequested.load())
	{
		// Snapshot every known universe under lock. A universe only appears here once
		// something has written it, so an idle project still sends nothing at all.
		std::map<uint16_t, UniverseBuffer> snapshot;
		{
			std::lock_guard<std::mutex> lock(m_bufferMutex);

			snapshot= m_universeBuffers;
		}

		// Transmit outside the lock
		for (auto& [universe, buf] : snapshot)
		{
			transmitUniverse(universe, buf);

			// Write the advanced sequence number back
			{
				std::lock_guard<std::mutex> lock(m_bufferMutex);

				m_universeBuffers[universe].sequenceNumber= buf.sequenceNumber;
			}
		}

		std::this_thread::sleep_until(nextWakeUp);

		// Re-base after a stall rather than chasing a deadline already in the past,
		// which would spin the thread flat out trying to catch up frames nobody wants
		nextWakeUp= std::max(nextWakeUp + interval, Clock::now());
	}
}
