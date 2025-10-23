#include "ComponentScriptContext.h"
#include "LuaMath.h"
#include "LuaStencil.h"

ComponentScriptContext::ComponentScriptContext()
	: CommonScriptContext()
{

}

ComponentScriptContext::~ComponentScriptContext()
{

}

bool ComponentScriptContext::bindContextFunctions()
{
	if (!CommonScriptContext::bindContextFunctions())
		return false;

	bindStencilFunctions();

	return true;
}

void ComponentScriptContext::bindStencilFunctions()
{
	LuaQuadStencilList::bindFunctions(m_luaState);
	LuaBoxStencilList::bindFunctions(m_luaState);
	LuaModelStencilList::bindFunctions(m_luaState);

	LuaStencilQuad::bindFunctions(m_luaState);
	LuaStencilBox::bindFunctions(m_luaState);
	LuaStencilModel::bindFunctions(m_luaState);
}