#pragma once

#include "ComponentFwd.h"
#include "DMXSequenceComponent.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>
#include <string>

// -- DMXSequenceSystemDefinition -----
class DMXSequenceSystemDefinition
	: public MikanTypedObjectSystemDefinition<DMXSequenceComponent, DMXSequenceDefinition, MikanDMXSequenceID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<DMXSequenceComponent, DMXSequenceDefinition, MikanDMXSequenceID>;

	DMXSequenceSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

// -- DMXSequenceSystem -----
// Ticks every playing sequence once per frame. Registered before
// DMXObjectSystem so a frame's fixture writes flush over DMX the same frame.
class DMXSequenceSystem : public MikanTypedObjectSystem<DMXSequenceComponent, DMXSequenceDefinition, MikanDMXSequenceID,
														DMXSequenceSystem, DMXSequenceSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<DMXSequenceComponent, DMXSequenceDefinition, MikanDMXSequenceID,
										DMXSequenceSystem, DMXSequenceSystemDefinition>;

	DMXSequenceSystem(ProjectManagerPtr ownerProjectManager);

	inline static const std::string k_objectSystemClassName= "DMXSequenceSystem";
	virtual std::string getObjectSystemClassName() const override { return k_objectSystemClassName; }

	virtual void update(float deltaSeconds) override;
	// A default sequence binds to the first group, so it has fixtures to animate
	virtual MikanComponentPtr addNewObjectWithDefaultDefinition(const std::string& primaryComponentClass) override;

	inline DMXSequenceComponentPtr getSequenceById(MikanDMXSequenceID sequenceId) const
	{
		return Super::getTypedComponentById(sequenceId);
	}
	inline DMXSequenceComponentPtr getSequenceByName(const std::string& name) const
	{
		return Super::getTypedComponentByName(name);
	}

	// A stopped sequence for the group, named by the caller or by default
	DMXSequenceComponentPtr createSequence(MikanDMXFixtureGroupID groupId, const std::string& name);
	bool removeSequence(MikanDMXSequenceID sequenceId);
	// Destroy every sequence addressing the group, each as its own transaction.
	// Called before the group's own removal so undo recreates the group first.
	void removeSequencesForGroup(MikanDMXFixtureGroupID groupId);

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);
};
