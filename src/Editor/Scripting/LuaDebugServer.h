#pragma once

#include "LuaDebugSocket.h"

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4267)  // size_t → int conversion in third-party LRDB headers
#endif
#include "lrdb/basic_server.hpp"
#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#include <cstdint>
#include <memory>

class CommonScriptContext;

/// Singleton that owns one TCP port and one LRDB debugger instance.
/// A single server can be migrated between any number of per-component
/// script contexts — call attach() to switch, detach() to unhook.
///
/// Typical workflow:
///   1. LuaDebugServer::getInstance()->startListening();  // once at app init
///   2. User selects a component → call attach(context)
///   3. In VSCode: Run → "Attach to MikanXR Lua" launch config
///   4. Set breakpoints in the .lua file and debug normally
///   5. To switch context, call attach(otherContext) — the port stays open
///
/// poll() must be called each frame from the app update loop so that
/// non-blocking I/O events (connection, data) are serviced even when
/// Lua is not actively executing (i.e., between updateScript() calls).
class LuaDebugServer
{
public:
	static LuaDebugServer* getInstance();

	/// Start listening on the given TCP port.
	/// @param port  Defaults to 21110 (LRDB VSCode extension default).
	/// @returns false if the socket could not be opened.
	bool startListening(uint16_t port = 21110);

	/// Stop listening and close any connected client.
	void stopListening();

	bool isListening() const { return m_server != nullptr; }

	/// Attach the debugger hook to the given script context's Lua state.
	/// Automatically detaches from any previously-attached context first.
	/// If context is nullptr, this is equivalent to detach().
	void attach(CommonScriptContext* context);

	/// Remove the debugger hook from the currently-attached context.
	void detach();

	bool isAttached() const { return m_attachedContext != nullptr; }

	CommonScriptContext* getAttachedContext() const { return m_attachedContext; }

	/// Call once per frame from the app update loop.
	/// Services socket I/O events (new connections, incoming commands)
	/// even when Lua is not executing.
	void poll();

private:
	LuaDebugServer() = default;

	using Server = lrdb::basic_server<LuaDebugSocket>;

	std::unique_ptr<Server> m_server;
	CommonScriptContext*    m_attachedContext = nullptr;
};
