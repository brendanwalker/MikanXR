#pragma once

#include "MkRendererExport.h"

#include <string>

enum class eUniformDataType : int
{
	INVALID= -1,

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
	INVALID= -1,

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
	ambientStrength,
	ambientTexture,
	diffuseTexture,
	specularTexture,
	specularHightlightTexture,
	alphaTexture,
	bumpTexture,
	rgbTexture,        // 24-bit RGB texture
	rgbaTexture,       // 32-bit RGBA texture
	distortionTexture, // vec2f texture applying lens undistortion
	depthTexture,      // float texture with depth values

	// Order-2 spherical harmonic environment, one RGB coefficient per slot.
	// Nine separate uniforms rather than an array because the uniform binding
	// layer is one semantic per uniform. Deliberately mirrors the SH0..SH8
	// parameters on the Unreal skydome material so the two evaluations can be
	// compared directly.
	shCoefficient0,
	shCoefficient1,
	shCoefficient2,
	shCoefficient3,
	shCoefficient4,
	shCoefficient5,
	shCoefficient6,
	shCoefficient7,
	shCoefficient8,

	COUNT
};

MIKAN_RENDERER_FUNC(eUniformDataType) getUniformSemanticDataType(eUniformSemantic semantic);
MIKAN_RENDERER_FUNC(std::string) getUniformSemanticName(eUniformSemantic semantic);
