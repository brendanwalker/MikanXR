#pragma once

#include "IDMXManager.h"
#include "DMXSendThread.h"

/// Concrete E1.31 implementation of IDMXManager.
/// Owns a DMXSendThread that transmits dirty universe buffers at the configured rate.
class DMXManager : public IDMXManager
{
public:
	DMXManager()= default;
	virtual ~DMXManager() override;

	virtual bool startup(const DMXManagerConfig& config) override;
	virtual void shutdown() override;
	virtual bool getIsRunning() const override;
	virtual bool restart(const DMXManagerConfig& config) override;

	virtual void setChannels(
		uint16_t universe,
		uint16_t startChannel,
		const uint8_t* values,
		uint16_t count) override;

	virtual void setUniverseData(
		uint16_t universe,
		const uint8_t* slotData,
		uint16_t slotCount) override;

private:
	DMXSendThread m_sendThread;
};
