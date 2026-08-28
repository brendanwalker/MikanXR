#pragma once

#include <cstdint>
#include <functional>
#include <string>

/// Loopback TCP line socket for the automation command server.
/// Listens on 127.0.0.1 and accepts one client at a time.
/// Incoming bytes are split on '\n' (an optional trailing '\r' is stripped)
/// and each complete line is delivered through onLineReceived.
///
/// All I/O is serviced by poll() on the main thread; no background thread.
///   Windows: Winsock2   (WSAStartup handled by IXWebSocket at app startup)
///   Linux/macOS: POSIX sockets
class AutomationSocket
{
public:
#if defined(_WIN32)
	using SocketHandle= uintptr_t;
	static constexpr SocketHandle k_invalidSocket= static_cast<SocketHandle>(~0ULL);
#else
	using SocketHandle= int;
	static constexpr SocketHandle k_invalidSocket= -1;
#endif

	explicit AutomationSocket(uint16_t port);
	~AutomationSocket();

	// Non-copyable
	AutomationSocket(const AutomationSocket&)= delete;
	AutomationSocket& operator=(const AutomationSocket&)= delete;

	// ---- Callbacks ----
	std::function<void(const std::string& line)> onLineReceived; // complete line received
	std::function<void()> onClientConnected;                     // client connected
	std::function<void()> onClientDisconnected;                  // client disconnected

	/// Non-blocking: accept a pending connection and read any available lines.
	void poll();

	/// Send raw bytes to the connected client (no terminator appended).
	/// @returns false if the send failed (client disconnected)
	bool sendText(const std::string& text);

	/// @returns true when the listener socket opened successfully.
	bool isListening() const { return m_listenSocket != k_invalidSocket; }

	/// @returns true when an automation client is connected.
	bool isClientConnected() const { return m_clientSocket != k_invalidSocket; }

	/// Close listener and any connected client socket.
	void close();

private:
	/// Try to accept a pending incoming connection on the listener socket.
	void tryAccept();

	/// Read any available data from the connected client into m_readBuffer.
	/// Emits onLineReceived for each complete newline-terminated line.
	void readAvailable();

	/// Handle client disconnect: close client socket, fire onClientDisconnected,
	/// then resume listening for the next connection.
	void handleDisconnect(const char* reason= nullptr);

	uint16_t m_port;
	SocketHandle m_listenSocket= k_invalidSocket;
	SocketHandle m_clientSocket= k_invalidSocket;
	std::string m_readBuffer; // partial line accumulator
};
