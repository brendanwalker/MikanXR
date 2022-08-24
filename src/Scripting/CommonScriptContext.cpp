#include "CommonScriptContext.h"
#include "MathGLM.h"
#include "LuaMath.h"
#include "Logger.h"

#include <algorithm>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#endif
#include <assert.h>
#include "luaaa.hpp"
#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include "easy/profiler.h"

using namespace luaaa;

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

bool CommonScriptContext::loadScript(const std::string& scriptPath)
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

	int ret= luaL_dofile(m_luaState, scriptPath.c_str());
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

void CommonScriptContext::updateScript()
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
		MIKAN_LOG_ERROR("CommonScriptContext::loadScript") << "Failed to add coroutine scheduler to Lua state";
		return false;
	}

	bindCommonScriptFunctions();
	LuaVec3f::bindFunctions(m_luaState);

	return true;
}

bool CommonScriptContext::invokeScriptTrigger(const std::string& triggerName)
{
	if (std::find(m_triggers.begin(), m_triggers.end(), triggerName) != m_triggers.end())
	{
		lua_getglobal(m_luaState, triggerName.c_str());
		int ret= lua_pcall(m_luaState, 0, 0, 0);
		return !checkLuaResult(ret, __FILE__,  __LINE__);
	}

	MIKAN_LOG_ERROR("CommonScriptContext::invokeScriptTrigger") << "Failed to find triggerName " << triggerName;
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

void CommonScriptContext::bindCommonScriptFunctions()
{
	LuaModule(m_luaState, "ScriptContext")
	.fun("registerTrigger", [this](const char* functionName) {
		m_triggers.push_back(functionName);
	});
}

void CommonScriptContext::disposeScriptState()
{
	m_triggers.clear();

	if (m_luaState != nullptr)
	{
		lua_close(m_luaState);
		m_luaState= nullptr;
	}
}