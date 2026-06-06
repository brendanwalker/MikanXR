#include "CommonScriptContext.h"
#include "CompositorConstants.h"
#include "LuaDebugServer.h"
#include "MathGLM.h"
#include "LuaMath.h"
#include "Logger.h"

#include <algorithm>
#include <assert.h>
#include <fstream>
#include <filesystem>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "easy/profiler.h"

// -- CommonScriptContext -----
CommonScriptContext::CommonScriptContext()
	: m_luaState(nullptr)
{

}

CommonScriptContext::~CommonScriptContext()
{
	disposeScriptState();
}

int CommonScriptContext::panicHandler(lua_State* state)
{
	const char* err = lua_tostring(state, 1);
	MIKAN_LOG_ERROR("CommonScriptContext::panicHandler") << err;

	return -1;
}

bool CommonScriptContext::checkLuaResult(int ret, const char* filename, int line)
{
	if (m_luaState == nullptr)
		return false;

	if (ret != 0) 
	{
		MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << filename << ", Line " << line;

		switch (ret) 
		{
		case LUA_ERRFILE:
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Couldn't open the given file";
			break;
		case LUA_ERRSYNTAX:
			{
				luaL_traceback(m_luaState, m_luaState, nullptr, 1);
				const std::string traceback = lua_tostring(m_luaState, -1);

				MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Syntax error during pre-compilation";
				MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << traceback;

			} break;
		case LUA_ERRMEM:
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Memory allocation error";
			break;
		case LUA_ERRRUN:
			{
				const std::string errMsg = lua_tostring(m_luaState, -1);
				luaL_traceback(m_luaState, m_luaState, nullptr, 1);
				const std::string traceback = lua_tostring(m_luaState, -1);

				MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << errMsg;
				MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << traceback;

			} break;
		case LUA_ERRERR:
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Error while running the error handler function";
			break;
		default:
			const std::string errMsg = lua_tostring(m_luaState, -1);
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << errMsg;
			break;
		}

		// Terminate the script state
		disposeScriptState();

		return false;
	}

	return true;
}

bool CommonScriptContext::loadScript(const std::filesystem::path& scriptPath)
{
	disposeScriptState();

	m_luaState = luaL_newstate();
	if (m_luaState == nullptr)
	{
		MIKAN_LOG_ERROR("CommonScriptContext::loadScript") << "Failed to create new Lua state";
		return false;
	}

	lua_atpanic(m_luaState, panicHandler);
	luaL_openlibs(m_luaState);

	if (!bindContextFunctions())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::loadScript") << "Find to bind script context functions";
		disposeScriptState();
		return false;
	}

	// Build a chunk name that is relative to CWD (= the workspace / repo root when
	// launched from VS Code).  The vscode-lrdb extension converts editor paths to
	// paths relative to its "sourceRoot" setting (${workspaceFolder}), so both
	// sides must agree on the same relative form for breakpoint matching to work.
	std::filesystem::path cwd = std::filesystem::current_path();
	std::filesystem::path relPath = scriptPath.lexically_relative(cwd);
	bool isUnderCwd = !relPath.empty() &&
	                  relPath.native().substr(0, 2) != L".." &&
	                  relPath.native().front() != L'/';
	std::string chunkName = "@" + (isUnderCwd ? relPath.generic_string()
	                                           : scriptPath.generic_string());

	// Read the file ourselves so we can supply the custom chunk name to lua_load.
	std::ifstream scriptFile(scriptPath, std::ios::binary);
	if (!scriptFile.is_open())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::loadScript") << "Failed to open lua script " << scriptPath;
		return false;
	}
	std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), {});

	int ret = luaL_loadbuffer(m_luaState, scriptContent.c_str(), scriptContent.size(), chunkName.c_str());
	if (ret == LUA_OK)
		ret = lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
	if (!checkLuaResult(ret, __FILE__, __LINE__))
	{
		MIKAN_LOG_ERROR("CommonScriptContext::loadScript") << "Failed to load lua script " << scriptPath;
		return false;
	}

	m_scriptFilename= scriptPath;

	return true;
}

bool CommonScriptContext::reloadScript()
{
	if (!m_scriptFilename.empty())
	{
		return loadScript(m_scriptFilename);
	}

	return false;
}

// RAII guard: temporarily redirects the Lua debug server to this context
// for the duration of a Lua execution call, then restores the previous context.
// This lets the debugger follow whichever script is actively executing without
// requiring the user to manually attach to each component.
namespace {
struct LuaDebugContextGuard
{
	explicit LuaDebugContextGuard(CommonScriptContext* ctx)
	{
		auto* dbg = LuaDebugServer::getInstance();
		if (dbg->isListening())
		{
			m_server = dbg;
			m_prevContext = dbg->getAttachedContext();
			dbg->attach(ctx);
		}
	}
	~LuaDebugContextGuard()
	{
		if (m_server)
		{
			if (m_prevContext)
				m_server->attach(m_prevContext);
			else
				m_server->detach();
		}
	}
	LuaDebugServer*      m_server      = nullptr;
	CommonScriptContext* m_prevContext  = nullptr;
};
} // namespace

void CommonScriptContext::updateScript(float deltaSeconds)
{
	EASY_FUNCTION();

	if (m_luaState != nullptr)
	{
		LuaDebugContextGuard debugGuard(this);
		lua_getglobal(m_luaState, "update_scheduler");
		int ret= lua_pcall(m_luaState, 0, 0, 0);
		checkLuaResult(ret, __FILE__, __LINE__);
	}
}

bool CommonScriptContext::bindContextFunctions()
{
	if (!addLuaCoroutineScheduler())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::loadScript") << "Failed to add coroutine scheduler to Lua state";
		return false;
	}

	bindCommonScriptFunctions();
	LuaVec3f::bindFunctions(m_luaState);
	LuaQuatf::bindFunctions(m_luaState);

	return true;
}

bool CommonScriptContext::invokeScriptTrigger(const std::string& triggerName)
{
	if (std::find(m_triggers.begin(), m_triggers.end(), triggerName) != m_triggers.end())
	{
		LuaDebugContextGuard debugGuard(this);
		lua_getglobal(m_luaState, triggerName.c_str());
		int ret= lua_pcall(m_luaState, 0, 0, 0);
		return !checkLuaResult(ret, __FILE__,  __LINE__);
	}

	MIKAN_LOG_ERROR("CommonScriptContext::invokeScriptTrigger") << "Failed to find triggerName " << triggerName;
	return false;
}

bool CommonScriptContext::invokeScriptMessageHandler(const std::string& message)
{
	if (m_luaState != nullptr)
	{
		for (const std::string& function_name : m_messageHandlers)
		{
			// Fetch the message handler
			lua_getglobal(m_luaState, function_name.c_str());

			// Push the request onto the stack
			lua_pushstring(m_luaState, message.c_str());

			// Call the message handlers
			int ret = lua_pcall(m_luaState, 1, 1, 0);
			if (checkLuaResult(ret, __FILE__, __LINE__))
			{
				// See if the message was considered handled
				bool bHandeled= lua_toboolean(m_luaState, -1);

				if (bHandeled)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CommonScriptContext::addLuaCoroutineScheduler()
{
	// Adapted from: https://stackoverflow.com/a/24969185
	static const char* x_coroutineScript=
	R""""(
		local function make_coroutine_scheduler()
			local coroutine_container = {}
			return {
				schedule_coroutine = function(frame, coroutine_thread)
					--print("schedule routine ", coroutine_thread, " for frame ", frame)
					if coroutine_container[frame] == nil then
						coroutine_container[frame] = {}
					end
					table.insert(coroutine_container[frame], coroutine_thread)
				end,
				run = function(frame_number, script_control)
					if coroutine_container[frame_number] ~= nil then
						local i = 1
						--recheck length every time, to allow coroutine to resume on the same frame
						local coroutine_threads = coroutine_container[frame_number]
						while i <= #coroutine_threads do
							--print("resume ", coroutine_threads[i], " on frame ", frame_number)
							local success, msg = coroutine.resume(coroutine_threads[i])
							if not success then error(msg) end
							i = i + 1
						end
					end
				end
			}
		end

		wait_frames = function(frame_duration)
			scheduler.schedule_coroutine(
				frame_number+math.floor(frame_duration),
				coroutine.running())
			coroutine.yield()
		end

		wait_next_frame = function()
			return wait_frames(1)
		end

		wait_seconds = function(seconds_duration)
			return wait_frames(math.floor(seconds_duration*fps))
		end

		get_frame_delta_seconds = function()
			return 1.0/fps;
		end

		start_coroutine = function(task)
			local coroutine_thread = coroutine.create(task)
			local success, msg = coroutine.resume(coroutine_thread)
			if not success then error(msg) end
		end

		fps = 60
		frame_number = 1
		scheduler = make_coroutine_scheduler()

		function update_scheduler()
			--print("frame", frame_number)
		    scheduler.run(frame_number)
			frame_number = frame_number+1
		end
	)"""";

	int ret= luaL_dostring(m_luaState, x_coroutineScript);
	return checkLuaResult(ret, __FILE__, __LINE__);
}

template<typename t_enum_class>
static void addEnumToLua(
	luabridge::Namespace& globalNamespace, 
	const std::string& enumName, 
	const std::string* enumStrings)
{
	for (int enumIntValue = 0; enumIntValue < (int)t_enum_class::COUNT; ++enumIntValue)
	{
		const std::string enumString = enumStrings[enumIntValue];

		globalNamespace.addProperty(enumString.c_str(), [enumIntValue]() { return enumIntValue; });
	}
}

void CommonScriptContext::bindCommonScriptFunctions()
{
	auto globalNamespace= luabridge::getGlobalNamespace(m_luaState);
	auto contextNamespace = globalNamespace.beginNamespace("ScriptContext");

	contextNamespace.addFunction("registerTrigger", [this](const char* functionName) {
		m_triggers.push_back(functionName);
	});

	contextNamespace.addFunction("registerMessageHandler", [this](const char* functionName) {
		m_messageHandlers.push_back(functionName);
	});

	contextNamespace.addFunction("broadcastMessage", [this](const char* message) {
		if (OnScriptMessage)
			OnScriptMessage(message);
	});

	// Register enums
	addEnumToLua<eStencilCullMode>(contextNamespace, "CullMode", k_stencilCullModeStrings);

	contextNamespace.endNamespace();

	// Programmatic breakpoint helper: call lrdb_break() anywhere in a script to
	// force a pause on the next line event, without needing gutter breakpoints.
	luabridge::getGlobalNamespace(m_luaState)
		.addFunction("lrdb_break", []() {
			LuaDebugServer::getInstance()->pauseOnNextLine();
		});
}

void CommonScriptContext::disposeScriptState()
{
	m_triggers.clear();

	if (m_luaState != nullptr)
	{
		// Detach the debug server before closing the Lua state so it doesn't
		// call lua_sethook on a freed state during its own teardown.
		auto* debugServer = LuaDebugServer::getInstance();
		if (debugServer->getAttachedContext() == this)
			debugServer->detach();

		lua_close(m_luaState);
		m_luaState= nullptr;
	}
}