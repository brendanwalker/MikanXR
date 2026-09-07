#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "ProjectConfig.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- StageObjectSystemDefinition -----
StageObjectSystemDefinition::StageObjectSystemDefinition(const std::string& configName,
														 IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- StageObjectSystem -----
StageObjectSystem::StageObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

// -- Lua Binding ----
void StageObjectSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<StageObjectSystem>("StageObjectSystem")
		.addFunction("getStageById", [](StageObjectSystem* s, int id) -> StageComponent*
					 { return s->getStageById(static_cast<MikanStageID>(id)).get(); })
		.addFunction("getStageByName", [](StageObjectSystem* s, const std::string& name) -> StageComponent*
					 { return s->getStageByName(name).get(); })
		.addFunction("getFirstStageId",
					 [](StageObjectSystem* s) -> int { return static_cast<int>(s->getFirstStageId()); })
		.addFunction("getFirstStage", [](StageObjectSystem* s) -> StageComponent*
					 { return s->getStageById(s->getFirstStageId()).get(); })
		.addFunction("getStageCount",
					 [](StageObjectSystem* s) -> int { return static_cast<int>(s->getComponentMap().size()); })
		.addFunction("getStageAtIndex",
					 [](StageObjectSystem* s, int i) -> StageComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.endClass();
}
