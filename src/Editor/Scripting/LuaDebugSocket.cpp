#include "LuaDebugSocket.h"
#include "Logger.h"

// ---- Platform socket setup -------------------------------------------------
#if defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "Ws2_32.lib")
using SockLen = int;
// WSAStartup is handled by IXWebSocket at application startup.
#else
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <fcntl.h>
using SockLen = socklen_t;
#   define INVALID_SOCKET  (-1)
#   define SOCKET_ERROR    (-1)
#   define SOCKET          int
#   define closesocket(s)  ::close(s)
#endif

#include <cstring>

// ---- Helpers ---------------------------------------------------------------

static void setNonBlocking(LuaDebugSocket::SocketHandle sock)
{
#if defined(_WIN32)
	u_long mode = 1;
	ioctlsocket(static_cast<SOCKET>(sock), FIONBIO, &mode);
#else
	int flags = fcntl(static_cast<int>(sock), F_GETFL, 0);
	fcntl(static_cast<int>(sock), F_SETFL, flags | O_NONBLOCK);
#endif
}

// ---- LuaDebugSocket --------------------------------------------------------

LuaDebugSocket::LuaDebugSocket(uint16_t port)
	: m_port(port)
{
	// Create TCP listener socket
	SOCKET listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET)
	{
		MIKAN_LOG_ERROR("LuaDebugSocket") << "Failed to create listener socket";
		return;
	}

	// Allow rapid reuse of the port after restart
	int optval = 1;
	::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
		reinterpret_cast<const char*>(&optval), sizeof(optval));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (::bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR ||
		::listen(listener, 1) == SOCKET_ERROR)
	{
		MIKAN_LOG_ERROR("LuaDebugSocket") << "Failed to bind/listen on port " << port;
		closesocket(listener);
		return;
	}

	setNonBlocking(listener);
	m_listenSocket = static_cast<SocketHandle>(listener);
	MIKAN_LOG_INFO("LuaDebugSocket") << "Listening for Lua debugger on port " << port;
}

LuaDebugSocket::~LuaDebugSocket()
{
	close();
}

void LuaDebugSocket::close()
{
	if (m_clientSocket != k_invalidSocket)
	{
		closesocket(static_cast<SOCKET>(m_clientSocket));
		m_clientSocket = k_invalidSocket;
	}
	if (m_listenSocket != k_invalidSocket)
	{
		closesocket(static_cast<SOCKET>(m_listenSocket));
		m_listenSocket = k_invalidSocket;
	}
}

bool LuaDebugSocket::is_open() const
{
	return m_clientSocket != k_invalidSocket;
}

// ---- Internal helpers ------------------------------------------------------

void LuaDebugSocket::tryAccept()
{
	SOCKET listener = static_cast<SOCKET>(m_listenSocket);
	sockaddr_in clientAddr{};
	SockLen addrLen = sizeof(clientAddr);
	SOCKET client = ::accept(listener,
		reinterpret_cast<sockaddr*>(&clientAddr),
		&addrLen);
	if (client == INVALID_SOCKET) return;

	setNonBlocking(client);
	m_clientSocket = static_cast<SocketHandle>(client);
	MIKAN_LOG_INFO("LuaDebugSocket") << "Lua debugger client connected";

	if (on_connection) on_connection();
}

void LuaDebugSocket::readAvailable()
{
	char buf[1024];
	SOCKET client = static_cast<SOCKET>(m_clientSocket);

	while (true)
	{
		int n = ::recv(client, buf, static_cast<int>(sizeof(buf)), 0);
		if (n > 0)
		{
			m_readBuffer.append(buf, static_cast<size_t>(n));

			// Emit on_data for each complete newline-terminated line
			size_t pos;
			while ((pos = m_readBuffer.find('\n')) != std::string::npos)
			{
				std::string line = m_readBuffer.substr(0, pos);
				m_readBuffer.erase(0, pos + 1);

				// Strip trailing '\r' if present
				if (!line.empty() && line.back() == '\r')
					line.pop_back();

				MIKAN_LOG_DEBUG("LuaDebugSocket") << "RX: " << line;
				if (on_data) on_data(line);
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
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) break;   // no more data right now
#else
			if (errno == EWOULDBLOCK || errno == EAGAIN) break;
#endif
			handleDisconnect("recv error");
			break;
		}
	}
}

void LuaDebugSocket::handleDisconnect(const char* reason)
{
	if (m_clientSocket == k_invalidSocket) return;

	if (reason)
		MIKAN_LOG_INFO("LuaDebugSocket") << "Lua debugger disconnected: " << reason;

	closesocket(static_cast<SOCKET>(m_clientSocket));
	m_clientSocket = k_invalidSocket;
	m_readBuffer.clear();

	if (on_close) on_close();
}

bool LuaDebugSocket::dispatchSelect(bool blocking)
{
	if (m_listenSocket == k_invalidSocket) return false;

	fd_set readfds;
	FD_ZERO(&readfds);

	SOCKET listener = static_cast<SOCKET>(m_listenSocket);
	FD_SET(listener, &readfds);

	SOCKET client = INVALID_SOCKET;
	if (m_clientSocket != k_invalidSocket)
	{
		client = static_cast<SOCKET>(m_clientSocket);
		FD_SET(client, &readfds);
	}

#if defined(_WIN32)
	int nfds = 0;  // ignored on Windows
#else
	int nfds = static_cast<int>(std::max(m_listenSocket, m_clientSocket)) + 1;
#endif

	timeval zero{ 0, 0 };
	timeval* timeout = blocking ? nullptr : &zero;

	int ready = ::select(nfds, &readfds, nullptr, nullptr, timeout);
	if (ready <= 0) return false;

	bool processed = false;

	if (FD_ISSET(listener, &readfds))
	{
		tryAccept();
		processed = true;
	}

	if (client != INVALID_SOCKET && FD_ISSET(client, &readfds))
	{
		readAvailable();
		processed = true;
	}

	return processed;
}

// ---- Public interface (lrdb::basic_server contract) ------------------------

void LuaDebugSocket::poll()
{
	dispatchSelect(/*blocking=*/false);
}

void LuaDebugSocket::run_one()
{
	dispatchSelect(/*blocking=*/true);
}

void LuaDebugSocket::wait_for_connection()
{
	while (!is_open())
		dispatchSelect(/*blocking=*/true);
}

bool LuaDebugSocket::send_message(const std::string& message)
{
	if (!is_open()) return false;

	MIKAN_LOG_DEBUG("LuaDebugSocket") << "TX: " << message;

	std::string data = message + "\r\n";
	SOCKET client = static_cast<SOCKET>(m_clientSocket);

	int sent = ::send(client, data.c_str(), static_cast<int>(data.size()), 0);
	if (sent == SOCKET_ERROR)
	{
		handleDisconnect("send error");
		return false;
	}
	return true;
}
