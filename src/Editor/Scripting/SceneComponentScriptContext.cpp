#include "SceneComponentScriptContext.h"
#include "BoxStencilComponent.h"
#include "LuaMath.h"
#include "ModelStencilComponent.h"
#include "QuadStencilComponent.h"
#include "SceneComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

SceneComponentScriptContext::SceneComponentScriptContext(
	MikanComponentPtr ownerComponent)
	: ComponentScriptContext(ownerComponent)
{
}

bool SceneComponentScriptContext::bindContextFunctions()
{
	if (!ComponentScriptContext::bindContextFunctions())
		return false;

	// Bind Lua functions for components used in scenes
	StencilComponent::bindLuaFunctions(m_luaState);
	QuadStencilComponent::bindLuaFunctions(m_luaState);
	BoxStencilComponent::bindLuaFunctions(m_luaState);
	ModelStencilComponent::bindLuaFunctions(m_luaState);
	SceneComponent::bindLuaFunctions(m_luaState);

	luabridge::setGlobal(m_luaState, this, "ownerSceneComponent");

	return true;
}