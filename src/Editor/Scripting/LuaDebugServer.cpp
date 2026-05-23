#include "LuaDebugServer.h"
#include "CommonScriptContext.h"
#include "Logger.h"

// ---- Singleton -------------------------------------------------------------

LuaDebugServer* LuaDebugServer::getInstance()
{
    static LuaDebugServer s_instance;
    return &s_instance;
}

// ---- Lifecycle -------------------------------------------------------------

bool LuaDebugServer::startListening(uint16_t port)
{
    if (m_server)
    {
        MIKAN_LOG_WARNING("LuaDebugServer") << "Already listening";
        return true;
    }

    // Construct the server — this creates the LuaDebugSocket listener.
    m_server = std::make_unique<Server>(port);

    // Disable blocking wait-for-connection so the editor's main loop is never
    // stalled.  Breakpoints set in VSCode still work — the LRDB tick handler
    // calls poll() on every Lua line event, so commands are processed as
    // the script runs.
    m_server->set_wait_for_connection(false);

    MIKAN_LOG_INFO("LuaDebugServer") << "Lua debug server started on port " << port;
    return true;
}

void LuaDebugServer::stopListening()
{
    if (!m_server) return;

    detach();
    m_server.reset();
    MIKAN_LOG_INFO("LuaDebugServer") << "Lua debug server stopped";
}

// ---- Attach / Detach -------------------------------------------------------

void LuaDebugServer::attach(CommonScriptContext* context)
{
    if (!m_server)
    {
        MIKAN_LOG_ERROR("LuaDebugServer") << "Call startListening() before attach()";
        return;
    }

    // Detach from current context first (resets the Lua hook)
    if (m_attachedContext)
    {
        m_server->reset(nullptr);
        MIKAN_LOG_INFO("LuaDebugServer") << "Lua debugger detached from previous context";
    }

    m_attachedContext = context;

    if (context && context->hasLoadedScript())
    {
        m_server->reset(context->getLuaState());
        MIKAN_LOG_INFO("LuaDebugServer") << "Lua debugger attached to script context";
    }
    else
    {
        m_attachedContext = nullptr;
        if (context)
            MIKAN_LOG_WARNING("LuaDebugServer") << "Context has no loaded script — nothing to attach to";
    }
}

void LuaDebugServer::detach()
{
    if (m_server && m_attachedContext)
    {
        m_server->reset(nullptr);
        MIKAN_LOG_INFO("LuaDebugServer") << "Lua debugger detached";
    }
    m_attachedContext = nullptr;
}

// ---- Programmatic break ----------------------------------------------------

void LuaDebugServer::pauseOnNextLine()
{
    if (m_server && m_attachedContext)
        m_server->pause();
}

// ---- Per-frame polling -----------------------------------------------------

void LuaDebugServer::poll()
{
    if (!m_server) return;

    // Service any pending socket I/O (new connections, incoming debugger
    // commands) without blocking.  When Lua IS executing, the LRDB tick
    // handler calls LuaDebugSocket::poll() automatically on every line
    // event.  This external poll() covers the gaps between Lua updates.
    m_server->command_stream().poll();
}
