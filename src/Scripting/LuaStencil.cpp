#include "App.h"
#include "BoxStencilComponent.h"
#include "LuaStencil.h"
#include "MathTypeConversion.h"
#include "MikanClientTypes.h"
#include "ModelStencilComponent.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"

#include <algorithm>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- LuaQuadStencilList -----
LuaQuadStencilList::LuaQuadStencilList()
{
	ProfileConfigPtr profile = App::getInstance()->getProfileConfig();

	for (QuadStencilDefinitionPtr quadConfig : profile->stencilConfig->quadStencilList)
	{
		m_stencilIdList.push_back(quadConfig->getStencilId());
	}
}

void LuaQuadStencilList::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaQuadStencilList>("QuadStencilList")
		.addConstructor<void (*)()>()
		.addFunction("getStencilCount", &LuaQuadStencilList::getStencilCount)
		.addFunction("getStencilId", &LuaQuadStencilList::getStencilId)
		.endClass();
}

// -- LuaBoxStencilList -----
LuaBoxStencilList::LuaBoxStencilList()
{
	ProfileConfigPtr profile = App::getInstance()->getProfileConfig();

	for (BoxStencilDefinitionPtr boxStencil : profile->stencilConfig->boxStencilList)
	{
		m_stencilIdList.push_back(boxStencil->getStencilId());
	}
}

void LuaBoxStencilList::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaBoxStencilList>("BoxStencilList")
		.addConstructor<void (*)()>()
		.addFunction("getStencilCount", &LuaBoxStencilList::getStencilCount)
		.addFunction("getStencilId", &LuaBoxStencilList::getStencilId)
		.endClass();
}

// -- LuaModelStencilList -----
LuaModelStencilList::LuaModelStencilList()
{
	ProfileConfigPtr profile = App::getInstance()->getProfileConfig();

	for (ModelStencilDefinitionPtr modelStencil : profile->stencilConfig->modelStencilList)
	{
		m_stencilIdList.push_back(modelStencil->getStencilId());
	}
}

void LuaModelStencilList::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaModelStencilList>("ModelStencilList")
		.addConstructor<void (*)()>()
		.addFunction("getStencilCount", &LuaModelStencilList::getStencilCount)
		.addFunction("getStencilId", &LuaModelStencilList::getStencilId)
		.endClass();
}

// -- LuaStencilQuad -----
LuaStencilQuad::LuaStencilQuad(int stencilId)
	: m_stencilId(stencilId)
{
}

int LuaStencilQuad::getStencilId() const { return m_stencilId; }
int LuaStencilQuad::getParentSpatialAnchorId() const {
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getQuadStencilDefinition()->getParentAnchorId();
	}
	return -1;
}

LuaVec3f LuaStencilQuad::getQuadCenter() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getRelativeTransform().getPosition());
	}
	return result;
}
float LuaStencilQuad::getQuadWidth() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getQuadStencilDefinition()->getQuadWidth();
	}
	return 0.f;
}
float LuaStencilQuad::getQuadHeight() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getQuadStencilDefinition()->getQuadHeight();
	}
	return 0.f;
}
bool LuaStencilQuad::getIsDoubleSided() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getQuadStencilDefinition()->getIsDoubleSided();
	}
	return false;
}
bool LuaStencilQuad::getIsDisabled() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getQuadStencilDefinition()->getIsDisabled();
	}
	return false;
}

bool LuaStencilQuad::setQuadCenter(const LuaVec3f& center)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->setRelativePosition(center.toGlmVec3f());
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadWidth(float width)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getQuadStencilDefinition()->setQuadWidth(width);
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadHeight(float height)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getQuadStencilDefinition()->setQuadHeight(height);
		return true;
	}
	return false;
}
bool LuaStencilQuad::setIsDoubleSided(bool flag)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getQuadStencilDefinition()->setIsDoubleSided(flag);
		return true;
	}
	return false;
}
bool LuaStencilQuad::setIsDisabled(bool flag)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getQuadStencilDefinition()->setIsDisabled(flag);
		return true;
	}
	return false;
}

QuadStencilComponentConstPtr LuaStencilQuad::findConstStencilByID(int stencilId) const
{
	return StencilObjectSystem::getSystem()->getQuadStencilById(stencilId);
}

QuadStencilComponentPtr LuaStencilQuad::findStencilByID(int stencilId)
{
	return std::const_pointer_cast<QuadStencilComponent>(findConstStencilByID(stencilId));
}

void LuaStencilQuad::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaStencilQuad>("QuadStencil")
		.addConstructor<void (*)(int)>()
		.addFunction("getStencilId", &LuaStencilQuad::getStencilId)
		.addFunction("getParentSpatialAnchorId", &LuaStencilQuad::getParentSpatialAnchorId)
		.addFunction("getQuadCenter", &LuaStencilQuad::getQuadCenter)
		.addFunction("getQuadWidth", &LuaStencilQuad::getQuadWidth)
		.addFunction("getQuadHeight", &LuaStencilQuad::getQuadHeight)
		.addFunction("getIsDoubleSided", &LuaStencilQuad::getIsDoubleSided)
		.addFunction("getIsDisabled", &LuaStencilQuad::getIsDisabled)
		.addFunction("setQuadCenter", &LuaStencilQuad::setQuadCenter)
		.addFunction("setQuadWidth", &LuaStencilQuad::setQuadWidth)
		.addFunction("setQuadHeight", &LuaStencilQuad::setQuadHeight)
		.addFunction("setIsDoubleSided", &LuaStencilQuad::setIsDoubleSided)
		.addFunction("setIsDisabled", &LuaStencilQuad::setIsDisabled)
		.endClass();
}

// -- LuaStencilBox -----
LuaStencilBox::LuaStencilBox(int stencilId)
		: m_stencilId(stencilId)
{
}

int LuaStencilBox::getStencilId() const { return m_stencilId; }
int LuaStencilBox::getParentSpatialAnchorId() const {
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getBoxStencilDefinition()->getParentAnchorId();
	}
	return -1;
}

LuaVec3f LuaStencilBox::getBoxCenter() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getRelativeTransform().getPosition());
	}
	return result;
}
float LuaStencilBox::getBoxXSize() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getBoxStencilDefinition()->getBoxXSize();
	}
	return 0.f;
}
float LuaStencilBox::getBoxYSize() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getBoxStencilDefinition()->getBoxYSize();
	}
	return 0.f;
}
float LuaStencilBox::getBoxZSize() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getBoxStencilDefinition()->getBoxZSize();
	}
	return 0.f;
}
bool LuaStencilBox::getIsDisabled() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getBoxStencilDefinition()->getIsDisabled();
	}
	return false;
}

bool LuaStencilBox::setBoxCenter(const LuaVec3f& center)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->setRelativePosition(center.toGlmVec3f());
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxXSize(float size)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getBoxStencilDefinition()->setBoxXSize(size);
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxYSize(float size)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getBoxStencilDefinition()->setBoxYSize(size);
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxZSize(float size)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getBoxStencilDefinition()->setBoxZSize(size);
		return true;
	}
	return false;
}
bool LuaStencilBox::setIsDisabled(bool flag)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getBoxStencilDefinition()->setIsDisabled(flag);
		return true;
	}
	return false;
}

BoxStencilComponentConstPtr LuaStencilBox::findConstStencilByID(int stencilId) const
{
	return StencilObjectSystem::getSystem()->getBoxStencilById(stencilId);
}

BoxStencilComponentPtr LuaStencilBox::findStencilByID(int stencilId)
{
	return std::const_pointer_cast<BoxStencilComponent>(findConstStencilByID(stencilId));
}

void LuaStencilBox::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaStencilBox>("BoxStencil")
		.addConstructor<void (*)(int)>()
		.addFunction("getStencilId", &LuaStencilBox::getStencilId)
		.addFunction("getParentSpatialAnchorId", &LuaStencilBox::getParentSpatialAnchorId)
		.addFunction("getBoxCenter", &LuaStencilBox::getBoxCenter)
		.addFunction("getBoxXSize", &LuaStencilBox::getBoxXSize)
		.addFunction("getBoxYSize", &LuaStencilBox::getBoxYSize)
		.addFunction("getBoxZSize", &LuaStencilBox::getBoxZSize)
		.addFunction("getIsDisabled", &LuaStencilBox::getIsDisabled)
		.addFunction("setBoxCenter", &LuaStencilBox::setBoxCenter)
		.addFunction("setBoxXSize", &LuaStencilBox::setBoxXSize)
		.addFunction("setBoxYSize", &LuaStencilBox::setBoxYSize)
		.addFunction("setBoxZSize", &LuaStencilBox::setBoxZSize)
		.addFunction("setIsDisabled", &LuaStencilBox::setIsDisabled)
		.endClass();
}

// -- LuaStencilModel -----
LuaStencilModel::LuaStencilModel(int stencilId)
		: m_stencilId(stencilId)
{
}

int LuaStencilModel::getStencilId() const { return m_stencilId; }
int LuaStencilModel::getParentSpatialAnchorId() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getModelStencilDefinition()->getParentAnchorId();
	}
	return -1;
}

LuaVec3f LuaStencilModel::getModelPosition() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getRelativeTransform().getPosition());
	}
	return result;
}
LuaVec3f LuaStencilModel::getModelScale() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getRelativeTransform().getScale());
	}
	return result;
}
bool LuaStencilModel::getIsDisabled() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getModelStencilDefinition()->getIsDisabled();
	}
	return true;
}

bool LuaStencilModel::setModelPosition(const LuaVec3f & position)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->setRelativePosition(position.toGlmVec3f());
		return true;
	}
	return false;
}
bool LuaStencilModel::setModelScale(const LuaVec3f & scale)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->setRelativeScale(scale.toGlmVec3f());
		return true;
	}
	return false;
}
bool LuaStencilModel::setIsDisabled(bool flag)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getModelStencilDefinition()->setIsDisabled(flag);
		return true;
	}
	return false;
}

ModelStencilComponentConstPtr LuaStencilModel::findConstStencilByID(int stencilId) const
{
	return StencilObjectSystem::getSystem()->getModelStencilById(stencilId);
}

ModelStencilComponentPtr LuaStencilModel::findStencilByID(int stencilId)
{
	return std::const_pointer_cast<ModelStencilComponent>(findConstStencilByID(stencilId));
}

void LuaStencilModel::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaStencilModel>("ModelStencil")
		.addConstructor<void (*)(int)>()
		.addFunction("getStencilId", &LuaStencilModel::getStencilId)
		.addFunction("getParentSpatialAnchorId", &LuaStencilModel::getParentSpatialAnchorId)
		.addFunction("getModelPosition", &LuaStencilModel::getModelPosition)
		.addFunction("getModelScale", &LuaStencilModel::getModelScale)
		.addFunction("getIsDisabled", &LuaStencilModel::getIsDisabled)
		.addFunction("setModelPosition", &LuaStencilModel::setModelPosition)
		.addFunction("setModelScale", &LuaStencilModel::setModelScale)
		.addFunction("setIsDisabled", &LuaStencilModel::setIsDisabled)
		.endClass();
}