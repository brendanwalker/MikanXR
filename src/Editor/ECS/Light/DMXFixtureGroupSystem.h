#pragma once

#include "ComponentFwd.h"
#include "DMXFixtureGroupComponent.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>
#include <string>

// -- DMXFixtureGroupSystemDefinition -----
class DMXFixtureGroupSystemDefinition
	: public MikanTypedObjectSystemDefinition<DMXFixtureGroupComponent, DMXFixtureGroupDefinition,
											  MikanDMXFixtureGroupID>
{
public:
	using Super=
		MikanTypedObjectSystemDefinition<DMXFixtureGroupComponent, DMXFixtureGroupDefinition, MikanDMXFixtureGroupID>;

	DMXFixtureGroupSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

// -- DMXFixtureGroupSystem -----
// Owns the fixture groups and keeps their membership free of dangling ids by
// listening to the fixture systems' destroy events
class DMXFixtureGroupSystem
	: public MikanTypedObjectSystem<DMXFixtureGroupComponent, DMXFixtureGroupDefinition, MikanDMXFixtureGroupID,
									DMXFixtureGroupSystem, DMXFixtureGroupSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<DMXFixtureGroupComponent, DMXFixtureGroupDefinition, MikanDMXFixtureGroupID,
										DMXFixtureGroupSystem, DMXFixtureGroupSystemDefinition>;

	DMXFixtureGroupSystem(ProjectManagerPtr ownerProjectManager);

	inline static const std::string k_objectSystemClassName= "DMXFixtureGroupSystem";
	virtual std::string getObjectSystemClassName() const override { return k_objectSystemClassName; }

	virtual void postInit() override;
	virtual void dispose() override;
	// A default group lands on the first stage, so it shows under a stage in
	// the outliner however it was created
	virtual MikanComponentPtr addNewObjectWithDefaultDefinition(const std::string& primaryComponentClass) override;

	inline DMXFixtureGroupComponentPtr getGroupById(MikanDMXFixtureGroupID groupId) const
	{
		return Super::getTypedComponentById(groupId);
	}
	inline DMXFixtureGroupComponentPtr getGroupByName(const std::string& name) const
	{
		return Super::getTypedComponentByName(name);
	}

	// An empty group on the stage, named by the caller or by default
	DMXFixtureGroupComponentPtr createGroup(MikanStageID stageId, const std::string& name);
	bool removeGroup(MikanDMXFixtureGroupID groupId);

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

private:
	void bindFixtureLifecycleEvents();
	void unbindFixtureLifecycleEvents();
	// Fires during the fixture's teardown, after the transaction recorder has
	// opened the destroy composite, so the membership change folds into that
	// composite and undoes with it
	void onFixtureComponentDisposed(MikanObjectSystemPtr objectSystem, MikanComponentConstPtr component);

	bool m_bLifecycleEventsBound= false;
};
