#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "MikanVariantTypes.h"
#include "MulticastDelegate.h"
#include "ScriptingFwd.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

//-- definitions -----
// One Lua state hosting any number of script files. Files run into the state
// in the order given, share globals, and every trigger, message handler, and
// HTTP route is attributed to the script whose chunk registered it.
class CommonScriptContext : public std::enable_shared_from_this<CommonScriptContext>
{
public:
	struct LoadedScript
	{
		MikanScriptID scriptId;
		std::filesystem::path path;
	};

	struct TriggerBinding
	{
		std::string functionName;
		MikanScriptID scriptId;
	};

	struct MessageHandlerBinding
	{
		std::string functionName;
		MikanScriptID scriptId;
	};

	// Route name -> Lua trigger function name, declared via ScriptContext.registerHttpTrigger(...).
	// Resolution to an actual HTTP route happens externally, in ScriptRequestHandler,
	// when this context is bound/unbound.
	struct HttpTriggerBinding
	{
		std::string routeName;
		std::string triggerName;
		MikanScriptID scriptId;
	};

	// A Lua global declared via ScriptContext.registerVariable(name, default).
	// The global's value is owned by the registering script's definition.
	struct VariableBinding
	{
		std::string name;
		MikanScriptID scriptId;
		MikanVariantType type;
	};

	CommonScriptContext();
	virtual ~CommonScriptContext();

	// Create the Lua state, bind the context functions, and attach the debugger
	bool createScriptState();
	// Run one script file's chunk into the live state; registrations made while
	// it runs are attributed to scriptId, and variable registrations resolve
	// their values against variableStore
	bool runScriptFile(const std::filesystem::path& scriptPath, MikanScriptID scriptId,
					   IScriptVariableStore* variableStore= nullptr);
	void disposeScriptState();
	void updateScript(float deltaSeconds);
	inline bool hasLoadedScript() const { return m_luaState != nullptr; }
	inline lua_State* getLuaState() const { return m_luaState; }

	const std::vector<LoadedScript>& getLoadedScripts() const { return m_loadedScripts; }
	bool isScriptLoaded(MikanScriptID scriptId) const;

	const std::vector<TriggerBinding>& getScriptTriggers() const { return m_triggers; }
	void getTriggerNamesForScript(MikanScriptID scriptId, std::vector<std::string>& outNames) const;
	bool hasTrigger(const std::string& triggerName) const;
	bool invokeScriptTrigger(const std::string& triggerName);

	/// Run a Lua statement in this context's state and stringify what it
	/// returns (or the error message on failure). Used by the automation
	/// server's script eval command.
	bool evalString(const std::string& code, std::string& outResult);

	const std::vector<MessageHandlerBinding>& getScriptMessageHandlers() const { return m_messageHandlers; }
	bool invokeScriptMessageHandler(const std::string& message);

	const std::vector<HttpTriggerBinding>& getHttpTriggerBindings() const { return m_httpTriggerBindings; }

	const std::vector<VariableBinding>& getScriptVariables() const { return m_variables; }
	void getVariableNamesForScript(MikanScriptID scriptId, std::vector<std::string>& outNames) const;
	bool hasVariable(const std::string& name) const;
	// Write a registered variable's Lua global. False with no state, an
	// unregistered name, or a value of another type than the registration.
	bool setVariableValue(const std::string& name, const MikanVariant& value);

	MulticastDelegate<void(const std::string& message)> OnScriptMessage;

protected:
	static int panicHandler(lua_State* state);
	bool checkLuaResult(int ret, const char* filename, int line);

	virtual bool bindContextFunctions();
	void bindCommonScriptFunctions();
	bool addLuaCoroutineScheduler();

	// Resolve the effective value against the loading script's store, write it
	// to the Lua global, and record the binding
	bool registerVariable(const std::string& name, const MikanVariant& defaultValue);
	bool pushVariantAsGlobal(const std::string& name, const MikanVariant& value);

	std::vector<LoadedScript> m_loadedScripts;
	std::vector<TriggerBinding> m_triggers;
	std::vector<MessageHandlerBinding> m_messageHandlers;
	std::vector<HttpTriggerBinding> m_httpTriggerBindings;
	std::vector<VariableBinding> m_variables;
	// The script whose chunk is executing, so registrations can be attributed
	MikanScriptID m_loadingScriptId= INVALID_MIKAN_ID;
	IScriptVariableStore* m_loadingVariableStore= nullptr;
	lua_State* m_luaState= nullptr;
};
