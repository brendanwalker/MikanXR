#pragma once

#include "MkShaderConstants.h"

#include <array>
#include <string>

// The value types a material graph link can carry. Wildcard pins take their
// type from whatever connects to them and are resolved by the compiler.
enum class eShaderValueType : int
{
	INVALID= -1,

	wildcard,
	float1,
	float2,
	float3,
	float4,
	texture2D,

	COUNT
};

using ShaderValueDefault= std::array<float, 4>;

namespace ShaderValueTypeUtils
{
const std::string& toString(eShaderValueType type);
eShaderValueType fromString(const std::string& name);

// Component count of a float vector type, 0 for wildcard and texture
int getComponentCount(eShaderValueType type);
bool isFloatVector(eShaderValueType type);
eShaderValueType makeFloatVector(int componentCount);

// True when a value of one type can feed a pin of another: equal types, or a
// float1 broadcast into any float vector. Wildcard on either side is allowed.
bool canConvert(eShaderValueType from, eShaderValueType to);

// The type that results from combining two float vector operands: the wider
// vector when one side is a float1, the shared type when equal, INVALID otherwise
eShaderValueType promote(eShaderValueType a, eShaderValueType b);

// The uniform data type and the generic parameter semantic a value of this type
// is exposed through when it becomes a material parameter
eUniformDataType toUniformDataType(eShaderValueType type);
eUniformSemantic getParameterSemantic(eShaderValueType type);
eShaderValueType fromUniformDataType(eUniformDataType dataType);
} // namespace ShaderValueTypeUtils
