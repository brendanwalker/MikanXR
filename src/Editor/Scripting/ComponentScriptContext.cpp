#include "ComponentScriptContext.h"
#include "AnchorComponent.h"
#include "BoxStencilComponent.h"
#include "CameraComponent.h"
#include "CompositorComponent.h"
#include "DMXFixtureComponent.h"
#include "LuaMath.h"
#include "MarkerComponent.h"
#include "MikanComponent.h"
#include "ModelStencilComponent.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"
#include "QuadStencilComponent.h"
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

	luabridge::setGlobal(m_luaState, this, "ownerComponent");

	return true;
}
