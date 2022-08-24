#include "MathUtility.h"
#include "LuaMath.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#endif
#include "luaaa.hpp"
#ifdef _MSC_VER
#pragma warning (pop)
#endif

using namespace luaaa;

// -- LuaVec3f -----
LuaVec3f::LuaVec3f()
{
	x = 0.f;
	y = 0.f;
	z = 0.f;
}

LuaVec3f::LuaVec3f(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

LuaVec3f::LuaVec3f(const MikanVector3f& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

LuaVec3f::LuaVec3f(const MikanRotator3f& r)
{
	x = r.x_angle;
	y = r.y_angle;
	z = r.z_angle;
}

LuaVec3f LuaVec3f::luaStackGet(lua_State* L, int idx)
{
	//LuaVec3f result;

	//luaL_argcheck(L, lua_istable(L, idx), 1, "required table not found on stack.");
	//if (lua_istable(L, idx))
	//{
	//	result.x = LuaStack<float>::get(L, idx + 1);
	//	result.y = LuaStack<float>::get(L, idx + 2);
	//	result.z = LuaStack<float>::get(L, idx + 3);
	//}

	//return result;
	auto dict = LuaStack<std::map<std::string, float>>::get(L, idx);
	return LuaVec3f(dict.find("x")->second, dict.find("y")->second, dict.find("z")->second);
}

void LuaVec3f::luaStackPut(lua_State* L, const LuaVec3f& v)
{
	//lua_newtable(L);
	//LuaStack<float>::put(L, v.x);
	//lua_rawseti(L, -2, 1);
	//LuaStack<float>::put(L, v.y);
	//lua_rawseti(L, -2, 2);
	//LuaStack<float>::put(L, v.z);
	//lua_rawseti(L, -2, 3);
	std::map<std::string, float> dict;
	dict["x"] = v.x;
	dict["y"] = v.y;
	dict["z"] = v.z;
	LuaStack<decltype(dict)>::put(L, dict);
}

MikanVector3f LuaVec3f::toMikanVector3f() const
{
	return { x, y, z };
}

MikanRotator3f LuaVec3f::toMikanRotator3f() const
{
	return { x, y, z };
}

LuaVec3f LuaVec3f::add(const LuaVec3f& a, const LuaVec3f& b)
{
	return LuaVec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

LuaVec3f LuaVec3f::sub(const LuaVec3f& a, const LuaVec3f& b)
{
	return LuaVec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}

LuaVec3f LuaVec3f::scaleUniform(const LuaVec3f& v, float s)
{
	return LuaVec3f(v.x * s, v.y * s, v.z * s);
}

LuaVec3f LuaVec3f::scaleNonUniform(const LuaVec3f& v, const LuaVec3f& s)
{
	return LuaVec3f(v.x * s.x, v.y * s.y, v.z * s.z);
}

float LuaVec3f::dot(const LuaVec3f& a, const LuaVec3f& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

LuaVec3f LuaVec3f::cross(const LuaVec3f& a, const LuaVec3f& b)
{
	return LuaVec3f(
		a.y * b.z - b.y * a.z,
		a.z * b.x - b.z * a.x,
		a.x * b.y - b.x * a.y);
}

float LuaVec3f::length(const LuaVec3f& v)
{
	return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

LuaVec3f LuaVec3f::normalize(const LuaVec3f& v)
{
	float N = length(v);
	return N > k_normal_epsilon ? LuaVec3f(v.x / N, v.y / N, v.z / N) : LuaVec3f();
}

void LuaVec3f::bindFunctions(lua_State* L)
{
	LuaClass<LuaVec3f> luaVec3f(L, "Vec3f");
	luaVec3f.ctor("zero");
	luaVec3f.ctor<float, float, float>();

	LuaModule luaVec3fMath(L, "Vec3fMath");
	luaVec3fMath.fun("add", &LuaVec3f::add);
	luaVec3fMath.fun("sub", &LuaVec3f::sub);
	luaVec3fMath.fun("scaleUniform", &LuaVec3f::scaleUniform);
	luaVec3fMath.fun("scaleNonUniform", &LuaVec3f::scaleNonUniform);
	luaVec3fMath.fun("dot", &LuaVec3f::dot);
	luaVec3fMath.fun("cross", &LuaVec3f::cross);
	luaVec3fMath.fun("length", &LuaVec3f::length);
	luaVec3fMath.fun("normalize", &LuaVec3f::normalize);
}