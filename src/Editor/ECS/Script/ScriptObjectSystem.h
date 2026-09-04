#pragma once

#include "MikanTypedObjectSystem.h"
#include "ScriptComponent.h"
#include "ScriptingFwd.h"

class ScriptObjectSystemDefinition
	: public MikanTypedObjectSystemDefinition<ScriptComponent, ScriptDefinition, MikanScriptID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<ScriptComponent, ScriptDefinition, MikanScriptID>;

	ScriptObjectSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

// Owns the project's one Lua state. Every script in the pool runs into it in
// pool order, and any change to the pool or to a script's path rebuilds it on
// the next update, never inside the change that requested it.
class ScriptObjectSystem : public MikanTypedObjectSystem<ScriptComponent, ScriptDefinition, MikanScriptID,
														 ScriptObjectSystem, ScriptObjectSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<ScriptComponent, ScriptDefinition, MikanScriptID, ScriptObjectSystem,
										ScriptObjectSystemDefinition>;

	ScriptObjectSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName= "ScriptObjectSystem";
	virtual std::string getObjectSystemClassName() const override { return k_objectSystemClassName; }

	virtual void postInit() override;
	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;

	inline ProjectScriptContextPtr getScriptContext() const { return m_scriptContext; }
	inline void requestReload() { m_bReloadPending= true; }
	// Tear down the state and run every script with a path again, in pool order
	void reloadAllScripts();

	// Create an empty timestamped .lua under the project's scripts folder and
	// register it
	ScriptComponentPtr addNewScript();

private:
	void disposeScriptContext();

	ProjectScriptContextPtr m_scriptContext;
	bool m_bReloadPending= false;
};
