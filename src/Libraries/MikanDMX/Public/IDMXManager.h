#pragma once

#include "DMXManagerConfig.h"
#include "MikanDMXExport.h"

#include <cstdint>
#include <memory>

/// Pure interface for sending DMX data via E1.31 (sACN) over UDP multicast.
/// Only supports sending — no receive capability.
class IDMXManager
{
public:
	IDMXManager()= default;
	virtual ~IDMXManager()= default;

	// Lifecycle
	virtual bool startup(const DMXManagerConfig& config)= 0;
	virtual void shutdown()= 0;
	virtual bool getIsRunning() const= 0;

	// Restart with a new config (e.g., when the user changes the network interface IP).
	// Equivalent to shutdown() + startup(newConfig).
	virtual bool restart(const DMXManagerConfig& config)= 0;

	/// Overwrite a contiguous range of channels within a universe.
	/// @param universe     E1.31 universe number (1–63999)
	/// @param startChannel 1-based DMX512 start channel (1–512)
	/// @param values       Pointer to channel values
	/// @param count        Number of channels to write; startChannel + count - 1 must be <= 512
	virtual void setChannels(
		uint16_t universe,
		uint16_t startChannel,
		const uint8_t* values,
		uint16_t count)= 0;

	/// Overwrite the full 512-slot buffer for a universe.
	/// Zeros out any slots beyond slotCount.
	/// @param universe   E1.31 universe number (1–63999)
	/// @param slotData   Pointer to at least slotCount bytes
	/// @param slotCount  Number of valid bytes in slotData (1–512)
	virtual void setUniverseData(
		uint16_t universe,
		const uint8_t* slotData,
		uint16_t slotCount)= 0;

	/// Factory — creates the concrete E1.31 implementation.
	static std::unique_ptr<IDMXManager> create();
};
using IDMXManagerPtr= std::shared_ptr<IDMXManager>;
using IDMXManagerUniquePtr= std::unique_ptr<IDMXManager>;
