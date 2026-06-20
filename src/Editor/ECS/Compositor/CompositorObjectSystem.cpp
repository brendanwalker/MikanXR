#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "ProjectConfig.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- CompositorObjectSystemDefinition -----
CompositorObjectSystemDefinition::CompositorObjectSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- CompositorObjectSystem -----
CompositorObjectSystem::CompositorObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

std::vector<MikanCompositorID> CompositorObjectSystem::getCompositorIdListForStage(MikanStageID stageId) const
{
	std::vector<MikanCompositorID> compositorIdList;

	for (const auto& compositorPair : Super::getComponentMap())
	{
		CompositorComponentPtr componentPtr= compositorPair.second.lock();

		if (componentPtr && componentPtr->getOwnerStageId() == stageId)
		{
			compositorIdList.push_back(compositorPair.first);
		}
	}

	return compositorIdList;
}

void CompositorObjectSystem::setActiveCompositors(
	const std::vector<MikanCompositorID>& activeCompositorIdList)
{
	// Iterate through all compositor components
	for (const auto& compositorPair : Super::getComponentMap())
	{
		MikanCompositorID compositorId= compositorPair.first;
		CompositorComponentPtr compositor= compositorPair.second.lock();

		if (compositor)
		{
			// Check if this compositor should be active
			bool shouldBeActive= std::find(activeCompositorIdList.begin(), activeCompositorIdList.end(), compositorId) != activeCompositorIdList.end();
			bool isCurrentlyRunning= compositor->getIsRunning();

			if (shouldBeActive && !isCurrentlyRunning)
			{
				// Start the compositor if it should be active but isn't running
				if (compositor->start())
				{
					OnCompositorActivated(compositor);
				}
			}
			else if (!shouldBeActive && isCurrentlyRunning)
			{
				// Stop the compositor if it shouldn't be active but is running
				compositor->stop();
				OnCompositorDeactivated(compositor);
			}
		}
	}
}
// -- Lua Binding ----
void CompositorObjectSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<CompositorObjectSystem>("CompositorObjectSystem")
		.addFunction("getCompositorById",
					 [](CompositorObjectSystem* s, int id) -> CompositorComponent*
					 {
						 return s->getCompositorById(static_cast<MikanCompositorID>(id)).get();
					 })
		.addFunction("getCompositorByName",
					 [](CompositorObjectSystem* s, const std::string& name) -> CompositorComponent*
					 {
						 return s->getCompositorByName(name).get();
					 })
		.addFunction("getCompositorCount",
					 [](CompositorObjectSystem* s) -> int
					 {
						 return static_cast<int>(s->getComponentMap().size());
					 })
		.addFunction("getCompositorAtIndex",
					 [](CompositorObjectSystem* s, int i) -> CompositorComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.addFunction("getCompositorCountForStage",
					 [](CompositorObjectSystem* s, int stageId) -> int
					 {
						 return static_cast<int>(
							 s->getCompositorIdListForStage(static_cast<MikanStageID>(stageId)).size());
					 })
		.addFunction("getCompositorForStageAtIndex",
					 [](CompositorObjectSystem* s, int stageId, int i) -> CompositorComponent*
					 {
						 auto ids= s->getCompositorIdListForStage(static_cast<MikanStageID>(stageId));
						 if (i >= 0 && i < static_cast<int>(ids.size()))
							 return s->getCompositorById(ids[i]).get();
						 return nullptr;
					 })
		.endClass();
}
