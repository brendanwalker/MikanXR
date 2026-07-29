#include "UdpReceiveSocket.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using SockLen= int;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using SockLen= socklen_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(s) ::close(s)
#endif

#include <cstring>

namespace
{
// Generous receive buffer: an undersized kernel socket buffer silently drops
// datagrams under load rather than surfacing an error.
constexpr int kReceiveBufferBytes= 1 << 20; // 1 MB

// Bounded so a worker thread blocked in receive() stays responsive to a
// should-stop flag instead of only being unblockable via close() from another
// thread.
constexpr int kReceiveTimeoutMs= 100;

#if defined(_WIN32)
// UdpMulticastSocket (MikanDMX) skips WSAStartup, relying on IXWebSocket having
// already initialized Winsock at editor process lifetime scope. This class is
// also used directly from lightweight standalone executables (e.g. the unit
// test suite) that don't link IXWebSocket, so it can't rely on that assumption
// - it initializes Winsock itself instead. WSAStartup is safe to call more than
// once per process (refcounted by Windows), so this doesn't conflict with
// IXWebSocket also calling it.
void ensureWinsockInitialized()
{
	static const int result= []
	{
		WSADATA wsaData;
		return ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	}();
	(void)result;
}
#endif
} // namespace

UdpReceiveSocket::UdpReceiveSocket()
	: m_socket(k_invalidSocket)
{
}

UdpReceiveSocket::~UdpReceiveSocket() { close(); }

bool UdpReceiveSocket::open(uint16_t port)
{
	close();

#if defined(_WIN32)
	ensureWinsockInitialized();
#endif

	m_socket= static_cast<SocketHandle>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
	if (m_socket == k_invalidSocket)
	{
		return false;
	}

	// Deliberately NOT setting SO_REUSEADDR here (unlike UdpMulticastSocket): on
	// Windows it doesn't just permit fast rebind after close, it lets a second,
	// unrelated socket silently bind to the exact same address:port that's already
	// in active use (a well-known Windows-specific footgun - Microsoft added
	// SO_EXCLUSIVEADDRUSE specifically to counter it). This class needs a genuine
	// "already in use" bind failure so a duplicate ARKit video source configured
	// with a colliding port is caught, not silently stealing/splitting traffic.

	int rcvBuf= kReceiveBufferBytes;
	::setsockopt(static_cast<SOCKET>(m_socket), SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&rcvBuf),
				 sizeof(rcvBuf));

#if defined(_WIN32)
	DWORD timeoutMs= static_cast<DWORD>(kReceiveTimeoutMs);
	::setsockopt(static_cast<SOCKET>(m_socket), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs),
				 sizeof(timeoutMs));
#else
	timeval timeout{};
	timeout.tv_sec= kReceiveTimeoutMs / 1000;
	timeout.tv_usec= (kReceiveTimeoutMs % 1000) * 1000;
	::setsockopt(static_cast<SOCKET>(m_socket), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout),
				 sizeof(timeout));
#endif

	// Bind on all interfaces, not INADDR_LOOPBACK - the iPhone is a remote sender
	// on the LAN, not a same-machine process.
	sockaddr_in bindAddr{};
	bindAddr.sin_family= AF_INET;
	bindAddr.sin_port= htons(port);
	bindAddr.sin_addr.s_addr= INADDR_ANY;

	if (::bind(static_cast<SOCKET>(m_socket), reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
	{
		close();
		return false;
	}

	return true;
}

void UdpReceiveSocket::close()
{
	if (m_socket != k_invalidSocket)
	{
		::closesocket(static_cast<SOCKET>(m_socket));
		m_socket= k_invalidSocket;
	}
}

bool UdpReceiveSocket::receive(uint8_t* buffer, size_t bufferSize, size_t& outBytesReceived, SenderAddress& outSender)
{
	outBytesReceived= 0;

	if (m_socket == k_invalidSocket || buffer == nullptr || bufferSize == 0)
		return false;

	sockaddr_in senderAddr{};
	SockLen senderAddrLen= sizeof(senderAddr);

	const int received=
		::recvfrom(static_cast<SOCKET>(m_socket), reinterpret_cast<char*>(buffer), static_cast<int>(bufferSize), 0,
				   reinterpret_cast<sockaddr*>(&senderAddr), &senderAddrLen);

	if (received == SOCKET_ERROR || received < 0)
		return false;

	outBytesReceived= static_cast<size_t>(received);
	outSender.ipv4= ntohl(senderAddr.sin_addr.s_addr);
	outSender.port= ntohs(senderAddr.sin_port);

	return true;
}
