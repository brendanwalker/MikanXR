#pragma once

#include "ComponentFwd.h"
#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "MikanVariantTypes.h"
#include "MulticastDelegate.h"
#include "ScriptingFwd.h"

#include <filesystem>
#include <functional>
#include <map>
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

	// A Lua global declared via ScriptContext.registerVariable(name, default) or
	// ScriptContext.registerComponent(name, className). The global's value is
	// owned by the registering script's definition. A component reference has
	// a class, an INT type, and the id the global currently resolves.
	struct VariableBinding
	{
		std::string name;
		MikanScriptID scriptId;
		MikanVariantType type;
		std::string componentClass;
		MikanComponentID componentId= INVALID_MIKAN_ID;

		inline bool isComponentReference() const { return !componentClass.empty(); }
	};

	// Pushes a component onto the stack as its concrete class (nil for null)
	using ComponentPushFunction= std::function<bool(lua_State*, MikanComponentPtr)>;

	// The value of LUA_NOREF, so the header needs no Lua include
	static constexpr int k_invalidLuaRef= -2;

	// A handler table declared via ScriptContext.registerSequence(name, table),
	// held as a registry reference that is released before the state closes
	struct SequenceBinding
	{
		std::string name;
		MikanScriptID scriptId;
		int handlerRef= k_invalidLuaRef;
	};

	// Pushes a handler call's arguments and returns how many it pushed
	using LuaArgPusher= std::function<int(lua_State*)>;

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
	// A component reference takes the INT component id and pushes the
	// resolved component.
	bool setVariableValue(const std::string& name, const MikanVariant& value);
	// Re-resolve and re-push every component reference global. An id equal to
	// excludedId pushes nil, for an object that is about to be destroyed.
	void refreshComponentVariables(MikanComponentID excludedId= INVALID_MIKAN_ID);
	// Classes registerComponent accepts, keyed by k_componentClassName
	void registerComponentClass(const std::string& className, ComponentPushFunction pushFunction);
	// Push a component onto the stack as its concrete class, which is what a
	// binding returning a base pointer needs: LuaBridge pushes by static type,
	// so a subclass returned as its base loses the subclass's own bindings. A
	// null component, or one of a class this context does not bind, pushes nil.
	bool pushComponent(lua_State* L, MikanComponentPtr component) const;
	// The context owning a Lua state, for bindings that reach back into it.
	// Null for a state this class did not create.
	static CommonScriptContext* getFromLuaState(lua_State* L);

	const std::vector<SequenceBinding>& getScriptSequences() const { return m_sequences; }
	void getSequenceNames(std::vector<std::string>& outNames) const;
	bool hasSequence(const std::string& name) const;
	MikanScriptID getSequenceScriptId(const std::string& name) const;
	// Call one field of a registered handler table. A missing or non-function
	// field is not an error. A Lua error fills outError (message and traceback)
	// and returns false without disposing the state, so a broken handler stops
	// only its own sequence.
	bool callSequenceHandler(const std::string& name, const char* field, const LuaArgPusher& pushArgs,
							 std::string& outError);

	MulticastDelegate<void(const std::string& message)> OnScriptMessage;

protected:
	static int panicHandler(lua_State* state);
	bool checkLuaResult(int ret, const char* filename, int line);

	virtual bool bindContextFunctions();
	void bindCommonScriptFunctions();
	bool addLuaCoroutineScheduler();
	// Point require() at the project's scripts folder and take over the file
	// searcher so required modules load under the same chunk names
	void setupModuleSearchPath();

	// Resolve the effective value against the loading script's store, write it
	// to the Lua global, and record the binding
	bool registerVariable(const std::string& name, const MikanVariant& defaultValue);
	bool registerComponentVariable(const std::string& name, const std::string& componentClass);
	bool pushVariantAsGlobal(const std::string& name, const MikanVariant& value);
	// Resolve the binding's id and write the component (or nil) to its global
	bool pushComponentGlobal(const VariableBinding& binding, MikanComponentID excludedId= INVALID_MIKAN_ID);
	// The component a reference global should hold: subclasses with scene
	// access resolve the id and check the class, the base resolves nothing
	virtual MikanComponentPtr resolveComponent(const std::string& componentClass, MikanComponentID componentId) const;

	std::vector<LoadedScript> m_loadedScripts;
	std::vector<TriggerBinding> m_triggers;
	std::vector<MessageHandlerBinding> m_messageHandlers;
	std::vector<HttpTriggerBinding> m_httpTriggerBindings;
	std::vector<VariableBinding> m_variables;
	std::vector<SequenceBinding> m_sequences;
	std::map<std::string, ComponentPushFunction> m_componentPushFunctions;
	// The script whose chunk is executing, so registrations can be attributed
	MikanScriptID m_loadingScriptId= INVALID_MIKAN_ID;
	IScriptVariableStore* m_loadingVariableStore= nullptr;
	lua_State* m_luaState= nullptr;
};
