#pragma once

#include "ScriptingFwd.h"
#include "MulticastDelegate.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

//-- definitions -----
class CommonScriptContext : public std::enable_shared_from_this<CommonScriptContext>
{
public:
	CommonScriptContext();
	virtual ~CommonScriptContext();

	bool loadScript(const std::filesystem::path& scriptPath);
	bool reloadScript();
	void disposeScriptState();
	void updateScript(float deltaSeconds);
	inline bool hasScriptFilename() const { return !m_scriptFilename.empty(); }
	inline bool hasLoadedScript() const { return m_luaState != nullptr; }
	inline lua_State* getLuaState() const { return m_luaState; }

	const std::vector<std::string>& getScriptTriggers() const { return m_triggers; }
	bool invokeScriptTrigger(const std::string& triggerName);

	const std::vector<std::string>& getScriptMessageHandler() const { return m_messageHandlers; }
	bool invokeScriptMessageHandler(const std::string& message);

	// Route name -> Lua trigger function name, declared via ScriptContext.registerHttpTrigger(...).
	// Resolution to an actual HTTP route (and the owning system/component) happens externally,
	// in ScriptRequestHandler, when this context is bound/unbound.
	struct HttpTriggerBinding
	{
		std::string routeName;
		std::string triggerName;
	};
	const std::vector<HttpTriggerBinding>& getHttpTriggerBindings() const { return m_httpTriggerBindings; }

	MulticastDelegate<void(const std::string& message)> OnScriptMessage;

protected:
	static int panicHandler(lua_State* state);
	bool checkLuaResult(int ret, const char* filename, int line);

	virtual bool bindContextFunctions();
	void bindCommonScriptFunctions();
	bool addLuaCoroutineScheduler();

	std::filesystem::path m_scriptFilename;
	std::vector<std::string> m_triggers;
	std::vector<std::string> m_messageHandlers;
	std::vector<HttpTriggerBinding> m_httpTriggerBindings;
	lua_State* m_luaState;
};