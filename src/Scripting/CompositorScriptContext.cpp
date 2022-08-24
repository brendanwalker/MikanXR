#include "App.h"
#include "ProfileConfig.h"
#include "CompositorScriptContext.h"
#include "Logger.h"
#include "LuaMath.h"
#include "MathTypeConversion.h"
#include "MikanClientTypes.h"

#include <algorithm>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#endif
#include <assert.h>
#include "luaaa.hpp"
#ifdef _MSC_VER
#pragma warning (pop)
#endif

using namespace luaaa;

class LuaQuadStencilList
{
public:
	LuaQuadStencilList()
	{
		const ProfileConfig* profile = App::getInstance()->getProfileConfig();

		for (const MikanStencilQuad& stencil : profile->quadStencilList)
		{
			m_stencilIdList.push_back(stencil.stencil_id);
		}
	}

	int getStencilCount() const { return (int)m_stencilIdList.size(); }
	int getStencilId(int listIndex) const { return m_stencilIdList[listIndex]; }

private:
	std::vector<int> m_stencilIdList;
};

class LuaModelStencilList
{
public:
	LuaModelStencilList()
	{
		const ProfileConfig* profile = App::getInstance()->getProfileConfig();

		for (const MikanStencilModelConfig& stencil : profile->modelStencilList)
		{
			m_stencilIdList.push_back(stencil.modelInfo.stencil_id);
		}
	}

	int getStencilCount() const { return (int)m_stencilIdList.size(); }
	int getStencilId(int listIndex) const { return m_stencilIdList[listIndex]; }

private:
	std::vector<int> m_stencilIdList;
};

class LuaStencilQuad
{
public:
	LuaStencilQuad(int stencilId)
		: m_stencilId(stencilId)
	{
	}

	int getStencilId() const { return m_stencilId; }
	int getParentSpatialAnchorId() const { 
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->parent_anchor_id;
		}
		return -1;
	}

	LuaVec3f getQuadCenter() const 
	{ 
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->quad_center);
		}
		return result;
	}
	LuaVec3f getQuadXAxis() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->quad_x_axis);
		}
		return result;
	}
	LuaVec3f getQuadYAxis() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->quad_y_axis);
		}
		return result;
	}
	LuaVec3f getQuadNormal() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->quad_normal);
		}
		return result;
	}
	float getQuadWidth() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->quad_width;
		}
		return 0.f;
	}
	float getQuadHeight() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->quad_height;
		}
		return 0.f;
	}
	bool getIsDoubleSided() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->is_double_sided;
		}
		return false;
	}
	bool getIsDisabled() const
	{
		const MikanStencilQuad* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->is_disabled;
		}
		return false;
	}

	bool setQuadCenter(const LuaVec3f& center)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->quad_center = center.toMikanVector3f();
			return true;
		}
		return false;
	}
	bool setQuadXAxis(const LuaVec3f& x_axis)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->quad_x_axis = x_axis.toMikanVector3f();
			return true;
		}
		return false;
	}
	bool setQuadYAxis(const LuaVec3f& y_axis)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->quad_y_axis = y_axis.toMikanVector3f();
			return true;
		}
		return false;
	}
	bool setQuadNormal(const LuaVec3f& normal)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->quad_normal = normal.toMikanVector3f();
			return true;
		}
		return false;
	}
	bool setQuadWidth(float width)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->quad_width = width;
			return true;
		}
		return false;
	}
	bool setQuadHeight(float height)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->quad_height = height;
			return true;
		}
		return false;
	}
	bool setIsDoubleSided(bool flag)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->is_double_sided = flag;
			return true;
		}
		return false;
	}
	bool setIsDisabled(bool flag)
	{
		MikanStencilQuad* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->is_disabled = flag;
			return true;
		}
		return false;
	}

private:
	const MikanStencilQuad* findConstStencilByID(int stencilId) const
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

	MikanStencilQuad* findStencilByID(int stencilId)
	{
		return const_cast<MikanStencilQuad*>(findConstStencilByID(stencilId));
	}

	MikanStencilID m_stencilId; // filled in on allocation
};

class LuaStencilModel
{
public:
	LuaStencilModel(int stencilId)
		: m_stencilId(stencilId)
	{
	}

	int getStencilId() const { return m_stencilId; }
	int getParentSpatialAnchorId() const
	{
		const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->modelInfo.parent_anchor_id;
		}
		return -1;
	}

	LuaVec3f getModelPosition() const
	{ 
		const MikanStencilModelConfig* stencil= findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->modelInfo.model_position);
		}
		return result;
	}
	LuaVec3f getModelRotator() const
	{
		const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->modelInfo.model_rotator);
		}
		return result;
	}
	LuaVec3f getModelScale() const
	{
		const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
		LuaVec3f result;
		if (stencil != nullptr)
		{
			result = LuaVec3f(stencil->modelInfo.model_scale);
		}
		return result;
	}
	bool getIsDisabled() const 
	{ 
		const MikanStencilModelConfig* stencil = findConstStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			return stencil->modelInfo.is_disabled;
		}
		return true;
	}

	bool setModelPosition(const LuaVec3f& position)
	{
		MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->modelInfo.model_position= position.toMikanVector3f();
			return true;
		}
		return false;
	}
	bool setModelRotator(const LuaVec3f& rotator)
	{
		MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->modelInfo.model_rotator = rotator.toMikanRotator3f();
			return true;
		}
		return false;
	}
	bool setModelScale(const LuaVec3f& scale)
	{
		MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->modelInfo.model_scale = scale.toMikanVector3f();
			return true;
		}
		return false;
	}
	bool setIsDisabled(bool flag)
	{
		MikanStencilModelConfig* stencil = findStencilByID(m_stencilId);
		if (stencil != nullptr)
		{
			stencil->modelInfo.is_disabled = flag;
			return true;
		}
		return false;
	}

private:
	const MikanStencilModelConfig* findConstStencilByID(int stencilId) const
	{
		ProfileConfig* profile = App::getInstance()->getProfileConfig();

		auto it = std::find_if(
			profile->modelStencilList.begin(),
			profile->modelStencilList.end(),
			[stencilId](const MikanStencilModelConfig& stencil)
		{
			return stencil.modelInfo.stencil_id == stencilId;
		});

		if (it != profile->modelStencilList.end())
		{
			const MikanStencilModelConfig& modelConfig= *it;

			return &modelConfig;
		}
		else
		{
			return nullptr;
		}
	}

	MikanStencilModelConfig* findStencilByID(int stencilId)
	{
		return const_cast<MikanStencilModelConfig*>(findConstStencilByID(stencilId));
	}

	MikanStencilID m_stencilId; // filled in on allocation
};

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
	LuaClass<LuaQuadStencilList> luaQuadStencilList(m_luaState, "QuadStencilList");
	luaQuadStencilList.ctor();
	luaQuadStencilList.fun("getStencilCount", &LuaQuadStencilList::getStencilCount);
	luaQuadStencilList.fun("getStencilId", &LuaQuadStencilList::getStencilId);

	LuaClass<LuaModelStencilList> luaModelStencilList(m_luaState, "ModelStencilList");
	luaModelStencilList.ctor();
	luaModelStencilList.fun("getStencilCount", &LuaModelStencilList::getStencilCount);
	luaModelStencilList.fun("getStencilId", &LuaModelStencilList::getStencilId);

	LuaClass<LuaStencilQuad> luaQuadStencil(m_luaState, "QuadStencil");
	luaQuadStencil.ctor<int>();
	luaQuadStencil.fun("getStencilId", &LuaStencilQuad::getStencilId);
	luaQuadStencil.fun("getParentSpatialAnchorId", &LuaStencilQuad::getParentSpatialAnchorId);
	luaQuadStencil.fun("getQuadCenter", &LuaStencilQuad::getQuadCenter);
	luaQuadStencil.fun("getQuadXAxis", &LuaStencilQuad::getQuadXAxis);
	luaQuadStencil.fun("getQuadYAxis", &LuaStencilQuad::getQuadYAxis);
	luaQuadStencil.fun("getQuadNormal", &LuaStencilQuad::getQuadNormal);
	luaQuadStencil.fun("getQuadWidth", &LuaStencilQuad::getQuadWidth);
	luaQuadStencil.fun("getQuadHeight", &LuaStencilQuad::getQuadHeight);
	luaQuadStencil.fun("getIsDoubleSided", &LuaStencilQuad::getIsDoubleSided);
	luaQuadStencil.fun("getIsDisabled", &LuaStencilQuad::getIsDisabled);
	luaQuadStencil.fun("setQuadCenter", &LuaStencilQuad::setQuadCenter);
	luaQuadStencil.fun("setQuadXAxis", &LuaStencilQuad::setQuadXAxis);
	luaQuadStencil.fun("setQuadYAxis", &LuaStencilQuad::setQuadYAxis);
	luaQuadStencil.fun("setQuadNormal", &LuaStencilQuad::setQuadNormal);
	luaQuadStencil.fun("setQuadWidth", &LuaStencilQuad::setQuadWidth);
	luaQuadStencil.fun("setQuadHeight", &LuaStencilQuad::setQuadHeight);
	luaQuadStencil.fun("setIsDoubleSided", &LuaStencilQuad::setIsDoubleSided);
	luaQuadStencil.fun("setIsDisabled", &LuaStencilQuad::setIsDisabled);

	LuaClass<LuaStencilModel> luaModelStencil(m_luaState, "ModelStencil");
	luaModelStencil.ctor<int>();
	luaModelStencil.fun("getStencilId", &LuaStencilModel::getStencilId);
	luaModelStencil.fun("getParentSpatialAnchorId", &LuaStencilModel::getParentSpatialAnchorId);
	luaModelStencil.fun("getModelPosition", &LuaStencilModel::getModelPosition);
	luaModelStencil.fun("getModelRotator", &LuaStencilModel::getModelRotator);
	luaModelStencil.fun("getModelScale", &LuaStencilModel::getModelScale);
	luaModelStencil.fun("getIsDisabled", &LuaStencilModel::getIsDisabled);
	luaModelStencil.fun("setModelPosition", &LuaStencilModel::setModelPosition);
	luaModelStencil.fun("setModelRotator", &LuaStencilModel::setModelRotator);
	luaModelStencil.fun("setModelScale", &LuaStencilModel::setModelScale);
	luaModelStencil.fun("setIsDisabled", &LuaStencilModel::setIsDisabled);
}