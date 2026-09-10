#include "ShaderValueType.h"

namespace
{
const std::string k_shaderValueTypeNames[(int)eShaderValueType::COUNT]= {"wildcard", "float",  "float2",
																		 "float3",   "float4", "texture2D"};
const std::string k_invalidValueTypeName= "INVALID";
} // namespace

namespace ShaderValueTypeUtils
{
const std::string& toString(eShaderValueType type)
{
	const int index= (int)type;
	return (index > (int)eShaderValueType::INVALID && index < (int)eShaderValueType::COUNT)
			   ? k_shaderValueTypeNames[index]
			   : k_invalidValueTypeName;
}

eShaderValueType fromString(const std::string& name)
{
	for (int index= 0; index < (int)eShaderValueType::COUNT; ++index)
	{
		if (k_shaderValueTypeNames[index] == name)
		{
			return (eShaderValueType)index;
		}
	}

	return eShaderValueType::INVALID;
}

int getComponentCount(eShaderValueType type)
{
	switch (type)
	{
	case eShaderValueType::float1:
		return 1;
	case eShaderValueType::float2:
		return 2;
	case eShaderValueType::float3:
		return 3;
	case eShaderValueType::float4:
		return 4;
	default:
		return 0;
	}
}

bool isFloatVector(eShaderValueType type) { return getComponentCount(type) > 0; }

eShaderValueType makeFloatVector(int componentCount)
{
	switch (componentCount)
	{
	case 1:
		return eShaderValueType::float1;
	case 2:
		return eShaderValueType::float2;
	case 3:
		return eShaderValueType::float3;
	case 4:
		return eShaderValueType::float4;
	default:
		return eShaderValueType::INVALID;
	}
}

bool canConvert(eShaderValueType from, eShaderValueType to)
{
	if (from == eShaderValueType::INVALID || to == eShaderValueType::INVALID)
		return false;

	if (from == eShaderValueType::wildcard || to == eShaderValueType::wildcard)
		return true;

	if (from == to)
		return true;

	return from == eShaderValueType::float1 && isFloatVector(to);
}

eShaderValueType promote(eShaderValueType a, eShaderValueType b)
{
	if (!isFloatVector(a) || !isFloatVector(b))
		return eShaderValueType::INVALID;

	if (a == b)
		return a;

	if (a == eShaderValueType::float1)
		return b;

	if (b == eShaderValueType::float1)
		return a;

	return eShaderValueType::INVALID;
}

eUniformDataType toUniformDataType(eShaderValueType type)
{
	switch (type)
	{
	case eShaderValueType::float1:
		return eUniformDataType::datatype_float;
	case eShaderValueType::float2:
		return eUniformDataType::datatype_float2;
	case eShaderValueType::float3:
		return eUniformDataType::datatype_float3;
	case eShaderValueType::float4:
		return eUniformDataType::datatype_float4;
	case eShaderValueType::texture2D:
		return eUniformDataType::datatype_texture;
	default:
		return eUniformDataType::INVALID;
	}
}

eUniformSemantic getParameterSemantic(eShaderValueType type)
{
	switch (type)
	{
	case eShaderValueType::float1:
		return eUniformSemantic::floatParam;
	case eShaderValueType::float2:
		return eUniformSemantic::float2Param;
	case eShaderValueType::float3:
		return eUniformSemantic::float3Param;
	case eShaderValueType::float4:
		return eUniformSemantic::float4Param;
	case eShaderValueType::texture2D:
		return eUniformSemantic::textureParam;
	default:
		return eUniformSemantic::INVALID;
	}
}

eShaderValueType fromUniformDataType(eUniformDataType dataType)
{
	switch (dataType)
	{
	case eUniformDataType::datatype_float:
		return eShaderValueType::float1;
	case eUniformDataType::datatype_float2:
		return eShaderValueType::float2;
	case eUniformDataType::datatype_float3:
		return eShaderValueType::float3;
	case eUniformDataType::datatype_float4:
		return eShaderValueType::float4;
	case eUniformDataType::datatype_texture:
		return eShaderValueType::texture2D;
	default:
		return eShaderValueType::INVALID;
	}
}
} // namespace ShaderValueTypeUtils
