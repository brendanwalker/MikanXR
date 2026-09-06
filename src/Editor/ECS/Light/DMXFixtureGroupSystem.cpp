#include "DMXFixtureGroupSystem.h"
#include "DMXFixtureComponent.h"
#include "DMXPresetSystem.h"
#include "DMXSequenceSystem.h"
#include "MikanObject.h"
#include "ProjectManager.h"
#include "RGBPixelGridSystem.h"
#include "RGBSpotLightSystem.h"
#include "StageObjectSystem.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- DMXFixtureGroupSystemDefinition -----
DMXFixtureGroupSystemDefinition::DMXFixtureGroupSystemDefinition(const std::string& configName,
																 IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- DMXFixtureGroupSystem -----
DMXFixtureGroupSystem::DMXFixtureGroupSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

void DMXFixtureGroupSystem::postInit()
{
	Super::postInit();

	bindFixtureLifecycleEvents();
}

void DMXFixtureGroupSystem::dispose()
{
	// Registered after the fixture systems, so disposed before them
	unbindFixtureLifecycleEvents();

	Super::dispose();
}

MikanComponentPtr DMXFixtureGroupSystem::addNewObjectWithDefaultDefinition(const std::string& primaryComponentClass)
{
	if (primaryComponentClass != DMXFixtureGroupComponent::k_componentClassName)
		return MikanComponentPtr();

	MikanStageID stageId= INVALID_MIKAN_ID;
	if (StageObjectSystemPtr stageSystem= getOwnerProjectManager()->getSystemOfType<StageObjectSystem>())
	{
		stageId= stageSystem->getFirstStageId();
	}

	return createGroup(stageId, "");
}

DMXFixtureGroupComponentPtr DMXFixtureGroupSystem::createGroup(MikanStageID stageId, const std::string& name)
{
	return addNewObjectByTypedDefinition(
		[stageId, name](DMXFixtureGroupDefinitionPtr definition)
		{
			definition->setOwnerStageId(stageId);
			if (!name.empty())
				definition->setComponentName(name);
			return true;
		});
}

bool DMXFixtureGroupSystem::removeGroup(MikanDMXFixtureGroupID groupId)
{
	// Dependents go first, each as its own transaction, so a reverse-order
	// undo recreates the group before the presets that address it
	if (DMXPresetSystemPtr presetSystem= getOwnerProjectManager()->getSystemOfType<DMXPresetSystem>())
	{
		presetSystem->removePresetsForGroup(groupId);
	}
	if (DMXSequenceSystemPtr sequenceSystem= getOwnerProjectManager()->getSystemOfType<DMXSequenceSystem>())
	{
		sequenceSystem->removeSequencesForGroup(groupId);
	}

	return removeObjectByPrimaryComponentId(groupId);
}

void DMXFixtureGroupSystem::bindFixtureLifecycleEvents()
{
	ProjectManagerPtr projectManager= getOwnerProjectManager();
	if (!projectManager || m_bLifecycleEventsBound)
		return;

	if (RGBSpotLightSystemPtr spotLightSystem= projectManager->getSystemOfType<RGBSpotLightSystem>())
	{
		spotLightSystem->OnComponentDisposed+= MakeDelegate(this, &DMXFixtureGroupSystem::onFixtureComponentDisposed);
	}
	if (RGBPixelGridSystemPtr pixelGridSystem= projectManager->getSystemOfType<RGBPixelGridSystem>())
	{
		pixelGridSystem->OnComponentDisposed+= MakeDelegate(this, &DMXFixtureGroupSystem::onFixtureComponentDisposed);
	}
	m_bLifecycleEventsBound= true;
}

void DMXFixtureGroupSystem::unbindFixtureLifecycleEvents()
{
	ProjectManagerPtr projectManager= getOwnerProjectManager();
	if (!projectManager || !m_bLifecycleEventsBound)
		return;

	if (RGBSpotLightSystemPtr spotLightSystem= projectManager->getSystemOfType<RGBSpotLightSystem>())
	{
		spotLightSystem->OnComponentDisposed-= MakeDelegate(this, &DMXFixtureGroupSystem::onFixtureComponentDisposed);
	}
	if (RGBPixelGridSystemPtr pixelGridSystem= projectManager->getSystemOfType<RGBPixelGridSystem>())
	{
		pixelGridSystem->OnComponentDisposed-= MakeDelegate(this, &DMXFixtureGroupSystem::onFixtureComponentDisposed);
	}
	m_bLifecycleEventsBound= false;
}

void DMXFixtureGroupSystem::onFixtureComponentDisposed(MikanObjectSystemPtr objectSystem,
													   MikanComponentConstPtr component)
{
	// Every component of a fixture object disposes; only the fixture itself is a member
	auto fixture= std::dynamic_pointer_cast<const DMXFixtureComponent>(component);
	if (!fixture)
		return;

	const MikanLightID fixtureId= fixture->getComponentId();
	visitComponents([fixtureId](DMXFixtureGroupComponentPtr group)
					{ group->getDMXFixtureGroupDefinition()->removeFixture(fixtureId); });
}

// -- Lua Binding ----
void DMXFixtureGroupSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<DMXFixtureGroupSystem>("DMXFixtureGroupSystem")
		.addFunction("getGroupById", [](DMXFixtureGroupSystem* s, int id) -> DMXFixtureGroupComponent*
					 { return s->getGroupById(static_cast<MikanDMXFixtureGroupID>(id)).get(); })
		.addFunction("getGroupByName",
					 [](DMXFixtureGroupSystem* s, const std::string& name) -> DMXFixtureGroupComponent*
					 { return s->getGroupByName(name).get(); })
		.addFunction("getGroupCount",
					 [](DMXFixtureGroupSystem* s) -> int { return static_cast<int>(s->getComponentMap().size()); })
		.addFunction("getGroupAtIndex",
					 [](DMXFixtureGroupSystem* s, int i) -> DMXFixtureGroupComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.addFunction("createGroup",
					 [](DMXFixtureGroupSystem* s, int stageId, const std::string& name) -> DMXFixtureGroupComponent*
					 { return s->createGroup(static_cast<MikanStageID>(stageId), name).get(); })
		.addFunction("removeGroup", [](DMXFixtureGroupSystem* s, int groupId) -> bool
					 { return s->removeGroup(static_cast<MikanDMXFixtureGroupID>(groupId)); })
		.endClass();
}
