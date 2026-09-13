#include "CommonScriptContext.h"
#include "CompositorConstants.h"
#include "LuaDebugServer.h"
#include "MathGLM.h"
#include "LuaMath.h"
#include "Logger.h"
#include "MikanComponent.h"
#include "PathUtils.h"
#include "ScriptVariableTable.h"

#include <algorithm>
#include <assert.h>
#include <fstream>
#include <filesystem>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "easy/profiler.h"

namespace
{
// Registry slot holding the context that owns a Lua state. Its address is the
// key, so it only has to be unique.
const char k_scriptContextRegistryKey= 0;

// Map a Lua value to the variant type a script variable can hold. Lua keeps
// integer and float subtypes apart, so 30 registers INT and 30.0 FLOAT.
bool luaRefToVariant(const luabridge::LuaRef& ref, MikanVariant& outValue)
{
	if (ref.isBool())
	{
		outValue.setValue(ref.unsafe_cast<bool>());
		return true;
	}
	else if (ref.isNumber())
	{
		lua_State* L= ref.state();
		ref.push(L);
		const bool bIsInteger= lua_isinteger(L, -1) != 0;
		lua_pop(L, 1);

		if (bIsInteger)
			outValue.setValue(ref.unsafe_cast<int>());
		else
			outValue.setValue(ref.unsafe_cast<float>());
		return true;
	}
	else if (ref.isString())
	{
		outValue.setValue(ref.unsafe_cast<std::string>());
		return true;
	}
	else if (ref.isUserdata() && ref.isInstance<LuaVec3f>())
	{
		outValue.setValue(ref.unsafe_cast<LuaVec3f>().toMikanVector3f());
		return true;
	}

	return false;
}

// lua_pcall message handler: append the traceback while the erroring frames
// are still on the stack
int tracebackMessageHandler(lua_State* L)
{
	const char* message= lua_tostring(L, 1);
	luaL_traceback(L, L, message, 1);
	return 1;
}

// The chunk name a script file loads under: the path relative to the project
// folder. The vscode-lrdb extension sends breakpoint paths relative to its
// "sourceRoot", which the project's generated launch config sets to the
// workspace (the project folder), so both sides agree on the same relative
// form. A script outside the project keeps its absolute path.
std::string makeChunkName(const std::filesystem::path& scriptPath)
{
	const std::filesystem::path projectDir= PathUtils::getProjectDirectory();
	std::filesystem::path relPath=
		projectDir.empty() ? std::filesystem::path() : scriptPath.lexically_relative(projectDir);
	const bool isUnderProject=
		!relPath.empty() && relPath.native().substr(0, 2) != L".." && relPath.native().front() != L'/';

	return "@" + (isUnderProject ? relPath.generic_string() : scriptPath.generic_string());
}

// The package.searchers entry that replaces Lua's stock file loader. It finds
// the module through package.path exactly as the stock loader does, but loads
// the chunk under makeChunkName, so a breakpoint set in a required module lands
// the same way it does in a script the editor ran itself.
int projectModuleSearcher(lua_State* L)
{
	const char* moduleName= luaL_checkstring(L, 1);

	// package.searchpath does the module name to file path substitution and,
	// on failure, reports every path it tried
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "searchpath");
	lua_pushvalue(L, 1);
	lua_getfield(L, -3, "path");
	lua_call(L, 2, 2);

	if (lua_isnil(L, -2))
	{
		// Hand back the list of paths tried; Lua folds it into the error
		return 1;
	}

	const std::string filePath= lua_tostring(L, -2);
	lua_pop(L, 2);

	std::ifstream moduleFile(filePath, std::ios::binary);
	if (!moduleFile.is_open())
	{
		lua_pushfstring(L, "\n\tno file '%s'", filePath.c_str());
		return 1;
	}
	std::string moduleContent((std::istreambuf_iterator<char>(moduleFile)), {});

	const std::string chunkName= makeChunkName(filePath);
	if (luaL_loadbuffer(L, moduleContent.c_str(), moduleContent.size(), chunkName.c_str()) != LUA_OK)
	{
		return luaL_error(L, "error loading module '%s' from file '%s':\n\t%s", moduleName, filePath.c_str(),
						  lua_tostring(L, -1));
	}

	// The chunk, then the file path Lua passes it as its second argument
	lua_pushstring(L, filePath.c_str());
	return 2;
}
} // namespace

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

	// Bindings reach the context back through the state they are called with
	lua_pushlightuserdata(m_luaState, this);
	lua_rawsetp(m_luaState, LUA_REGISTRYINDEX, &k_scriptContextRegistryKey);

	setupModuleSearchPath();

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

void CommonScriptContext::setupModuleSearchPath()
{
	lua_State* L= m_luaState;

	// require() searches the project's own scripts folder first, then the bundled
	// scripts behind it, so a project module shadows a bundled one of the same
	// name. Lua's stock entries stay behind ours rather than being replaced, so
	// nothing that resolved before stops resolving.
	std::string searchPath;
	const std::filesystem::path projectDir= PathUtils::getProjectDirectory();
	if (!projectDir.empty())
	{
		// Forward slashes: Windows accepts them, and package.path treats the
		// string literally
		const std::string scriptsDir= (projectDir / "scripts").generic_string();

		searchPath+= scriptsDir + "/?.lua;";
		searchPath+= scriptsDir + "/?/init.lua;";
	}

	const std::string bundledScriptsDir= (PathUtils::getResourceDirectory() / "scripts").generic_string();
	searchPath+= bundledScriptsDir + "/?.lua;";
	searchPath+= bundledScriptsDir + "/?/init.lua;";

	lua_getglobal(L, "package");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		MIKAN_LOG_ERROR("CommonScriptContext::setupModuleSearchPath") << "No package table in the Lua state";
		return;
	}

	lua_getfield(L, -1, "path");
	const char* defaultPath= lua_tostring(L, -1);
	if (defaultPath != nullptr)
		searchPath+= defaultPath;
	lua_pop(L, 1);

	lua_pushstring(L, searchPath.c_str());
	lua_setfield(L, -2, "path");

	// Take over the file searcher, leaving package.preload ahead of it and the
	// C loaders behind it
	lua_getfield(L, -1, "searchers");
	if (lua_istable(L, -1))
	{
		lua_pushcfunction(L, projectModuleSearcher);
		lua_seti(L, -2, 2);
	}
	lua_pop(L, 2);
}

bool CommonScriptContext::runScriptFile(const std::filesystem::path& scriptPath, MikanScriptID scriptId,
										IScriptVariableStore* variableStore)
{
	if (m_luaState == nullptr)
	{
		MIKAN_LOG_ERROR("CommonScriptContext::runScriptFile") << "No script state to run " << scriptPath << " in";
		return false;
	}

	const std::string chunkName= makeChunkName(scriptPath);

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
	m_loadingVariableStore= variableStore;
	int ret= luaL_loadbuffer(m_luaState, scriptContent.c_str(), scriptContent.size(), chunkName.c_str());
	if (ret == LUA_OK)
		ret= lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
	m_loadingScriptId= INVALID_MIKAN_ID;
	m_loadingVariableStore= nullptr;

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

void CommonScriptContext::getVariableNamesForScript(MikanScriptID scriptId, std::vector<std::string>& outNames) const
{
	for (const VariableBinding& variable : m_variables)
	{
		if (variable.scriptId == scriptId)
			outNames.push_back(variable.name);
	}
}

void CommonScriptContext::getSequenceNames(std::vector<std::string>& outNames) const
{
	for (const SequenceBinding& sequence : m_sequences)
	{
		outNames.push_back(sequence.name);
	}
}

bool CommonScriptContext::hasSequence(const std::string& name) const
{
	return std::find_if(m_sequences.begin(), m_sequences.end(),
						[&name](const SequenceBinding& sequence) { return sequence.name == name; })
		   != m_sequences.end();
}

MikanScriptID CommonScriptContext::getSequenceScriptId(const std::string& name) const
{
	auto it= std::find_if(m_sequences.begin(), m_sequences.end(),
						  [&name](const SequenceBinding& sequence) { return sequence.name == name; });
	return it != m_sequences.end() ? it->scriptId : INVALID_MIKAN_ID;
}

bool CommonScriptContext::callSequenceHandler(const std::string& name, const char* field, const LuaArgPusher& pushArgs,
											  std::string& outError)
{
	outError.clear();
	if (m_luaState == nullptr)
		return false;

	auto it= std::find_if(m_sequences.begin(), m_sequences.end(),
						  [&name](const SequenceBinding& sequence) { return sequence.name == name; });
	if (it == m_sequences.end())
	{
		outError= "sequence " + name + " is not registered";
		return false;
	}

	lua_State* L= m_luaState;
	const int baseTop= lua_gettop(L);

	// Stack: traceback handler, handler table, field
	lua_pushcfunction(L, tracebackMessageHandler);
	lua_rawgeti(L, LUA_REGISTRYINDEX, it->handlerRef);
	lua_getfield(L, -1, field);
	if (!lua_isfunction(L, -1))
	{
		lua_settop(L, baseTop);
		return true;
	}

	// Drop the table from under the function, then push the arguments
	lua_remove(L, -2);
	const int argCount= pushArgs(L);
	const int ret= lua_pcall(L, argCount, 0, baseTop + 1);
	if (ret != LUA_OK)
	{
		const char* message= lua_tostring(L, -1);
		outError= message != nullptr ? message : "unknown Lua error";
	}

	lua_settop(L, baseTop);
	return ret == LUA_OK;
}

bool CommonScriptContext::hasVariable(const std::string& name) const
{
	return std::find_if(m_variables.begin(), m_variables.end(),
						[&name](const VariableBinding& variable) { return variable.name == name; })
		   != m_variables.end();
}

bool CommonScriptContext::setVariableValue(const std::string& name, const MikanVariant& value)
{
	if (m_luaState == nullptr)
		return false;

	auto it= std::find_if(m_variables.begin(), m_variables.end(),
						  [&name](const VariableBinding& variable) { return variable.name == name; });
	if (it == m_variables.end() || it->type != value.value_type)
		return false;

	if (it->isComponentReference())
	{
		it->componentId= value.getIntValue();
		return pushComponentGlobal(*it);
	}

	return pushVariantAsGlobal(name, value);
}

void CommonScriptContext::refreshComponentVariables(MikanComponentID excludedId)
{
	if (m_luaState == nullptr)
		return;

	for (const VariableBinding& binding : m_variables)
	{
		if (binding.isComponentReference())
		{
			pushComponentGlobal(binding, excludedId);
		}
	}
}

void CommonScriptContext::registerComponentClass(const std::string& className, ComponentPushFunction pushFunction)
{
	m_componentPushFunctions[className]= pushFunction;
}

bool CommonScriptContext::pushComponent(lua_State* L, MikanComponentPtr component) const
{
	if (!component)
	{
		lua_pushnil(L);
		return true;
	}

	auto pushIt= m_componentPushFunctions.find(component->getComponentClassName());
	if (pushIt == m_componentPushFunctions.end())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::pushComponent")
			<< "Component class " << component->getComponentClassName() << " has no Lua push thunk";
		lua_pushnil(L);
		return false;
	}

	// Exactly one value lands on the stack either way, so callers can pop or
	// assign it without checking. A failed push leaves nothing behind.
	const int baseTop= lua_gettop(L);
	const bool bPushed= pushIt->second(L, component);
	if (lua_gettop(L) != baseTop + 1)
	{
		lua_settop(L, baseTop);
		lua_pushnil(L);
		return false;
	}

	return bPushed;
}

CommonScriptContext* CommonScriptContext::getFromLuaState(lua_State* L)
{
	lua_rawgetp(L, LUA_REGISTRYINDEX, &k_scriptContextRegistryKey);
	auto* context= static_cast<CommonScriptContext*>(lua_touserdata(L, -1));
	lua_pop(L, 1);

	return context;
}

MikanComponentPtr CommonScriptContext::resolveComponent(const std::string& componentClass,
														MikanComponentID componentId) const
{
	return nullptr;
}

bool CommonScriptContext::registerVariable(const std::string& name, const MikanVariant& defaultValue)
{
	if (m_loadingScriptId == INVALID_MIKAN_ID)
	{
		MIKAN_LOG_ERROR("CommonScriptContext::registerVariable")
			<< "Variable " << name << " registered outside a script chunk";
		return false;
	}

	// Every script shares one global table, so a second registration would
	// silently alias the first script's value
	auto existing= std::find_if(m_variables.begin(), m_variables.end(),
								[&name](const VariableBinding& variable) { return variable.name == name; });
	if (existing != m_variables.end())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::registerVariable")
			<< "Variable " << name << " already registered by script " << existing->scriptId << ", ignored by script "
			<< m_loadingScriptId;
		return false;
	}

	// A stored value wins over the script's default; a default with no stored
	// value is adopted into the store
	MikanVariant effectiveValue= defaultValue;
	if (m_loadingVariableStore != nullptr)
	{
		MikanVariant storedValue;
		if (m_loadingVariableStore->getScriptVariableOfType(name, defaultValue.value_type, storedValue))
		{
			effectiveValue= storedValue;
		}
		else
		{
			m_loadingVariableStore->setScriptVariable(name, defaultValue);
		}
	}

	if (!pushVariantAsGlobal(name, effectiveValue))
		return false;

	m_variables.push_back({name, m_loadingScriptId, defaultValue.value_type, "", INVALID_MIKAN_ID});
	return true;
}

bool CommonScriptContext::registerComponentVariable(const std::string& name, const std::string& componentClass)
{
	if (m_loadingScriptId == INVALID_MIKAN_ID)
	{
		MIKAN_LOG_ERROR("CommonScriptContext::registerComponentVariable")
			<< "Component variable " << name << " registered outside a script chunk";
		return false;
	}

	if (m_componentPushFunctions.find(componentClass) == m_componentPushFunctions.end())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::registerComponentVariable")
			<< "Component variable " << name << " names unknown class " << componentClass;
		return false;
	}

	auto existing= std::find_if(m_variables.begin(), m_variables.end(),
								[&name](const VariableBinding& variable) { return variable.name == name; });
	if (existing != m_variables.end())
	{
		MIKAN_LOG_ERROR("CommonScriptContext::registerComponentVariable")
			<< "Variable " << name << " already registered by script " << existing->scriptId << ", ignored by script "
			<< m_loadingScriptId;
		return false;
	}

	// A stored reference of the same class wins; otherwise none is adopted
	MikanComponentID componentId= INVALID_MIKAN_ID;
	if (m_loadingVariableStore != nullptr)
	{
		MikanComponentID storedId= INVALID_MIKAN_ID;
		if (m_loadingVariableStore->getScriptComponentVariable(name, componentClass, storedId))
		{
			componentId= storedId;
		}
		else
		{
			m_loadingVariableStore->setScriptComponentVariable(name, componentClass, componentId);
		}
	}

	VariableBinding binding{name, m_loadingScriptId, MikanVariantType::INT, componentClass, componentId};
	if (!pushComponentGlobal(binding))
		return false;

	m_variables.push_back(binding);
	return true;
}

bool CommonScriptContext::pushComponentGlobal(const VariableBinding& binding, MikanComponentID excludedId)
{
	if (m_componentPushFunctions.find(binding.componentClass) == m_componentPushFunctions.end())
		return false;

	MikanComponentPtr component;
	if (binding.componentId != INVALID_MIKAN_ID && binding.componentId != excludedId)
	{
		component= resolveComponent(binding.componentClass, binding.componentId);
	}

	// A null component pushes nil
	const bool bPushed= pushComponent(m_luaState, component);
	lua_setglobal(m_luaState, binding.name.c_str());

	return bPushed;
}

bool CommonScriptContext::pushVariantAsGlobal(const std::string& name, const MikanVariant& value)
{
	switch (value.value_type)
	{
	case MikanVariantType::BOOL:
		return luabridge::setGlobal(m_luaState, value.getBoolValue(), name.c_str());
	case MikanVariantType::INT:
		return luabridge::setGlobal(m_luaState, value.getIntValue(), name.c_str());
	case MikanVariantType::FLOAT:
		return luabridge::setGlobal(m_luaState, value.getFloatValue(), name.c_str());
	case MikanVariantType::STRING:
		return luabridge::setGlobal(m_luaState, std::string(value.getUtf8Value()), name.c_str());
	case MikanVariantType::VECTOR3F:
		return luabridge::setGlobal(m_luaState, LuaVec3f(value.getVector3fValue()), name.c_str());
	default:
		MIKAN_LOG_ERROR("CommonScriptContext::pushVariantAsGlobal")
			<< "Variable " << name << " has unsupported type " << mikanVariantTypeToString(value.value_type);
		return false;
	}
}

bool CommonScriptContext::invokeScriptTrigger(const std::string& triggerName,
											  const std::map<std::string, std::string>& args)
{
	if (m_luaState != nullptr && hasTrigger(triggerName))
	{
		lua_getglobal(m_luaState, triggerName.c_str());

		lua_createtable(m_luaState, 0, static_cast<int>(args.size()));
		for (const auto& [key, value] : args)
		{
			lua_pushlstring(m_luaState, value.c_str(), value.size());
			lua_setfield(m_luaState, -2, key.c_str());
		}

		int ret= lua_pcall(m_luaState, 1, 0, 0);
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

	contextNamespace.addFunction("registerVariable",
								 [this](const char* name, luabridge::LuaRef defaultValue) -> bool
								 {
									 MikanVariant value;
									 if (!luaRefToVariant(defaultValue, value))
									 {
										 MIKAN_LOG_ERROR("CommonScriptContext::registerVariable")
											 << "Variable " << name << " has unsupported default type "
											 << lua_typename(defaultValue.state(), defaultValue.type());
										 return false;
									 }

									 return registerVariable(name, value);
								 });

	contextNamespace.addFunction("registerComponent", [this](const char* name, const char* componentClass) -> bool
								 { return registerComponentVariable(name, componentClass); });

	contextNamespace.addFunction("registerSequence",
								 [this](const char* name, luabridge::LuaRef handler) -> bool
								 {
									 if (m_loadingScriptId == INVALID_MIKAN_ID)
									 {
										 MIKAN_LOG_ERROR("CommonScriptContext::registerSequence")
											 << "Sequence " << name << " registered outside a script chunk";
										 return false;
									 }

									 if (!handler.isTable() || !handler["update"].isFunction())
									 {
										 MIKAN_LOG_ERROR("CommonScriptContext::registerSequence")
											 << "Sequence " << name << " needs a handler table with an update function";
										 return false;
									 }

									 if (hasSequence(name))
									 {
										 MIKAN_LOG_ERROR("CommonScriptContext::registerSequence")
											 << "Sequence " << name << " already registered by script "
											 << getSequenceScriptId(name) << ", ignored by script "
											 << m_loadingScriptId;
										 return false;
									 }

									 // Held as a raw registry reference: the binding vector is
									 // released before the state closes
									 handler.push(m_luaState);
									 const int handlerRef= luaL_ref(m_luaState, LUA_REGISTRYINDEX);
									 m_sequences.push_back({name, m_loadingScriptId, handlerRef});
									 return true;
								 });

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
	m_variables.clear();
	m_componentPushFunctions.clear();

	// Handler tables are registry references, released while the state is alive
	if (m_luaState != nullptr)
	{
		for (const SequenceBinding& sequence : m_sequences)
		{
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, sequence.handlerRef);
		}
	}
	m_sequences.clear();

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
