#pragma once

#include <filesystem>
#include <string>
#include <vector>

//-- predeclarations -----
struct lua_State;
typedef struct lua_State lua_State;

//-- definitions -----
class CommonScriptContext
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
	void bindGLMFunctions();
	bool addLuaCoroutineScheduler();

	std::filesystem::path m_scriptFilename;
	std::vector<std::string> m_triggers;
	lua_State* m_luaState;
};