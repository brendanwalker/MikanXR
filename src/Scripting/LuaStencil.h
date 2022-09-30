#pragma once

#include "MikanClientTypes.h"
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
	int getStencilId(int listIndex) const { return m_stencilIdList[listIndex]; }

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
	LuaVec3f getQuadXAxis() const;
	LuaVec3f getQuadYAxis() const;
	LuaVec3f getQuadNormal() const;
	float getQuadWidth() const;
	float getQuadHeight() const;
	bool getIsDoubleSided() const;
	bool getIsDisabled() const;

	bool setQuadCenter(const LuaVec3f& center);
	bool setQuadXAxis(const LuaVec3f& x_axis);
	bool setQuadYAxis(const LuaVec3f& y_axis);
	bool setQuadNormal(const LuaVec3f& normal);
	bool setQuadWidth(float width);
	bool setQuadHeight(float height);
	bool setIsDoubleSided(bool flag);
	bool setIsDisabled(bool flag);

	static void bindFunctions(lua_State* L);

private:
	const MikanStencilQuad* findConstStencilByID(int stencilId) const;
	MikanStencilQuad* findStencilByID(int stencilId);

	MikanStencilID m_stencilId; // filled in on allocation
};

class LuaStencilBox
{
public:
	LuaStencilBox(int stencilId);

	int getStencilId() const;
	int getParentSpatialAnchorId() const;
	LuaVec3f getBoxCenter() const;
	LuaVec3f getBoxXAxis() const;
	LuaVec3f getBoxYAxis() const;
	LuaVec3f getBoxZAxis() const;
	float getBoxXSize() const;
	float getBoxYSize() const;
	float getBoxZSize() const;
	bool getIsDisabled() const;

	bool setBoxCenter(const LuaVec3f& center);
	bool setBoxXAxis(const LuaVec3f& x_axis);
	bool setBoxYAxis(const LuaVec3f& y_axis);
	bool setBoxZAxis(const LuaVec3f& normal);
	bool setBoxXSize(float size);
	bool setBoxYSize(float size);
	bool setBoxZSize(float size);
	bool setIsDisabled(bool flag);

	static void bindFunctions(lua_State* L);

private:
	const MikanStencilBox* findConstStencilByID(int stencilId) const;
	MikanStencilBox* findStencilByID(int stencilId);

	MikanStencilID m_stencilId; // filled in on allocation
};

class LuaStencilModel
{
public:
	LuaStencilModel(int stencilId);

	int getStencilId() const;
	int getParentSpatialAnchorId() const;
	LuaVec3f getModelPosition() const;
	LuaVec3f getModelRotator() const;
	LuaVec3f getModelScale() const;
	bool getIsDisabled() const;

	bool setModelPosition(const LuaVec3f& position);
	bool setModelRotator(const LuaVec3f& rotator);
	bool setModelScale(const LuaVec3f& scale);
	bool setIsDisabled(bool flag);

	static void bindFunctions(lua_State* L);

private:
	const struct MikanStencilModelConfig* findConstStencilByID(int stencilId) const;
	struct MikanStencilModelConfig* findStencilByID(int stencilId);

	MikanStencilID m_stencilId; // filled in on allocation
};