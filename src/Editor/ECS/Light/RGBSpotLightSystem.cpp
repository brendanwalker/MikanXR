#include "RGBSpotLightSystem.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "Transform.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- RGBSpotLightSystemDefinition -----
RGBSpotLightSystemDefinition::RGBSpotLightSystemDefinition(const std::string& configName,
														   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- RGBSpotLightSystem -----
RGBSpotLightSystem::RGBSpotLightSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}

void RGBSpotLightSystem::additionalComponentFactory(MikanObjectPtr ownerComponentObject,
													ComponentDefinitionPtr componentDefinition)
{
	// Add a selection component so the mesh can be clicked in the viewport
	ownerComponentObject->addComponent<SelectionComponent>();

	// Build mesh + collider components for the spot light model
	RGBSpotLightComponentPtr lightComponentPtr= ownerComponentObject->getComponentOfType<RGBSpotLightComponent>();
	if (lightComponentPtr)
		lightComponentPtr->rebuildMeshComponents();
}

RGBSpotLightComponentPtr RGBSpotLightSystem::createLight(MikanStageID stageId, const std::string& name)
{
	return addNewObjectByTypedDefinition(
		[stageId, name](auto def)
		{
			def->setOwnerStageId(stageId);
			def->setParentTransformId(stageId);
			def->setRelativeTransform(GlmTransform());
			if (!name.empty())
				def->setComponentName(name);
			return true;
		});
}

bool RGBSpotLightSystem::removeLight(MikanLightID lightId) { return removeObjectByPrimaryComponentId(lightId); }

void RGBSpotLightSystem::renderConeVolumes(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	visitComponents([graphicsContext, viewportCamera](RGBSpotLightComponentPtr light)
					{ light->renderConeVolume(graphicsContext, viewportCamera); });
}

// -- Lua Binding ----
void RGBSpotLightSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<RGBSpotLightSystem>("RGBSpotLightSystem")
		.addFunction("getLightById", [](RGBSpotLightSystem* s, int id) -> RGBSpotLightComponent*
					 { return s->getLightById(static_cast<MikanLightID>(id)).get(); })
		.addFunction("getLightByName", [](RGBSpotLightSystem* s, const std::string& name) -> RGBSpotLightComponent*
					 { return s->getLightByName(name).get(); })
		.addFunction("getLightCount",
					 [](RGBSpotLightSystem* s) -> int { return static_cast<int>(s->getComponentMap().size()); })
		.addFunction("getLightAtIndex",
					 [](RGBSpotLightSystem* s, int i) -> RGBSpotLightComponent*
					 {
						 int n= 0;
						 for (auto& [id, wp] : s->getComponentMap())
							 if (n++ == i)
								 return wp.lock().get();
						 return nullptr;
					 })
		.addFunction("createLight",
					 [](RGBSpotLightSystem* s, int stageId, const std::string& name) -> RGBSpotLightComponent*
					 { return s->createLight(static_cast<MikanStageID>(stageId), name).get(); })
		.addFunction("removeLight", [](RGBSpotLightSystem* s, int lightId) -> bool
					 { return s->removeLight(static_cast<MikanLightID>(lightId)); })
		.endClass();
}