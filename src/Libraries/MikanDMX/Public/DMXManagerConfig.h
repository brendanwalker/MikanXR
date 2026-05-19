#pragma once

#include <cstdint>
#include <string>

struct DMXManagerConfig
{
	// Network interface IP to bind the UDP socket to (use "0.0.0.0" for default)
	std::string networkInterfaceIP = "0.0.0.0";

	// Human-readable source name embedded in E1.31 packets (up to 64 chars)
	std::string sourceName = "MikanXR";

	// E1.31 priority (0–200; 100 is the standard default)
	uint8_t priority = 100;

	// Transmit rate in Hz; E1.31 spec recommends <= 44 Hz
	float transmitRateHz = 44.0f;
};
