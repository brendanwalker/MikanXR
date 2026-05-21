#pragma once

#include <cstdint>
#include <string>

/// RAII wrapper for a UDP socket bound for E1.31 multicast transmission.
/// Windows: Winsock2.  Linux/macOS: POSIX sockets.
class UdpMulticastSocket
{
public:
	UdpMulticastSocket();
	~UdpMulticastSocket();

	// Non-copyable
	UdpMulticastSocket(const UdpMulticastSocket&) = delete;
	UdpMulticastSocket& operator=(const UdpMulticastSocket&) = delete;

	/// Open and bind the socket.
	/// @param bindIP  Local interface IP to bind to ("0.0.0.0" for default)
	/// @returns true on success
	bool open(const std::string& bindIP);

	/// Close the socket.
	void close();

	bool isOpen() const { return m_socket != k_invalidSocket; }

	/// Send a raw buffer to the given destination IP and port (UDP).
	/// @param destIP    Destination IP string (e.g., "239.255.0.1")
	/// @param port      Destination UDP port (E1.31 = 5568)
	/// @param data      Pointer to data buffer
	/// @param length    Number of bytes to send
	/// @returns true if all bytes were sent
	bool sendTo(const std::string& destIP, uint16_t port, const void* data, int length);

private:
#if defined(_WIN32)
	using SocketHandle = uintptr_t;
	static constexpr SocketHandle k_invalidSocket = static_cast<SocketHandle>(~0);
#else
	using SocketHandle = int;
	static constexpr SocketHandle k_invalidSocket = -1;
#endif

	SocketHandle m_socket = k_invalidSocket;
};
