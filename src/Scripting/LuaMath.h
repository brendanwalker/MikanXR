#pragma once

#include "MikanMathTypes.h"

//-- predeclarations -----
struct lua_State;
typedef struct lua_State lua_State;

namespace luaaa {
	template <typename T> struct LuaStack;
}

//-- definitions -----
class LuaVec3f : public MikanVector3f
{
public:
	LuaVec3f();
	LuaVec3f(float _x, float _y, float _z);
	LuaVec3f(const MikanVector3f& v);
	LuaVec3f(const MikanRotator3f& r);

	static LuaVec3f luaStackGet(lua_State* L, int idx);
	static void luaStackPut(lua_State* L, const LuaVec3f& v);

	MikanVector3f toMikanVector3f() const;
	MikanRotator3f toMikanRotator3f() const;

	static LuaVec3f add(const LuaVec3f& a, const LuaVec3f& b);
	static LuaVec3f sub(const LuaVec3f& a, const LuaVec3f& b);
	static LuaVec3f scaleUniform(const LuaVec3f& v, float s);
	static LuaVec3f scaleNonUniform(const LuaVec3f& v, const LuaVec3f& s);
	static float dot(const LuaVec3f& a, const LuaVec3f& b);
	static LuaVec3f cross(const LuaVec3f& a, const LuaVec3f& b);
	static float length(const LuaVec3f& v);
	static LuaVec3f normalize(const LuaVec3f& v);

	static void bindFunctions(lua_State* L);
};

namespace luaaa {
	template<> struct LuaStack<LuaVec3f>
	{
		inline static LuaVec3f get(lua_State* L, int idx)
		{
			return LuaVec3f::luaStackGet(L, idx);
		}

		inline static void put(lua_State* L, const LuaVec3f& v)
		{
			LuaVec3f::luaStackPut(L, v);
		}
	};
}
