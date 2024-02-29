#pragma once

#include <functional>
#include <string>
#include <memory>

enum class eUniformDataType : int
{
	INVALID = -1,

	datatype_int,
	datatype_int2,
	datatype_int3,
	datatype_int4,
	datatype_float,
	datatype_float2,
	datatype_float3,
	datatype_float4,
	datatype_mat4,
	datatype_texture,
};

enum class eUniformSemantic : int
{
	INVALID = -1,

	transformMatrix,
	modelMatrix,
	normalMatrix, // inverse transpose of the model matrix
	viewMatrix,
	projectionMatrix,
	modelViewProjectionMatrix,
	cameraPosition,
	ambientColorRGBA,
	diffuseColorRGBA,
	diffuseColorRGB,
	specularColorRGBA,
	lightColor,
	lightDirection,
	screenPosition,
	shininess,
	floatConstant0,
	floatConstant1,
	floatConstant2,
	floatConstant3,
	texture0,
	texture1,
	texture2,
	texture3,
	texture4,
	texture5,
	texture6,
	texture7,
	texture8,
	texture9,
	texture10,
	texture11,
	texture12,
	texture13,
	texture14,
	texture15,
	texture16,
	texture17,
	texture18,
	texture19,
	texture20,
	texture21,
	texture22,
	texture23,
	texture24,
	texture25,
	texture26,
	texture27,
	texture28,
	texture29,
	texture30,
	texture31,

	COUNT
};
extern const std::string* k_UniformSemanticName;

enum class eUniformBindResult : int
{
	bound,
	unbound,
	error
};

using BindUniformCallback =
	std::function<eUniformBindResult(
		std::shared_ptr<class GlProgram>, // Source program to bind the uniform for
		eUniformDataType, // Data type of the uniform
		eUniformSemantic, // Semantic of the uniform
		const std::string&)>; // Name of the uniform