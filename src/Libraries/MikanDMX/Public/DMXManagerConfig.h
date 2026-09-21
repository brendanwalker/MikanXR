#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct DMXManagerConfig
{
	// Network interface IP to bind the UDP socket to (use "0.0.0.0" for default)
	std::string networkInterfaceIP= "0.0.0.0";

	// Human-readable source name embedded in E1.31 packets (up to 64 chars)
	std::string sourceName= "MikanXR";

	// E1.31 priority (0–200; 100 is the standard default)
	uint8_t priority= 100;

	// Transmit rate in Hz; E1.31 spec recommends <= 44 Hz
	float transmitRateHz= 44.0f;

	// Unicast destinations keyed by universe. A universe listed here is sent to each of
	// its addresses instead of to a multicast group, which is what reaches a controller
	// an access point will not forward multicast to. A universe absent from the map goes
	// to its 239.255.x.y group, the E1.31 default and the right choice on a wired LAN.
	std::map<uint16_t, std::vector<std::string>> universeDestinations;
};
