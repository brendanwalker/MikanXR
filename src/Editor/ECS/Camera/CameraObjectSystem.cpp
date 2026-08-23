#include "CameraObjectSystem.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- CameraObjectSystemDefinition -----
CameraObjectSystemDefinition::CameraObjectSystemDefinition(const std::string& configName,
														   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- CameraObjectSystem -----
CameraObjectSystem::CameraObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

// -- Lua Binding ----
void CameraObjectSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<CameraObjectSystem>("CameraObjectSystem")
		.addFunction("getCameraById", [](CameraObjectSystem* s, int id) -> CameraComponent*
					 { return s->getCameraById(static_cast<MikanCameraID>(id)).get(); })
		.addFunction("getCameraByName", [](CameraObjectSystem* s, const std::string& name) -> CameraComponent*
					 { return s->getCameraByName(name).get(); })
		.addFunction("getCameraCount",
					 [](CameraObjectSystem* s) -> int { return static_cast<int>(s->getComponentMap().size()); })
		.addFunction("getCameraAtIndex",
					 [](CameraObjectSystem* s, int i) -> CameraComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.endClass();
}
