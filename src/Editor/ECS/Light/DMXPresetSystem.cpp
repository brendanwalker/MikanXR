#include "DMXPresetSystem.h"
#include "DMXFixtureGroupSystem.h"
#include "MikanObject.h"
#include "ProjectManager.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- DMXPresetSystemDefinition -----
DMXPresetSystemDefinition::DMXPresetSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- DMXPresetSystem -----
DMXPresetSystem::DMXPresetSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

MikanComponentPtr DMXPresetSystem::addNewObjectWithDefaultDefinition(const std::string& primaryComponentClass)
{
	if (primaryComponentClass != DMXPresetComponent::k_componentClassName)
		return MikanComponentPtr();

	MikanDMXFixtureGroupID groupId= INVALID_MIKAN_ID;
	if (DMXFixtureGroupSystemPtr groupSystem= getOwnerProjectManager()->getSystemOfType<DMXFixtureGroupSystem>())
	{
		groupId= groupSystem->getFirstComponentId();
	}

	return createPreset(groupId, "");
}

DMXPresetComponentPtr DMXPresetSystem::createPreset(MikanDMXFixtureGroupID groupId, const std::string& name)
{
	return addNewObjectByTypedDefinition(
		[groupId, name](DMXPresetDefinitionPtr definition)
		{
			definition->setGroupId(groupId);
			if (!name.empty())
				definition->setComponentName(name);
			return true;
		});
}

bool DMXPresetSystem::removePreset(MikanDMXPresetID presetId) { return removeObjectByPrimaryComponentId(presetId); }

void DMXPresetSystem::removePresetsForGroup(MikanDMXFixtureGroupID groupId)
{
	// Collect first: the component map cannot be walked while it changes
	std::vector<MikanDMXPresetID> presetIds;
	visitComponents([&presetIds](DMXPresetComponentPtr preset) { presetIds.push_back(preset->getComponentId()); },
					[groupId](DMXPresetComponentPtr preset)
					{ return preset->getDMXPresetDefinition()->getGroupId() == groupId; });

	for (const MikanDMXPresetID presetId : presetIds)
	{
		removePreset(presetId);
	}
}

// -- Lua Binding ----
void DMXPresetSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<DMXPresetSystem>("DMXPresetSystem")
		.addFunction("getPresetById", [](DMXPresetSystem* s, int id) -> DMXPresetComponent*
					 { return s->getPresetById(static_cast<MikanDMXPresetID>(id)).get(); })
		.addFunction("getPresetByName", [](DMXPresetSystem* s, const std::string& name) -> DMXPresetComponent*
					 { return s->getPresetByName(name).get(); })
		.addFunction("getPresetCount",
					 [](DMXPresetSystem* s) -> int { return static_cast<int>(s->getComponentMap().size()); })
		.addFunction("getPresetAtIndex",
					 [](DMXPresetSystem* s, int i) -> DMXPresetComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.addFunction("createPreset", [](DMXPresetSystem* s, int groupId, const std::string& name) -> DMXPresetComponent*
					 { return s->createPreset(static_cast<MikanDMXFixtureGroupID>(groupId), name).get(); })
		.addFunction("removePreset", [](DMXPresetSystem* s, int presetId) -> bool
					 { return s->removePreset(static_cast<MikanDMXPresetID>(presetId)); })
		.endClass();
}
