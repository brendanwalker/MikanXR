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

// Don't forget to update GlProgram::getUniformSemanticDataType if chaning this enum
enum class eUniformSemantic : int
{
	INVALID = -1,

	transformMatrix,
	modelMatrix,
	normalMatrix, // inverse transpose of the model matrix
	viewMatrix,
	projectionMatrix,
	modelViewProjectionMatrix,
	diffuseColorRGBA,
	cameraPosition,
	ambientColorRGB,
	diffuseColorRGB,
	specularColorRGB,
	lightColorRGB,
	lightDirection,
	screenPosition,
	specularHighlights,
	opticalDensity,
	dissolve,
	floatConstant0,
	floatConstant1,
	floatConstant2,
	floatConstant3,
	ambientTexture,
	diffuseTexture,
	specularTexture,
	specularHightlightTexture,
	alphaTexture,
	bumpTexture,
	rgbTexture, // 24-bit RGB texture
	rgbaTexture, // 32-bit RGBA texture
	distortionTexture, // vec2f texture applying lens undistortion
	depthTexture, // float texture with depth values

	COUNT
};
extern const std::string* k_UniformSemanticName;

// Uniform binding callback
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