#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "CameraComponent.h"
#include "CompositorComponent.h"
#include "DMXFixtureComponent.h"
#include "DMXObjectSystem.h"
#include "LuaMath.h"
#include "MarkerComponent.h"
#include "MikanComponent.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "ProjectManager.h"
#include "ProjectScriptContext.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "QuadStencilComponent.h"
#include "QuadStencilSystem.h"
#include "ShapeComponent.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "StageComponent.h"
#include "TransformComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

ProjectScriptContext::ProjectScriptContext(ProjectManagerPtr projectManager)
	: CommonScriptContext()
	, m_projectManager(projectManager)
{
}

bool ProjectScriptContext::bindContextFunctions()
{
	if (!CommonScriptContext::bindContextFunctions())
		return false;

	ProjectManagerPtr projectManager= m_projectManager.lock();
	if (!projectManager)
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
	ModelShapeSystem::bindLuaFunctions(m_luaState);
	BoxShapeSystem::bindLuaFunctions(m_luaState);
	QuadShapeSystem::bindLuaFunctions(m_luaState);

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
	ShapeComponent::bindLuaFunctions(m_luaState);
	QuadShapeComponent::bindLuaFunctions(m_luaState);
	BoxShapeComponent::bindLuaFunctions(m_luaState);
	ModelShapeComponent::bindLuaFunctions(m_luaState);
	DMXFixtureComponent::bindLuaFunctions(m_luaState);
	RGBSpotLightComponent::bindLuaFunctions(m_luaState);
	RGBPixelGridComponent::bindLuaFunctions(m_luaState);
	CameraComponent::bindLuaFunctions(m_luaState);
	AnchorComponent::bindLuaFunctions(m_luaState);
	MarkerComponent::bindLuaFunctions(m_luaState);

	// Expose every scriptable object system as a global so scripts can look up
	// objects by name or id
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<CameraObjectSystem>().get(), "CameraSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<SceneObjectSystem>().get(), "SceneSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<AnchorObjectSystem>().get(), "AnchorSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<CompositorObjectSystem>().get(),
						 "CompositorSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<DMXObjectSystem>().get(), "DMXSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<ModelStencilSystem>().get(), "ModelStencilSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<BoxStencilSystem>().get(), "BoxStencilSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<QuadStencilSystem>().get(), "QuadStencilSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<ModelShapeSystem>().get(), "ModelShapeSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<BoxShapeSystem>().get(), "BoxShapeSystem");
	luabridge::setGlobal(m_luaState, projectManager->getSystemOfType<QuadShapeSystem>().get(), "QuadShapeSystem");

	return true;
}
