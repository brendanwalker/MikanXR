#include "ComponentScriptContext.h"
#include "MikanComponent.h"
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

	MikanComponent::bindLuaFunctions(m_luaState);

	luabridge::setGlobal(m_luaState, this, "ownerComponent");

	return true;
}