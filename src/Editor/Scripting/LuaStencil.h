#pragma once

#include "ComponentFwd.h"
#include "MikanAPITypes.h"
#include "LuaMath.h"

#include <vector>

//-- predeclarations -----
struct lua_State;
typedef struct lua_State lua_State;

//-- definitions -----
class LuaStencilList
{
public:
	LuaStencilList()
	{ }

	int getStencilCount() const { return (int)m_stencilIdList.size(); }
	int getStencilId(int listIndex) const { 
		return (listIndex >= 0 && listIndex < getStencilCount()) ? m_stencilIdList[listIndex] : INVALID_MIKAN_ID; 
	}

protected:
	std::vector<int> m_stencilIdList;
};


class LuaQuadStencilList : public LuaStencilList
{
public:
	LuaQuadStencilList();

	static void bindFunctions(lua_State* L);
};

class LuaBoxStencilList : public LuaStencilList
{
public:
	LuaBoxStencilList();

	static void bindFunctions(lua_State* L);
};

class LuaModelStencilList : public LuaStencilList
{
public:
	LuaModelStencilList();

	static void bindFunctions(lua_State* L);
};

class LuaStencilQuad
{
public:
	LuaStencilQuad(int stencilId);

	int getStencilId() const;
	int getParentSpatialAnchorId() const;

	LuaVec3f getQuadCenter() const;
	float getQuadWidth() const;
	float getQuadHeight() const;
	bool getIsDoubleSided() const;
	bool getIsDisabled() const;

	bool setQuadCenter(const LuaVec3f& center);
	bool setQuadWidth(float width);
	bool setQuadHeight(float height);
	bool setIsDoubleSided(bool flag);
	bool setIsDisabled(bool flag);

	static void bindFunctions(lua_State* L);

private:
	QuadStencilComponentConstPtr findConstStencilByID(int stencilId) const;
	QuadStencilComponentPtr findStencilByID(int stencilId);

	MikanStencilID m_stencilId; // filled in on allocation
};

class LuaStencilBox
{
public:
	LuaStencilBox(int stencilId);

	int getStencilId() const;
	int getParentSpatialAnchorId() const;
	LuaVec3f getBoxCenter() const;
	float getBoxXSize() const;
	float getBoxYSize() const;
	float getBoxZSize() const;
	bool getIsDisabled() const;

	bool setBoxCenter(const LuaVec3f& center);
	bool setBoxXSize(float size);
	bool setBoxYSize(float size);
	bool setBoxZSize(float size);
	bool setIsDisabled(bool flag);

	static void bindFunctions(lua_State* L);

private:
	BoxStencilComponentConstPtr findConstStencilByID(int stencilId) const;
	BoxStencilComponentPtr findStencilByID(int stencilId);

	MikanStencilID m_stencilId; // filled in on allocation
};

class LuaStencilModel
{
public:
	LuaStencilModel(int stencilId);

	int getStencilId() const;
	int getParentSpatialAnchorId() const;
	LuaVec3f getModelPosition() const;
	LuaVec3f getModelScale() const;
	bool getIsDisabled() const;

	bool setModelPosition(const LuaVec3f& position);
	bool setModelScale(const LuaVec3f& scale);
	bool setIsDisabled(bool flag);

	static void bindFunctions(lua_State* L);

private:
	ModelStencilComponentConstPtr findConstStencilByID(int stencilId) const;
	ModelStencilComponentPtr findStencilByID(int stencilId);

	MikanStencilID m_stencilId; // filled in on allocation
};