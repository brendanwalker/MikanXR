#pragma once

// -- includes -----
#include <cstddef>
#include <cstdint>

// RAII wrapper for a UDP socket bound to receive datagrams on a fixed local port
// from any sender (used for the ARKit pose channel - see ARKitWireProtocol.h).
// Windows: Winsock2. Linux/macOS: POSIX sockets. Modeled on
// src/Libraries/MikanDMX/Private/UdpMulticastSocket.h's RAII/socket-handle pattern,
// but receive-capable (that class is send-only) and unicast rather than multicast.
class UdpReceiveSocket
{
public:
	UdpReceiveSocket();
	~UdpReceiveSocket();

	// Non-copyable
	UdpReceiveSocket(const UdpReceiveSocket&)= delete;
	UdpReceiveSocket& operator=(const UdpReceiveSocket&)= delete;

	// Open and bind the socket to receive on the given local port (all interfaces,
	// not loopback-only - the iPhone is a remote sender on the LAN).
	// @returns true on success (false e.g. if the port is already in use)
	bool open(uint16_t port);

	// Close the socket. Safe to call from a different thread than a blocking
	// receive() call in progress - unblocks it so a worker loop can stop cleanly.
	void close();

	bool isOpen() const { return m_socket != k_invalidSocket; }

	// Sender address for a received datagram, kept platform-agnostic so this header
	// doesn't need to pull in <winsock2.h>/<netinet/in.h>.
	struct SenderAddress
	{
		uint32_t ipv4= 0; // host byte order, e.g. 0xC0A80001 = 192.168.0.1
		uint16_t port= 0;
	};

	// Blocking receive of a single UDP datagram (bounded by an internal timeout),
	// intended to be called in a loop from a dedicated worker thread. Returns false
	// on timeout, socket closure, or any error - callers should treat false as "try
	// again" (checking their own should-stop condition) rather than as fatal.
	bool receive(uint8_t* buffer, size_t bufferSize, size_t& outBytesReceived, SenderAddress& outSender);

private:
#if defined(_WIN32)
	using SocketHandle= uintptr_t;
	static constexpr SocketHandle k_invalidSocket= static_cast<SocketHandle>(~0);
#else
	using SocketHandle= int;
	static constexpr SocketHandle k_invalidSocket= -1;
#endif

	SocketHandle m_socket= k_invalidSocket;
};
