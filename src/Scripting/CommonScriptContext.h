#pragma once

#include "ScriptingFwd.h"

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
	void updateScript();
	inline bool hasScriptFilename() const { return !m_scriptFilename.empty(); }
	inline bool hasLoadedScript() const { return m_luaState != nullptr; }

	const std::vector<std::string>& getScriptTriggers() const
	{ return m_triggers; }
	bool invokeScriptTrigger(const std::string& triggerName);

protected:
	static int panicHandler(lua_State* state);
	bool checkLuaResult(int ret, const char* filename, int line);

	virtual bool bindContextFunctions();
	void bindCommonScriptFunctions();
	bool addLuaCoroutineScheduler();

	std::filesystem::path m_scriptFilename;
	std::vector<std::string> m_triggers;
	lua_State* m_luaState;
};