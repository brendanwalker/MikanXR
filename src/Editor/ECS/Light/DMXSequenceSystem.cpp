#include "DMXSequenceSystem.h"
#include "DMXFixtureGroupSystem.h"
#include "MikanObject.h"
#include "ProjectManager.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- DMXSequenceSystemDefinition -----
DMXSequenceSystemDefinition::DMXSequenceSystemDefinition(const std::string& configName,
														 IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- DMXSequenceSystem -----
DMXSequenceSystem::DMXSequenceSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

void DMXSequenceSystem::update(float deltaSeconds)
{
	Super::update(deltaSeconds);

	visitComponents([deltaSeconds](DMXSequenceComponentPtr sequence) { sequence->tick(deltaSeconds); });
}

MikanComponentPtr DMXSequenceSystem::addNewObjectWithDefaultDefinition(const std::string& primaryComponentClass)
{
	if (primaryComponentClass != DMXSequenceComponent::k_componentClassName)
		return MikanComponentPtr();

	MikanDMXFixtureGroupID groupId= INVALID_MIKAN_ID;
	if (DMXFixtureGroupSystemPtr groupSystem= getOwnerProjectManager()->getSystemOfType<DMXFixtureGroupSystem>())
	{
		groupId= groupSystem->getFirstComponentId();
	}

	return createSequence(groupId, "");
}

DMXSequenceComponentPtr DMXSequenceSystem::createSequence(MikanDMXFixtureGroupID groupId, const std::string& name)
{
	return addNewObjectByTypedDefinition(
		[groupId, name](DMXSequenceDefinitionPtr definition)
		{
			definition->setGroupId(groupId);
			if (!name.empty())
				definition->setComponentName(name);
			return true;
		});
}

bool DMXSequenceSystem::removeSequence(MikanDMXSequenceID sequenceId)
{
	return removeObjectByPrimaryComponentId(sequenceId);
}

void DMXSequenceSystem::removeSequencesForGroup(MikanDMXFixtureGroupID groupId)
{
	// Collect first: the component map cannot be walked while it changes
	std::vector<MikanDMXSequenceID> sequenceIds;
	visitComponents([&sequenceIds](DMXSequenceComponentPtr sequence)
					{ sequenceIds.push_back(sequence->getComponentId()); }, [groupId](DMXSequenceComponentPtr sequence)
					{ return sequence->getDMXSequenceDefinition()->getGroupId() == groupId; });

	for (const MikanDMXSequenceID sequenceId : sequenceIds)
	{
		removeSequence(sequenceId);
	}
}

// -- Lua Binding ----
void DMXSequenceSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<DMXSequenceSystem>("DMXSequenceSystem")
		.addFunction("getSequenceById", [](DMXSequenceSystem* s, int id) -> DMXSequenceComponent*
					 { return s->getSequenceById(static_cast<MikanDMXSequenceID>(id)).get(); })
		.addFunction("getSequenceByName", [](DMXSequenceSystem* s, const std::string& name) -> DMXSequenceComponent*
					 { return s->getSequenceByName(name).get(); })
		.addFunction("getSequenceCount",
					 [](DMXSequenceSystem* s) -> int { return static_cast<int>(s->getComponentMap().size()); })
		.addFunction("getSequenceAtIndex",
					 [](DMXSequenceSystem* s, int i) -> DMXSequenceComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.addFunction("createSequence",
					 [](DMXSequenceSystem* s, int groupId, const std::string& name) -> DMXSequenceComponent*
					 { return s->createSequence(static_cast<MikanDMXFixtureGroupID>(groupId), name).get(); })
		.addFunction("removeSequence", [](DMXSequenceSystem* s, int sequenceId) -> bool
					 { return s->removeSequence(static_cast<MikanDMXSequenceID>(sequenceId)); })
		.endClass();
}
