#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
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

	CommonScriptContext();
	virtual ~CommonScriptContext();

	// Create the Lua state, bind the context functions, and attach the debugger
	bool createScriptState();
	// Run one script file's chunk into the live state; registrations made while
	// it runs are attributed to scriptId
	bool runScriptFile(const std::filesystem::path& scriptPath, MikanScriptID scriptId);
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

	MulticastDelegate<void(const std::string& message)> OnScriptMessage;

protected:
	static int panicHandler(lua_State* state);
	bool checkLuaResult(int ret, const char* filename, int line);

	virtual bool bindContextFunctions();
	void bindCommonScriptFunctions();
	bool addLuaCoroutineScheduler();

	std::vector<LoadedScript> m_loadedScripts;
	std::vector<TriggerBinding> m_triggers;
	std::vector<MessageHandlerBinding> m_messageHandlers;
	std::vector<HttpTriggerBinding> m_httpTriggerBindings;
	// The script whose chunk is executing, so registrations can be attributed
	MikanScriptID m_loadingScriptId= INVALID_MIKAN_ID;
	lua_State* m_luaState= nullptr;
};
