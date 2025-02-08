#pragma once

#include "MkRendererExport.h"

#include <string>

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

// Don't forget to update IMkShader::getUniformSemanticDataType if chaning this enum
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
	screenSize,
	specularHighlights,
	opticalDensity,
	dissolve,
	zNear,
	zFar,
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


MIKAN_RENDERER_FUNC(eUniformDataType) getUniformSemanticDataType(eUniformSemantic semantic);
