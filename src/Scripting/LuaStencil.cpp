#include "App.h"
#include "ProfileConfig.h"
#include "LuaStencil.h"
#include "MathTypeConversion.h"
#include "MikanClientTypes.h"

#include <algorithm>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- LuaQuadStencilList -----
LuaQuadStencilList::LuaQuadStencilList()
{
	const ProfileConfig* profile = App::getInstance()->getProfileConfig();

	for (const MikanStencilQuad& stencil : profile->quadStencilList)
	{
		m_stencilIdList.push_back(stencil.stencil_id);
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
	const ProfileConfig* profile = App::getInstance()->getProfileConfig();

	for (const MikanStencilBox& stencil : profile->boxStencilList)
	{
		m_stencilIdList.push_back(stencil.stencil_id);
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
	const ProfileConfig* profile = App::getInstance()->getProfileConfig();

	for (const MikanStencilModelConfig& stencil : profile->modelStencilList)
	{
		m_stencilIdList.push_back(stencil.modelInfo.stencil_id);
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
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->parent_anchor_id;
	}
	return -1;
}

LuaVec3f LuaStencilQuad::getQuadCenter() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->quad_center);
	}
	return result;
}
LuaVec3f LuaStencilQuad::getQuadXAxis() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->quad_x_axis);
	}
	return result;
}
LuaVec3f LuaStencilQuad::getQuadYAxis() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->quad_y_axis);
	}
	return result;
}
LuaVec3f LuaStencilQuad::getQuadNormal() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->quad_normal);
	}
	return result;
}
float LuaStencilQuad::getQuadWidth() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->quad_width;
	}
	return 0.f;
}
float LuaStencilQuad::getQuadHeight() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->quad_height;
	}
	return 0.f;
}
bool LuaStencilQuad::getIsDoubleSided() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->is_double_sided;
	}
	return false;
}
bool LuaStencilQuad::getIsDisabled() const
{
	const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->is_disabled;
	}
	return false;
}

bool LuaStencilQuad::setQuadCenter(const LuaVec3f& center)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->quad_center = center.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadXAxis(const LuaVec3f& x_axis)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->quad_x_axis = x_axis.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadYAxis(const LuaVec3f& y_axis)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->quad_y_axis = y_axis.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadNormal(const LuaVec3f& normal)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->quad_normal = normal.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadWidth(float width)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->quad_width = width;
		return true;
	}
	return false;
}
bool LuaStencilQuad::setQuadHeight(float height)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->quad_height = height;
		return true;
	}
	return false;
}
bool LuaStencilQuad::setIsDoubleSided(bool flag)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->is_double_sided = flag;
		return true;
	}
	return false;
}
bool LuaStencilQuad::setIsDisabled(bool flag)
{
	MikanStencilQuad* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->is_disabled = flag;
		return true;
	}
	return false;
}

const MikanStencilQuad* LuaStencilQuad::findConstStencilByID(int stencilId) const
{
	ProfileConfig* profile = App::getInstance()->getProfileConfig();

	auto it = std::find_if(
		profile->quadStencilList.begin(),
		profile->quadStencilList.end(),
		[stencilId](const MikanStencilQuad& stencil)
	{
		return stencil.stencil_id == stencilId;
	});

	if (it != profile->quadStencilList.end())
	{
		const MikanStencilQuad& stencil = *it;

		return &stencil;
	}
	else
	{
		return nullptr;
	}
}

MikanStencilQuad* LuaStencilQuad::findStencilByID(int stencilId)
{
	return const_cast<MikanStencilQuad*>(findConstStencilByID(stencilId));
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
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->parent_anchor_id;
	}
	return -1;
}

LuaVec3f LuaStencilBox::getBoxCenter() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->box_center);
	}
	return result;
}
LuaVec3f LuaStencilBox::getBoxXAxis() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->box_x_axis);
	}
	return result;
}
LuaVec3f LuaStencilBox::getBoxYAxis() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->box_y_axis);
	}
	return result;
}
LuaVec3f LuaStencilBox::getBoxZAxis() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->box_z_axis);
	}
	return result;
}
float LuaStencilBox::getBoxXSize() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->box_x_size;
	}
	return 0.f;
}
float LuaStencilBox::getBoxYSize() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->box_y_size;
	}
	return 0.f;
}
float LuaStencilBox::getBoxZSize() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->box_z_size;
	}
	return 0.f;
}
bool LuaStencilBox::getIsDisabled() const
{
	const MikanStencilBox* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->is_disabled;
	}
	return false;
}

bool LuaStencilBox::setBoxCenter(const LuaVec3f& center)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_center = center.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxXAxis(const LuaVec3f& x_axis)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_x_axis = x_axis.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxYAxis(const LuaVec3f& y_axis)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_y_axis = y_axis.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxZAxis(const LuaVec3f& normal)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_z_axis = normal.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxXSize(float size)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_x_size = size;
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxYSize(float size)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_y_size = size;
		return true;
	}
	return false;
}
bool LuaStencilBox::setBoxZSize(float size)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->box_z_size = size;
		return true;
	}
	return false;
}
bool LuaStencilBox::setIsDisabled(bool flag)
{
	MikanStencilBox* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->is_disabled = flag;
		return true;
	}
	return false;
}

const MikanStencilBox* LuaStencilBox::findConstStencilByID(int stencilId) const
{
	ProfileConfig* profile = App::getInstance()->getProfileConfig();

	auto it = std::find_if(
		profile->boxStencilList.begin(),
		profile->boxStencilList.end(),
		[stencilId](const MikanStencilBox& stencil)
	{
		return stencil.stencil_id == stencilId;
	});

	if (it != profile->boxStencilList.end())
	{
		const MikanStencilBox& stencil = *it;

		return &stencil;
	}
	else
	{
		return nullptr;
	}
}

MikanStencilBox* LuaStencilBox::findStencilByID(int stencilId)
{
	return const_cast<MikanStencilBox*>(findConstStencilByID(stencilId));
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
	const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->modelInfo.parent_anchor_id;
	}
	return -1;
}

LuaVec3f LuaStencilModel::getModelPosition() const
{
	const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->modelInfo.model_position);
	}
	return result;
}
LuaVec3f LuaStencilModel::getModelRotator() const
{
	const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->modelInfo.model_rotator);
	}
	return result;
}
LuaVec3f LuaStencilModel::getModelScale() const
{
	const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
	LuaVec3f result;
	if (stencil != nullptr)
	{
		result = LuaVec3f(stencil->modelInfo.model_scale);
	}
	return result;
}
bool LuaStencilModel::getIsDisabled() const
{
	const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		return stencil->modelInfo.is_disabled;
	}
	return true;
}

bool LuaStencilModel::setModelPosition(const LuaVec3f & position)
{
	MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->modelInfo.model_position = position.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilModel::setModelRotator(const LuaVec3f & rotator)
{
	MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->modelInfo.model_rotator = rotator.toMikanRotator3f();
		return true;
	}
	return false;
}
bool LuaStencilModel::setModelScale(const LuaVec3f & scale)
{
	MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->modelInfo.model_scale = scale.toMikanVector3f();
		return true;
	}
	return false;
}
bool LuaStencilModel::setIsDisabled(bool flag)
{
	MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
	if (stencil != nullptr)
	{
		stencil->modelInfo.is_disabled = flag;
		return true;
	}
	return false;
}

const MikanStencilModelConfig* LuaStencilModel::findConstStencilByID(int stencilId) const
{
	return App::getInstance()->getProfileConfig()->getModelStencilConfig(stencilId);
}

MikanStencilModelConfig* LuaStencilModel::findStencilByID(int stencilId)
{
	return const_cast<MikanStencilModelConfig*>(findConstStencilByID(stencilId));
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