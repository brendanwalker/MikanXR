#pragma once

#include <cstdint>
#include <functional>
#include <string>

/// TCP command stream for lrdb::basic_server<LuaDebugSocket>.
/// Replaces lrdb/command_stream/socket.hpp without requiring ASIO.
/// Listens on a fixed port and accepts one client at a time.
/// Protocol: newline-delimited JSON-RPC 2.0 messages ("\r\n" terminated).
///
/// Follows the UdpMulticastSocket RAII pattern:
///   Windows: Winsock2   (WSAStartup handled by IXWebSocket at app startup)
///   Linux/macOS: POSIX sockets
class LuaDebugSocket
{
public:
#if defined(_WIN32)
	using SocketHandle= uintptr_t;
	static constexpr SocketHandle k_invalidSocket= static_cast<SocketHandle>(~0ULL);
#else
	using SocketHandle= int;
	static constexpr SocketHandle k_invalidSocket= -1;
#endif

	explicit LuaDebugSocket(uint16_t port= 21110);
	~LuaDebugSocket();

	// Non-copyable
	LuaDebugSocket(const LuaDebugSocket&)= delete;
	LuaDebugSocket& operator=(const LuaDebugSocket&)= delete;

	// ---- Callbacks (set by lrdb::basic_server) ----
	std::function<void(const std::string& data)> on_data; // complete line received
	std::function<void()> on_connection;                  // client connected
	std::function<void()> on_close;                       // client disconnected
	std::function<void(const std::string&)> on_error;     // error occurred

	// ---- Methods required by lrdb::basic_server ----

	/// Non-blocking: service any available I/O events.
	/// Called by the debugger's tick handler every Lua line event.
	void poll();

	/// Blocking: wait for and service exactly one I/O event.
	/// Called by the debugger's pause handler while execution is halted
	/// at a breakpoint.
	void run_one();

	/// Block until a client connects (called once on startup if
	/// wait_for_connect_ is set in basic_server).
	void wait_for_connection();

	/// Send a message to the connected client.
	/// Appends "\r\n" as required by the LRDB protocol.
	/// @returns false if the send failed (client disconnected)
	bool send_message(const std::string& message);

	/// @returns true when a debugger client is connected.
	bool is_open() const;

	/// Close listener and any connected client socket.
	void close();

private:
	/// Try to accept a pending incoming connection on the listener socket.
	void tryAccept();

	/// Read any available data from the connected client into m_readBuffer.
	/// Emits on_data for each complete newline-terminated line.
	void readAvailable();

	/// Handle client disconnect: close client socket, fire on_close,
	/// then resume listening for the next connection.
	void handleDisconnect(const char* reason= nullptr);

	/// Service whichever sockets are readable according to the given
	/// select() result. Returns true if any event was processed.
	bool dispatchSelect(bool blocking);

	uint16_t m_port;
	SocketHandle m_listenSocket= k_invalidSocket;
	SocketHandle m_clientSocket= k_invalidSocket;
	std::string m_readBuffer; // partial line accumulator
};
