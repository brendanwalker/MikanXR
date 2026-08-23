#include "DMXManager.h"
#include "IDMXManager.h"

// -- IDMXManager factory -----
std::unique_ptr<IDMXManager> IDMXManager::create() { return std::make_unique<DMXManager>(); }

// -- DMXManager -----
DMXManager::~DMXManager() { shutdown(); }

bool DMXManager::startup(const DMXManagerConfig& config)
{
	return m_sendThread.start(config.networkInterfaceIP, config.sourceName, config.priority, config.transmitRateHz);
}

void DMXManager::shutdown() { m_sendThread.stop(); }

bool DMXManager::getIsRunning() const { return m_sendThread.isRunning(); }

bool DMXManager::restart(const DMXManagerConfig& config)
{
	m_sendThread.stop();
	return m_sendThread.start(config.networkInterfaceIP, config.sourceName, config.priority, config.transmitRateHz);
}

void DMXManager::setChannels(uint16_t universe, uint16_t startChannel, const uint8_t* values, uint16_t count)
{
	m_sendThread.setChannels(universe, startChannel, values, count);
}

void DMXManager::setUniverseData(uint16_t universe, const uint8_t* slotData, uint16_t slotCount)
{
	m_sendThread.setUniverseData(universe, slotData, slotCount);
}
