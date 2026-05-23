#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "ComponentScriptContext.h"
#include "CameraComponent.h"
#include "CompositorComponent.h"
#include "DMXFixtureComponent.h"
#include "DMXObjectSystem.h"
#include "LuaMath.h"
#include "MarkerComponent.h"
#include "MikanComponent.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "QuadStencilComponent.h"
#include "QuadStencilSystem.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "StageComponent.h"
#include "TransformComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

ComponentScriptContext::ComponentScriptContext(MikanComponentPtr ownerComponent)
	: CommonScriptContext()
	, m_ownerComponent(ownerComponent)
{

}

bool ComponentScriptContext::bindContextFunctions()
{
	if (!CommonScriptContext::bindContextFunctions())
		return false;

	// Register object system classes before component classes
	CameraObjectSystem::bindLuaFunctions(m_luaState);
	SceneObjectSystem::bindLuaFunctions(m_luaState);
	DMXObjectSystem::bindLuaFunctions(m_luaState);
	AnchorObjectSystem::bindLuaFunctions(m_luaState);
	CompositorObjectSystem::bindLuaFunctions(m_luaState);
	ModelStencilSystem::bindLuaFunctions(m_luaState);
	BoxStencilSystem::bindLuaFunctions(m_luaState);
	QuadStencilSystem::bindLuaFunctions(m_luaState);

	// Bind in dependency order: parent classes before derived classes
	MikanComponent::bindLuaFunctions(m_luaState);
	CompositorComponent::bindLuaFunctions(m_luaState);
	TransformComponent::bindLuaFunctions(m_luaState);
	SceneComponent::bindLuaFunctions(m_luaState);
	StageComponent::bindLuaFunctions(m_luaState);
	StencilComponent::bindLuaFunctions(m_luaState);
	QuadStencilComponent::bindLuaFunctions(m_luaState);
	BoxStencilComponent::bindLuaFunctions(m_luaState);
	ModelStencilComponent::bindLuaFunctions(m_luaState);
	DMXFixtureComponent::bindLuaFunctions(m_luaState);
	RGBSpotLightComponent::bindLuaFunctions(m_luaState);
	RGBPixelGridComponent::bindLuaFunctions(m_luaState);
	CameraComponent::bindLuaFunctions(m_luaState);
	AnchorComponent::bindLuaFunctions(m_luaState);
	MarkerComponent::bindLuaFunctions(m_luaState);

	MikanComponent* ownerComponent= m_ownerComponent.lock().get();
	luabridge::setGlobal(m_luaState, ownerComponent, "ownerComponent");

	// Expose stencil system singletons so scripts can look up stencils by name
	luabridge::setGlobal(m_luaState,
		ownerComponent->getObjectSystemOfType<ModelStencilSystem>().get(),
		"ModelStencilSystem");
	luabridge::setGlobal(m_luaState,
		ownerComponent->getObjectSystemOfType<BoxStencilSystem>().get(),
		"BoxStencilSystem");
	luabridge::setGlobal(m_luaState,
		ownerComponent->getObjectSystemOfType<QuadStencilSystem>().get(),
		"QuadStencilSystem");

	return true;
}
