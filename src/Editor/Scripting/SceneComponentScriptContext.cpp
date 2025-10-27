#include "SceneComponentScriptContext.h"
#include "LuaMath.h"
#include "LuaStencil.h"

bool SceneComponentScriptContext::bindContextFunctions()
{
	if (!ComponentScriptContext::bindContextFunctions())
		return false;

	LuaQuadStencilList::bindFunctions(m_luaState);
	LuaBoxStencilList::bindFunctions(m_luaState);
	LuaModelStencilList::bindFunctions(m_luaState);

	LuaStencilQuad::bindFunctions(m_luaState);
	LuaStencilBox::bindFunctions(m_luaState);
	LuaStencilModel::bindFunctions(m_luaState);

	return true;
}