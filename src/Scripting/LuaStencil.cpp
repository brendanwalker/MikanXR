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

	for (QuadStencilConfigPtr quadConfig : profile->stencilConfig->quadStencilList)
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

	for (BoxStencilConfigPtr boxStencil : profile->stencilConfig->boxStencilList)
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

	for (ModelStencilConfigPtr modelStencil : profile->stencilConfig->modelStencilList)
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
		return stencil->getConfig()->getParentAnchorId();
	}
	return -1;
}

LuaVec3f LuaStencilQuad::getQuadCenter() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getQuadCenter());
	}
	return result;
}
LuaVec3f LuaStencilQuad::getQuadXAxis() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getQuadXAxis());
	}
	return result;
}
LuaVec3f LuaStencilQuad::getQuadYAxis() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getQuadYAxis());
	}
	return result;
}
LuaVec3f LuaStencilQuad::getQuadNormal() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getQuadNormal());
	}
	return result;
}
float LuaStencilQuad::getQuadWidth() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getQuadWidth();
	}
	return 0.f;
}
float LuaStencilQuad::getQuadHeight() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getQuadHeight();
	}
	return 0.f;
}
bool LuaStencilQuad::getIsDoubleSided() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getIsDoubleSided();
	}
	return false;
}
bool LuaStencilQuad::getIsDisabled() const
{
	QuadStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getIsDisabled();
	}
	return false;
}

bool LuaStencilQuad::setQuadCenter(const LuaVec3f& center)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setQuadCenter(center.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadXAxis(const LuaVec3f& x_axis)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setQuadXAxis(x_axis.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadYAxis(const LuaVec3f& y_axis)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setQuadYAxis(y_axis.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadNormal(const LuaVec3f& normal)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setQuadNormal(normal.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadWidth(float width)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setQuadWidth(width);
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadHeight(float height)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setQuadHeight(height);
		return true;
	}
	return false;
}
bool LuaStencilQuad::setIsDoubleSided(bool flag)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setIsDoubleSided(flag);
		return true;
	}
	return false;
}
bool LuaStencilQuad::setIsDisabled(bool flag)
{
	QuadStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setIsDisabled(flag);
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
		.addFunction("getQuadXAxis", &LuaStencilQuad::getQuadXAxis)
		.addFunction("getQuadYAxis", &LuaStencilQuad::getQuadYAxis)
		.addFunction("getQuadNormal", &LuaStencilQuad::getQuadNormal)
		.addFunction("getQuadWidth", &LuaStencilQuad::getQuadWidth)
		.addFunction("getQuadHeight", &LuaStencilQuad::getQuadHeight)
		.addFunction("getIsDoubleSided", &LuaStencilQuad::getIsDoubleSided)
		.addFunction("getIsDisabled", &LuaStencilQuad::getIsDisabled)
		.addFunction("setQuadCenter", &LuaStencilQuad::setQuadCenter)
		.addFunction("setQuadXAxis", &LuaStencilQuad::setQuadXAxis)
		.addFunction("setQuadYAxis", &LuaStencilQuad::setQuadYAxis)
		.addFunction("setQuadNormal", &LuaStencilQuad::setQuadNormal)
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
		return stencil->getConfig()->getParentAnchorId();
	}
	return -1;
}

LuaVec3f LuaStencilBox::getBoxCenter() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getBoxCenter());
	}
	return result;
}
LuaVec3f LuaStencilBox::getBoxXAxis() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getBoxXAxis());
	}
	return result;
}
LuaVec3f LuaStencilBox::getBoxYAxis() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getBoxYAxis());
	}
	return result;
}
LuaVec3f LuaStencilBox::getBoxZAxis() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getBoxZAxis());
	}
	return result;
}
float LuaStencilBox::getBoxXSize() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getBoxXSize();
	}
	return 0.f;
}
float LuaStencilBox::getBoxYSize() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getBoxYSize();
	}
	return 0.f;
}
float LuaStencilBox::getBoxZSize() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getBoxZSize();
	}
	return 0.f;
}
bool LuaStencilBox::getIsDisabled() const
{
	BoxStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getIsDisabled();
	}
	return false;
}

bool LuaStencilBox::setBoxCenter(const LuaVec3f& center)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxCenter(center.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxXAxis(const LuaVec3f& x_axis)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxXAxis(x_axis.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxYAxis(const LuaVec3f& y_axis)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxYAxis(y_axis.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxZAxis(const LuaVec3f& z_axis)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxZAxis(z_axis.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxXSize(float size)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxXSize(size);
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxYSize(float size)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxYSize(size);
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxZSize(float size)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setBoxZSize(size);
		return true;
	}
	return false;
}
bool LuaStencilBox::setIsDisabled(bool flag)
{
	BoxStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setIsDisabled(flag);
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
		.addFunction("getBoxXAxis", &LuaStencilBox::getBoxXAxis)
		.addFunction("getBoxYAxis", &LuaStencilBox::getBoxYAxis)
		.addFunction("getBoxZAxis", &LuaStencilBox::getBoxZAxis)
		.addFunction("getBoxXSize", &LuaStencilBox::getBoxXSize)
		.addFunction("getBoxYSize", &LuaStencilBox::getBoxYSize)
		.addFunction("getBoxZSize", &LuaStencilBox::getBoxZSize)
		.addFunction("getIsDisabled", &LuaStencilBox::getIsDisabled)
		.addFunction("setBoxCenter", &LuaStencilBox::setBoxCenter)
		.addFunction("setBoxXAxis", &LuaStencilBox::setBoxXAxis)
		.addFunction("setBoxYAxis", &LuaStencilBox::setBoxYAxis)
		.addFunction("setBoxZAxis", &LuaStencilBox::setBoxZAxis)
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
		return stencil->getConfig()->getParentAnchorId();
	}
	return -1;
}

LuaVec3f LuaStencilModel::getModelPosition() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getModelPosition());
	}
	return result;
}
LuaVec3f LuaStencilModel::getModelRotator() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getModelRotator());
	}
	return result;
}
LuaVec3f LuaStencilModel::getModelScale() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->getConfig()->getModelScale());
	}
	return result;
}
bool LuaStencilModel::getIsDisabled() const
{
	ModelStencilComponentConstPtr stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->getConfig()->getIsDisabled();
	}
	return true;
}

bool LuaStencilModel::setModelPosition(const LuaVec3f & position)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setModelPosition(position.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilModel::setModelRotator(const LuaVec3f & rotator)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setModelRotator(rotator.toMikanRotator3f());
		return true;
	}
	return false;
}
bool LuaStencilModel::setModelScale(const LuaVec3f & scale)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setModelScale(scale.toMikanVector3f());
		return true;
	}
	return false;
}
bool LuaStencilModel::setIsDisabled(bool flag)
{
	ModelStencilComponentPtr stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->getConfig()->setIsDisabled(flag);
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
		.addFunction("getModelRotator", &LuaStencilModel::getModelRotator)
		.addFunction("getModelScale", &LuaStencilModel::getModelScale)
		.addFunction("getIsDisabled", &LuaStencilModel::getIsDisabled)
		.addFunction("setModelPosition", &LuaStencilModel::setModelPosition)
		.addFunction("setModelRotator", &LuaStencilModel::setModelRotator)
		.addFunction("setModelScale", &LuaStencilModel::setModelScale)
		.addFunction("setIsDisabled", &LuaStencilModel::setIsDisabled)
		.endClass();
}