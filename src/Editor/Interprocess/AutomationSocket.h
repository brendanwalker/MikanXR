#pragma once

#include <cstdint>
#include <functional>
#include <string>

/// TCP line socket, accepting one client at a time.
/// Incoming bytes are split on '\n' (an optional trailing '\r' is stripped)
/// and each complete line is delivered through onLineReceived.
///
/// All I/O is serviced by poll() on the main thread; no background thread.
///   Windows: Winsock2   (WSAStartup handled by IXWebSocket at app startup)
///   Linux/macOS: POSIX sockets
class AutomationSocket
{
public:
	/// Which interfaces the listener binds. The automation command server is
	/// loopback only and must stay that way: it is a local debug surface that
	/// drives the editor. anyInterface exists for the ARKit debug channel,
	/// whose peer is a phone on the LAN, and is opt-in for that reason.
	enum class eBindScope
	{
		loopback,
		anyInterface
	};

#if defined(_WIN32)
	using SocketHandle= uintptr_t;
	static constexpr SocketHandle k_invalidSocket= static_cast<SocketHandle>(~0ULL);
#else
	using SocketHandle= int;
	static constexpr SocketHandle k_invalidSocket= -1;
#endif

	/// channelName labels this socket's log lines, so two channels sharing this
	/// class stay distinguishable in the log they both write to.
	explicit AutomationSocket(uint16_t port, eBindScope bindScope= eBindScope::loopback,
							  const std::string& channelName= "Automation");
	~AutomationSocket();

	/// A peer that never sends a newline would otherwise grow the read buffer
	/// without bound. Matters more since eBindScope::anyInterface exists.
	static const size_t k_maxBufferedLineBytes= 64 * 1024;

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

	/// @returns true when a client is connected.
	bool isClientConnected() const { return m_clientSocket != k_invalidSocket; }

	/// Dotted-quad address of the connected client, empty when none is connected.
	const std::string& getClientAddress() const { return m_clientAddress; }

	/// Drop the connected client but keep listening for the next one.
	/// Used to reject a peer that fails the protocol handshake.
	void disconnectClient(const char* reason);

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
	std::string m_channelName;
	SocketHandle m_listenSocket= k_invalidSocket;
	SocketHandle m_clientSocket= k_invalidSocket;
	std::string m_clientAddress;
	std::string m_readBuffer; // partial line accumulator
};
