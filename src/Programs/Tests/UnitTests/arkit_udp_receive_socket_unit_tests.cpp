//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <chrono>
#include <cstring>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "UdpReceiveSocket.h"
#include "unit_test.h"

// These tests exercise UdpReceiveSocket from "outside" using a plain platform UDP
// socket as an independent sender - not just calling UdpReceiveSocket's own API in
// both directions - since the class under test is receive-only. A genuinely remote
// (different-machine) sender can't be automated here; the INADDR_ANY-not-loopback
// binding test below is the closest automatable proxy for that edge case.
namespace
{
#if defined(_WIN32)
// This test file opens its own raw sender socket independent of
// UdpReceiveSocket's internal Winsock init, so it needs the same guard rather
// than depending on test execution order having already triggered it.
void ensureWinsockInitializedForTest()
{
	static const int result= []
	{
		WSADATA wsaData;
		return ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	}();
	(void)result;
}
#endif

// Sends `data` to 127.0.0.1:port from a short-lived plain UDP socket.
bool sendLoopbackDatagram(uint16_t port, const uint8_t* data, size_t length)
{
#if defined(_WIN32)
	ensureWinsockInitializedForTest();

	SOCKET sock= ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
		return false;

	sockaddr_in dest{};
	dest.sin_family= AF_INET;
	dest.sin_port= htons(port);
	::inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

	const int sent= ::sendto(sock, reinterpret_cast<const char*>(data), static_cast<int>(length), 0,
							 reinterpret_cast<const sockaddr*>(&dest), sizeof(dest));
	::closesocket(sock);

	return sent == static_cast<int>(length);
#else
	return false;
#endif
}

// A port unlikely to collide with other services/tests running concurrently.
uint16_t testPort(uint16_t offset) { return static_cast<uint16_t>(41100 + offset); }
} // namespace

//-- private functions -----
static bool arkit_udp_receive_socket_test_open_close()
{
	UNIT_TEST_BEGIN("open/close lifecycle")

	UdpReceiveSocket socket;
	success= !socket.isOpen();
	assert(success);

	success= success && socket.open(testPort(0));
	assert(success);
	success= success && socket.isOpen();
	assert(success);

	socket.close();
	success= success && !socket.isOpen();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_udp_receive_socket_test_receives_exact_bytes()
{
	UNIT_TEST_BEGIN("receives exact bytes sent over loopback")

	UdpReceiveSocket socket;
	success= socket.open(testPort(1));
	assert(success);

	const uint8_t payload[]= {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01, 0x02, 0x03};
	success= success && sendLoopbackDatagram(testPort(1), payload, sizeof(payload));
	assert(success);

	uint8_t buffer[64]= {};
	size_t bytesReceived= 0;
	UdpReceiveSocket::SenderAddress sender;

	// Retry briefly: the datagram can legitimately race the receive() call even on
	// loopback, and a single 100ms internal timeout window could miss it.
	bool received= false;
	for (int attempt= 0; !received && attempt < 20; ++attempt)
		received= socket.receive(buffer, sizeof(buffer), bytesReceived, sender);

	success= success && received;
	assert(success);
	success= success && (bytesReceived == sizeof(payload));
	assert(success);
	success= success && (std::memcmp(buffer, payload, sizeof(payload)) == 0);
	assert(success);

	// 127.0.0.1 in host byte order.
	success= success && (sender.ipv4 == 0x7F000001);
	assert(success);
	success= success && (sender.port != 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_udp_receive_socket_test_receives_multiple_datagrams_in_order()
{
	UNIT_TEST_BEGIN("receives multiple sequential datagrams")

	UdpReceiveSocket socket;
	success= socket.open(testPort(2));
	assert(success);

	const int kCount= 5;
	for (uint8_t i= 0; i < kCount; ++i)
	{
		const uint8_t payload[]= {i, static_cast<uint8_t>(i + 1)};
		success= success && sendLoopbackDatagram(testPort(2), payload, sizeof(payload));
		assert(success);
	}

	for (uint8_t i= 0; success && i < kCount; ++i)
	{
		uint8_t buffer[16]= {};
		size_t bytesReceived= 0;
		UdpReceiveSocket::SenderAddress sender;

		bool received= false;
		for (int attempt= 0; !received && attempt < 20; ++attempt)
			received= socket.receive(buffer, sizeof(buffer), bytesReceived, sender);

		success= received && bytesReceived == 2 && buffer[0] == i && buffer[1] == static_cast<uint8_t>(i + 1);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

static bool arkit_udp_receive_socket_test_rejects_oversized_datagram_gracefully()
{
	UNIT_TEST_BEGIN("small receive buffer does not overflow on a larger datagram")

	UdpReceiveSocket socket;
	success= socket.open(testPort(3));
	assert(success);

	std::vector<uint8_t> payload(2000, 0xAB);
	success= success && sendLoopbackDatagram(testPort(3), payload.data(), payload.size());
	assert(success);

	// A buffer smaller than the datagram: Winsock's actual UDP behavior is to fail
	// the call with WSAEMSGSIZE (the OS may still write the first part of the
	// message into the buffer before reporting the error - that part is
	// Windows-internal/undocumented-exactly, not this class's contract). What this
	// class guarantees is: no crash, no write past bufferSize (recvfrom is always
	// called with the real buffer size, so the OS itself can't overrun it), and a
	// `false` return with outBytesReceived left at 0 rather than silently reporting
	// a wrong "success".
	uint8_t smallBuffer[16]= {};
	size_t bytesReceived= 0;
	UdpReceiveSocket::SenderAddress sender;

	const bool received= socket.receive(smallBuffer, sizeof(smallBuffer), bytesReceived, sender);

	success= success && (received == false);
	assert(success);
	success= success && (bytesReceived == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_udp_receive_socket_test_receive_times_out_without_hanging()
{
	UNIT_TEST_BEGIN("receive() times out instead of blocking forever when idle")

	UdpReceiveSocket socket;
	success= socket.open(testPort(4));
	assert(success);

	uint8_t buffer[16]= {};
	size_t bytesReceived= 0;
	UdpReceiveSocket::SenderAddress sender;

	const auto start= std::chrono::steady_clock::now();
	const bool received= socket.receive(buffer, sizeof(buffer), bytesReceived, sender);
	const auto elapsed= std::chrono::steady_clock::now() - start;

	success= (received == false);
	assert(success);

	// Generous upper bound purely to prove this isn't an indefinite block; the
	// internal timeout is 100ms.
	success= success && (elapsed < std::chrono::seconds(5));
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_udp_receive_socket_test_rejects_duplicate_port_binding()
{
	UNIT_TEST_BEGIN("opening an already-bound port fails cleanly, not a crash")

	UdpReceiveSocket first;
	success= first.open(testPort(5));
	assert(success);

	UdpReceiveSocket second;
	success= success && (second.open(testPort(5)) == false);
	assert(success);
	success= success && !second.isOpen();
	assert(success);

	// The first socket should be unaffected by the failed second open() attempt.
	success= success && first.isOpen();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_udp_receive_socket_test_binds_all_interfaces_not_loopback_only()
{
	UNIT_TEST_BEGIN("binds INADDR_ANY, not loopback-only (remote senders must be receivable)")

#if defined(_WIN32)
	UdpReceiveSocket socket;
	success= socket.open(testPort(6));
	assert(success);

	// UdpReceiveSocket doesn't expose its raw handle, so reach in via a second
	// direct bind attempt on the *same port* restricted to loopback-only: if the
	// class under test had bound to INADDR_LOOPBACK instead of INADDR_ANY, this
	// wouldn't collide (different bind address, same port is allowed), and this
	// assertion would fail to catch the regression it's meant to catch - so the
	// real check is the inverse: attempting to bind 0.0.0.0 again on the same port
	// must fail, since that's only true if the class under test already holds the
	// wildcard binding.
	SOCKET probe= ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	success= success && (probe != INVALID_SOCKET);
	assert(success);

	sockaddr_in wildcardAddr{};
	wildcardAddr.sin_family= AF_INET;
	wildcardAddr.sin_port= htons(testPort(6));
	wildcardAddr.sin_addr.s_addr= INADDR_ANY;

	const int bindResult= ::bind(probe, reinterpret_cast<sockaddr*>(&wildcardAddr), sizeof(wildcardAddr));
	success= success && (bindResult == SOCKET_ERROR);
	assert(success);

	::closesocket(probe);
#else
	success= true; // Not exercised on non-Windows targets (see UdpReceiveSocket.cpp).
#endif

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_udp_receive_socket_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_udp_receive_socket")
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_open_close);
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_receives_exact_bytes);
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_receives_multiple_datagrams_in_order);
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_rejects_oversized_datagram_gracefully);
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_receive_times_out_without_hanging);
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_rejects_duplicate_port_binding);
	UNIT_TEST_MODULE_CALL_TEST(arkit_udp_receive_socket_test_binds_all_interfaces_not_loopback_only);
	UNIT_TEST_MODULE_END()
}
