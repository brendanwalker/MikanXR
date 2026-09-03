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
CommonScriptContext::CommonScriptContext() {}

CommonScriptContext::~CommonScriptContext() { disposeScriptState(); }

int CommonScriptContext::panicHandler(lua_State* state)
{
	const char* err= lua_tostring(state, 1);
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
			const std::string traceback= lua_tostring(m_luaState, -1);

			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Syntax error during pre-compilation";
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << traceback;
		}
		break;
		case LUA_ERRMEM:
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Memory allocation error";
			break;
		case LUA_ERRRUN:
		{
			const std::string errMsg= lua_tostring(m_luaState, -1);
			luaL_traceback(m_luaState, m_luaState, nullptr, 1);
			const std::string traceback= lua_tostring(m_luaState, -1);

			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << errMsg;
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << traceback;
		}
		break;
		case LUA_ERRERR:
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << "Error while running the error handler function";
			break;
		default:
			const std::string errMsg= lua_tostring(m_luaState, -1);
			MIKAN_LOG_ERROR("CommonScriptContext::checkLuaState") << errMsg;
			break;
		}

		// Terminate the script state
		disposeScriptState();

		return false;
	}

	return true;
}

bool CommonScriptContext::createScriptState()
{
	disposeScriptState();

	m_luaState= luaL_newstate();
	if (m_luaState == nullptr)
	{
		MIKAN_LOG_ERROR("CommonScriptContext::createScriptState") << "Failed to create new Lua state";
		return false;
	}

	lua_atpanic(m_luaState, panicHandler);
	luaL_openlibs(m_luaState);

	if (!bindContextFunctions())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::createScriptState") << "Failed to bind script context functions";
		disposeScriptState();
		return false;
	}

	// One state per project: the debugger attaches here and follows every
	// script that runs in it
	auto* debugServer= LuaDebugServer::getInstance();
	if (debugServer->isListening())
	{
		debugServer->attach(this);
	}

	return true;
}

bool CommonScriptContext::runScriptFile(const std::filesystem::path& scriptPath, MikanScriptID scriptId)
{
	if (m_luaState == nullptr)
	{
		MIKAN_LOG_ERROR("CommonScriptContext::runScriptFile") << "No script state to run " << scriptPath << " in";
		return false;
	}

	// Build a chunk name that is relative to CWD (= the workspace / repo root when
	// launched from VS Code).  The vscode-lrdb extension converts editor paths to
	// paths relative to its "sourceRoot" setting (${workspaceFolder}), so both
	// sides must agree on the same relative form for breakpoint matching to work.
	std::filesystem::path cwd= std::filesystem::current_path();
	std::filesystem::path relPath= scriptPath.lexically_relative(cwd);
	bool isUnderCwd= !relPath.empty() && relPath.native().substr(0, 2) != L".." && relPath.native().front() != L'/';
	std::string chunkName= "@" + (isUnderCwd ? relPath.generic_string() : scriptPath.generic_string());

	// Read the file ourselves so we can supply the custom chunk name to lua_load.
	std::ifstream scriptFile(scriptPath, std::ios::binary);
	if (!scriptFile.is_open())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::runScriptFile") << "Failed to open lua script " << scriptPath;
		return false;
	}
	std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)), {});

	// Registrations made while the chunk runs belong to this script
	m_loadingScriptId= scriptId;
	int ret= luaL_loadbuffer(m_luaState, scriptContent.c_str(), scriptContent.size(), chunkName.c_str());
	if (ret == LUA_OK)
		ret= lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
	m_loadingScriptId= INVALID_MIKAN_ID;

	if (!checkLuaResult(ret, __FILE__, __LINE__))
	{
		MIKAN_LOG_ERROR("CommonScriptContext::runScriptFile") << "Failed to run lua script " << scriptPath;
		return false;
	}

	m_loadedScripts.push_back({scriptId, scriptPath});

	return true;
}

bool CommonScriptContext::isScriptLoaded(MikanScriptID scriptId) const
{
	return std::find_if(m_loadedScripts.begin(), m_loadedScripts.end(),
						[scriptId](const LoadedScript& script) { return script.scriptId == scriptId; })
		   != m_loadedScripts.end();
}

void CommonScriptContext::updateScript(float deltaSeconds)
{
	EASY_FUNCTION();

	if (m_luaState != nullptr)
	{
		lua_getglobal(m_luaState, "update_scheduler");
		int ret= lua_pcall(m_luaState, 0, 0, 0);
		checkLuaResult(ret, __FILE__, __LINE__);
	}
}

bool CommonScriptContext::bindContextFunctions()
{
	if (!addLuaCoroutineScheduler())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::bindContextFunctions")
			<< "Failed to add coroutine scheduler to Lua state";
		return false;
	}

	bindCommonScriptFunctions();
	LuaVec3f::bindFunctions(m_luaState);
	LuaQuatf::bindFunctions(m_luaState);

	return true;
}

void CommonScriptContext::getTriggerNamesForScript(MikanScriptID scriptId, std::vector<std::string>& outNames) const
{
	for (const TriggerBinding& trigger : m_triggers)
	{
		if (trigger.scriptId == scriptId)
			outNames.push_back(trigger.functionName);
	}
}

bool CommonScriptContext::hasTrigger(const std::string& triggerName) const
{
	return std::find_if(m_triggers.begin(), m_triggers.end(),
						[&triggerName](const TriggerBinding& trigger) { return trigger.functionName == triggerName; })
		   != m_triggers.end();
}

bool CommonScriptContext::invokeScriptTrigger(const std::string& triggerName)
{
	if (m_luaState != nullptr && hasTrigger(triggerName))
	{
		lua_getglobal(m_luaState, triggerName.c_str());
		int ret= lua_pcall(m_luaState, 0, 0, 0);
		return checkLuaResult(ret, __FILE__, __LINE__);
	}

	MIKAN_LOG_ERROR("CommonScriptContext::invokeScriptTrigger") << "Failed to find triggerName " << triggerName;
	return false;
}

bool CommonScriptContext::evalString(const std::string& code, std::string& outResult)
{
	outResult.clear();

	if (m_luaState == nullptr)
	{
		outResult= "no script loaded";
		return false;
	}

	const int stackTop= lua_gettop(m_luaState);

	if (luaL_dostring(m_luaState, code.c_str()) != LUA_OK)
	{
		const char* errorMessage= lua_tostring(m_luaState, -1);
		outResult= errorMessage != nullptr ? errorMessage : "unknown lua error";
		lua_settop(m_luaState, stackTop);
		return false;
	}

	// Stringify any values the statement returned
	const int resultCount= lua_gettop(m_luaState) - stackTop;
	for (int i= 0; i < resultCount; ++i)
	{
		const char* valueString= luaL_tolstring(m_luaState, stackTop + 1 + i, nullptr);
		if (!outResult.empty())
			outResult+= " ";
		outResult+= valueString != nullptr ? valueString : "nil";
		lua_pop(m_luaState, 1); // pop luaL_tolstring's string copy
	}

	lua_settop(m_luaState, stackTop);
	return true;
}

bool CommonScriptContext::invokeScriptMessageHandler(const std::string& message)
{
	if (m_luaState != nullptr)
	{
		for (const MessageHandlerBinding& handler : m_messageHandlers)
		{
			// Fetch the message handler
			lua_getglobal(m_luaState, handler.functionName.c_str());

			// Push the request onto the stack
			lua_pushstring(m_luaState, message.c_str());

			// Call the message handler
			int ret= lua_pcall(m_luaState, 1, 1, 0);
			if (!checkLuaResult(ret, __FILE__, __LINE__))
			{
				// The state was disposed on error
				return false;
			}

			// See if the message was considered handled
			const bool bHandled= lua_toboolean(m_luaState, -1);
			lua_pop(m_luaState, 1);

			if (bHandled)
			{
				return true;
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

template <typename t_enum_class>
static void addEnumToLua(luabridge::Namespace& globalNamespace, const std::string& enumName,
						 const std::string* enumStrings)
{
	for (int enumIntValue= 0; enumIntValue < (int)t_enum_class::COUNT; ++enumIntValue)
	{
		const std::string enumString= enumStrings[enumIntValue];

		globalNamespace.addProperty(enumString.c_str(), [enumIntValue]() { return enumIntValue; });
	}
}

void CommonScriptContext::bindCommonScriptFunctions()
{
	auto globalNamespace= luabridge::getGlobalNamespace(m_luaState);
	auto contextNamespace= globalNamespace.beginNamespace("ScriptContext");

	contextNamespace.addFunction("registerTrigger", [this](const char* functionName)
								 { m_triggers.push_back({functionName, m_loadingScriptId}); });

	contextNamespace.addFunction("registerMessageHandler", [this](const char* functionName)
								 { m_messageHandlers.push_back({functionName, m_loadingScriptId}); });

	contextNamespace.addFunction(
		"registerHttpTrigger", [this](const char* routeName, const char* triggerFunctionName)
		{ m_httpTriggerBindings.push_back({routeName, triggerFunctionName, m_loadingScriptId}); });

	contextNamespace.addFunction("broadcastMessage",
								 [this](const char* message)
								 {
									 if (OnScriptMessage)
										 OnScriptMessage(message);
								 });

	// Register enums
	addEnumToLua<eStencilCullMode>(contextNamespace, "CullMode", k_stencilCullModeStrings);

	contextNamespace.endNamespace();

	// Programmatic breakpoint helper: call lrdb_break() anywhere in a script to
	// force a pause on the next line event, without needing gutter breakpoints.
	luabridge::getGlobalNamespace(m_luaState)
		.addFunction("lrdb_break", []() { LuaDebugServer::getInstance()->pauseOnNextLine(); });
}

void CommonScriptContext::disposeScriptState()
{
	m_loadedScripts.clear();
	m_triggers.clear();
	m_messageHandlers.clear();
	m_httpTriggerBindings.clear();

	if (m_luaState != nullptr)
	{
		// Detach the debug server before closing the Lua state so it doesn't
		// call lua_sethook on a freed state during its own teardown.
		auto* debugServer= LuaDebugServer::getInstance();
		if (debugServer->getAttachedContext() == this)
			debugServer->detach();

		lua_close(m_luaState);
		m_luaState= nullptr;
	}
}
