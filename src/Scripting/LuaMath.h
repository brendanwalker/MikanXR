#pragma once

#include "MikanMathTypes.h"
#include <string>
#include "glm/ext/vector_float3.hpp"

//-- predeclarations -----
struct lua_State;
typedef struct lua_State lua_State;

//-- definitions -----
class LuaVec3f : public MikanVector3f
{
public:
	LuaVec3f();
	LuaVec3f(float _x, float _y, float _z);
	LuaVec3f(const glm::vec3& v);
	LuaVec3f(const MikanVector3f& v);
	LuaVec3f(const MikanRotator3f& r);

	MikanVector3f toMikanVector3f() const;
	MikanRotator3f toMikanRotator3f() const;
	glm::vec3 toGlmVec3f() const;

	LuaVec3f operator + (const LuaVec3f& v) const;
	LuaVec3f operator - (const LuaVec3f& v) const;
	LuaVec3f scaleUniform(float s);
	LuaVec3f scaleNonUniform(const LuaVec3f& s);
	static float dot(const LuaVec3f& a, const LuaVec3f& b);
	static LuaVec3f cross(const LuaVec3f& a, const LuaVec3f& b);
	float length() const;
	LuaVec3f normalize();
	std::string toString() const;

	static void bindFunctions(lua_State* L);
};