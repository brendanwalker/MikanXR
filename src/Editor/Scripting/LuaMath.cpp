#include "MathUtility.h"
#include "MathTypeConversion.h"
#include "LuaMath.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- LuaVec3f -----
LuaVec3f::LuaVec3f()
{
	x= 0.f;
	y= 0.f;
	z= 0.f;
}

LuaVec3f::LuaVec3f(float _x, float _y, float _z)
{
	x= _x;
	y= _y;
	z= _z;
}

LuaVec3f::LuaVec3f(const glm::vec3& v)
{
	x= v.x;
	y= v.y;
	z= v.z;
}

LuaVec3f::LuaVec3f(const MikanVector3f& v)
{
	x= v.x;
	y= v.y;
	z= v.z;
}

LuaVec3f::LuaVec3f(const MikanRotator3f& r)
{
	x= r.x_angle;
	y= r.y_angle;
	z= r.z_angle;
}

MikanVector3f LuaVec3f::toMikanVector3f() const { return {x, y, z}; }

MikanRotator3f LuaVec3f::toMikanRotator3f() const { return {x, y, z}; }

glm::vec3 LuaVec3f::toGlmVec3f() const { return {x, y, z}; }

LuaVec3f LuaVec3f::operator+(const LuaVec3f& v) const { return LuaVec3f(x + v.x, y + v.y, z + v.z); }

LuaVec3f LuaVec3f::operator-(const LuaVec3f& v) const { return LuaVec3f(x - v.x, y - v.y, z - v.z); }

LuaVec3f LuaVec3f::scaleUniform(float s) { return LuaVec3f(x * s, y * s, z * s); }

LuaVec3f LuaVec3f::scaleNonUniform(const LuaVec3f& s) { return LuaVec3f(x * s.x, y * s.y, z * s.z); }

float LuaVec3f::dot(const LuaVec3f& a, const LuaVec3f& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

LuaVec3f LuaVec3f::cross(const LuaVec3f& a, const LuaVec3f& b)
{
	return LuaVec3f(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y);
}

float LuaVec3f::length() const { return sqrtf(x * x + y * y + z * z); }

LuaVec3f LuaVec3f::normalize()
{
	float N= length();
	return N > k_normal_epsilon ? LuaVec3f(x / N, y / N, z / N) : LuaVec3f();
}

std::string LuaVec3f::toString() const
{
	std::ostringstream os;

	os << "(" << x << ", " << y << ", " << z << ")";

	return os.str();
}

void LuaVec3f::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaVec3f>("Vec3f")
		.addConstructor<void (*)(float, float, float)>()
		.addProperty(
			"x", +[](const LuaVec3f* vec) { return vec->x; }, +[](LuaVec3f* vec, float v) { vec->x= v; })
		.addProperty(
			"y", +[](const LuaVec3f* vec) { return vec->y; }, +[](LuaVec3f* vec, float v) { vec->y= v; })
		.addProperty(
			"z", +[](const LuaVec3f* vec) { return vec->z; }, +[](LuaVec3f* vec, float v) { vec->z= v; })
		.addFunction("dot", &LuaVec3f::dot)
		.addFunction("__add", &LuaVec3f::operator+)
		.addFunction("__sub", &LuaVec3f::operator-)
		.addFunction("scaleUniform", &LuaVec3f::scaleUniform)
		.addFunction("scaleNonUniform", &LuaVec3f::scaleNonUniform)
		.addStaticFunction("dot", &LuaVec3f::dot)
		.addStaticFunction("cross", &LuaVec3f::cross)
		.addFunction("length", &LuaVec3f::length)
		.addFunction("normalize", &LuaVec3f::normalize)
		.addFunction("__tostring", &LuaVec3f::toString)
		.endClass();
}

// -- LuaQuatf -----
LuaQuatf::LuaQuatf()
{
	w= 1.f;
	x= 0.f;
	y= 0.f;
	z= 0.f;
}

LuaQuatf::LuaQuatf(float _w, float _x, float _y, float _z)
{
	w= _w;
	x= _x;
	y= _y;
	z= _z;
}

LuaQuatf::LuaQuatf(const glm::quat& q)
{
	w= q.w;
	x= q.x;
	y= q.y;
	z= q.z;
}

LuaQuatf::LuaQuatf(const MikanQuatf& q)
{
	w= q.w;
	x= q.x;
	y= q.y;
	z= q.z;
}

MikanQuatf LuaQuatf::toMikanQuatf() const { return {w, x, y, z}; }

glm::quat LuaQuatf::toGlmQuat() const { return glm::quat(w, x, y, z); }

LuaQuatf LuaQuatf::operator*(const LuaQuatf& q) const { return LuaQuatf(toGlmQuat() * q.toGlmQuat()); }

LuaVec3f LuaQuatf::rotateVec3f(const LuaVec3f& v) const { return LuaVec3f(toGlmQuat() * v.toGlmVec3f()); }

LuaQuatf LuaQuatf::inverse() const { return LuaQuatf(glm::inverse(toGlmQuat())); }

float LuaQuatf::length() const { return glm::length(toGlmQuat()); }

LuaQuatf LuaQuatf::normalize() const { return LuaQuatf(glm::normalize(toGlmQuat())); }

std::string LuaQuatf::toString() const
{
	std::ostringstream os;

	os << "(" << w << ", " << x << ", " << y << ", " << z << ")";

	return os.str();
}

void LuaQuatf::bindFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<LuaQuatf>("Quatf")
		.addConstructor<void (*)(float, float, float, float)>()
		.addProperty(
			"w", +[](const LuaQuatf* q) { return q->w; }, +[](LuaQuatf* q, float v) { q->w= v; })
		.addProperty(
			"x", +[](const LuaQuatf* q) { return q->x; }, +[](LuaQuatf* q, float v) { q->x= v; })
		.addProperty(
			"y", +[](const LuaQuatf* q) { return q->y; }, +[](LuaQuatf* q, float v) { q->y= v; })
		.addProperty(
			"z", +[](const LuaQuatf* q) { return q->z; }, +[](LuaQuatf* q, float v) { q->z= v; })
		.addFunction("__mul", &LuaQuatf::operator*)
		.addFunction("rotateVec3f", &LuaQuatf::rotateVec3f)
		.addFunction("inverse", &LuaQuatf::inverse)
		.addFunction("length", &LuaQuatf::length)
		.addFunction("normalize", &LuaQuatf::normalize)
		.addFunction("__tostring", &LuaQuatf::toString)
		.endClass();
}