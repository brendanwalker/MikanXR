#include "CompositorScriptContext.h"
#include "LuaMath.h"
#include "LuaStencil.h"

CompositorScriptContext::CompositorScriptContext()
	: CommonScriptContext()
{

}

CompositorScriptContext::~CompositorScriptContext()
{

}

bool CompositorScriptContext::bindContextFunctions()
{
	if (!CommonScriptContext::bindContextFunctions())
		return false;

	bindStencilFunctions();

	return true;
}

void CompositorScriptContext::bindStencilFunctions()
{
	LuaQuadStencilList::bindFunctions(m_luaState);
	LuaBoxStencilList::bindFunctions(m_luaState);
	LuaModelStencilList::bindFunctions(m_luaState);

	LuaStencilQuad::bindFunctions(m_luaState);
	LuaStencilBox::bindFunctions(m_luaState);
	LuaStencilModel::bindFunctions(m_luaState);
}