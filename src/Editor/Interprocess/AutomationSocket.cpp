#include "AutomationSocket.h"
#include "Logger.h"

// ---- Platform socket setup -------------------------------------------------
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using SockLen= int;
// WSAStartup is handled by IXWebSocket at application startup.
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
using SockLen= socklen_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SOCKET int
#define closesocket(s) ::close(s)
#endif

#include <cstring>

// ---- Helpers ---------------------------------------------------------------

static void setNonBlocking(AutomationSocket::SocketHandle sock)
{
#if defined(_WIN32)
	u_long mode= 1;
	ioctlsocket(static_cast<SOCKET>(sock), FIONBIO, &mode);
#else
	int flags= fcntl(static_cast<int>(sock), F_GETFL, 0);
	fcntl(static_cast<int>(sock), F_SETFL, flags | O_NONBLOCK);
#endif
}

// ---- AutomationSocket ------------------------------------------------------

AutomationSocket::AutomationSocket(uint16_t port)
	: m_port(port)
{
	// Create TCP listener socket
	SOCKET listener= ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET)
	{
		MIKAN_LOG_ERROR("AutomationSocket") << "Failed to create listener socket";
		return;
	}

	// Allow rapid reuse of the port after restart
	int optval= 1;
	::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval));

	// Loopback only: the automation channel is a local debug surface
	sockaddr_in addr{};
	addr.sin_family= AF_INET;
	addr.sin_addr.s_addr= htonl(INADDR_LOOPBACK);
	addr.sin_port= htons(port);

	if (::bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR
		|| ::listen(listener, 1) == SOCKET_ERROR)
	{
		MIKAN_LOG_ERROR("AutomationSocket") << "Failed to bind/listen on port " << port;
		closesocket(listener);
		return;
	}

	setNonBlocking(listener);
	m_listenSocket= static_cast<SocketHandle>(listener);
	MIKAN_LOG_INFO("AutomationSocket") << "Listening for automation clients on port " << port;
}

AutomationSocket::~AutomationSocket() { close(); }

void AutomationSocket::close()
{
	if (m_clientSocket != k_invalidSocket)
	{
		// Half-close the send side first so a reply still in flight (the
		// app quit acknowledgement) reaches the client instead of being
		// discarded by a hard reset
#if defined(_WIN32)
		::shutdown(static_cast<SOCKET>(m_clientSocket), SD_SEND);
#else
		::shutdown(static_cast<SOCKET>(m_clientSocket), SHUT_WR);
#endif
		closesocket(static_cast<SOCKET>(m_clientSocket));
		m_clientSocket= k_invalidSocket;
	}
	if (m_listenSocket != k_invalidSocket)
	{
		closesocket(static_cast<SOCKET>(m_listenSocket));
		m_listenSocket= k_invalidSocket;
	}
}

// ---- Internal helpers ------------------------------------------------------

void AutomationSocket::tryAccept()
{
	SOCKET listener= static_cast<SOCKET>(m_listenSocket);
	sockaddr_in clientAddr{};
	SockLen addrLen= sizeof(clientAddr);
	SOCKET client= ::accept(listener, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
	if (client == INVALID_SOCKET)
		return;

	setNonBlocking(client);
	m_clientSocket= static_cast<SocketHandle>(client);
	MIKAN_LOG_INFO("AutomationSocket") << "Automation client connected";

	if (onClientConnected)
		onClientConnected();
}

void AutomationSocket::readAvailable()
{
	char buf[1024];
	SOCKET client= static_cast<SOCKET>(m_clientSocket);

	while (true)
	{
		int n= ::recv(client, buf, static_cast<int>(sizeof(buf)), 0);
		if (n > 0)
		{
			m_readBuffer.append(buf, static_cast<size_t>(n));

			// Emit onLineReceived for each complete newline-terminated line
			size_t pos;
			while ((pos= m_readBuffer.find('\n')) != std::string::npos)
			{
				std::string line= m_readBuffer.substr(0, pos);
				m_readBuffer.erase(0, pos + 1);

				// Strip trailing '\r' if present
				if (!line.empty() && line.back() == '\r')
					line.pop_back();

				MIKAN_LOG_DEBUG("AutomationSocket") << "RX: " << line;
				if (onLineReceived)
					onLineReceived(line);
			}
		}
		else if (n == 0)
		{
			// Orderly shutdown
			handleDisconnect("client closed connection");
			break;
		}
		else
		{
#if defined(_WIN32)
			int err= WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
				break; // no more data right now
#else
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
#endif
			handleDisconnect("recv error");
			break;
		}
	}
}

void AutomationSocket::handleDisconnect(const char* reason)
{
	if (m_clientSocket == k_invalidSocket)
		return;

	if (reason)
		MIKAN_LOG_INFO("AutomationSocket") << "Automation client disconnected: " << reason;

	closesocket(static_cast<SOCKET>(m_clientSocket));
	m_clientSocket= k_invalidSocket;
	m_readBuffer.clear();

	if (onClientDisconnected)
		onClientDisconnected();
}

// ---- Public interface ------------------------------------------------------

void AutomationSocket::poll()
{
	if (m_listenSocket == k_invalidSocket)
		return;

	if (m_clientSocket == k_invalidSocket)
		tryAccept();

	if (m_clientSocket != k_invalidSocket)
		readAvailable();
}

bool AutomationSocket::sendText(const std::string& text)
{
	if (!isClientConnected())
		return false;

	SOCKET client= static_cast<SOCKET>(m_clientSocket);

	int sent= ::send(client, text.c_str(), static_cast<int>(text.size()), 0);
	if (sent == SOCKET_ERROR)
	{
		handleDisconnect("send error");
		return false;
	}
	return true;
}
