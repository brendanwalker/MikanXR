#include "MkShaderConstants.h"
#include "assert.h"

const std::string g_UniformSemanticName[(int)eUniformSemantic::COUNT]= {"transformMatrix",
																		"modelMatrix",
																		"inverseModelMatrix",
																		"viewMatrix",
																		"projectionMatrix",
																		"modelViewProjectionMatrix",
																		"diffuseColorRGBA",
																		"cameraPosition",
																		"ambientColorRGB",
																		"diffuseColorRGB",
																		"specularColorRGB",
																		"lightColorRGB",
																		"lightDirection",
																		"screenPosition",
																		"screenSize",
																		"specularHighlights",
																		"opticalDensity",
																		"dissolve",
																		"zNear",
																		"zFar",
																		"floatConstant0",
																		"floatConstant1",
																		"floatConstant2",
																		"floatConstant3",
																		"ambientStrength",
																		"ambientTexture",
																		"diffuseTexture",
																		"specularTexture",
																		"specularHightlightTexture",
																		"alphaTexture",
																		"bumpTexture",
																		"rgbTexture",
																		"rgbaTexture",
																		"distortionTexture",
																		"depthTexture",
																		"shCoefficient0",
																		"shCoefficient1",
																		"shCoefficient2",
																		"shCoefficient3",
																		"shCoefficient4",
																		"shCoefficient5",
																		"shCoefficient6",
																		"shCoefficient7",
																		"shCoefficient8"};

eUniformDataType getUniformSemanticDataType(eUniformSemantic semantic)
{
	eUniformDataType dataType= eUniformDataType::INVALID;

	static_assert((int)eUniformSemantic::COUNT == 44, "getUniformSemanticDataType out of date with eUniformSemantic");
	switch (semantic)
	{
	case eUniformSemantic::transformMatrix:
	case eUniformSemantic::modelMatrix:
	case eUniformSemantic::normalMatrix:
	case eUniformSemantic::viewMatrix:
	case eUniformSemantic::projectionMatrix:
	case eUniformSemantic::modelViewProjectionMatrix:
		dataType= eUniformDataType::datatype_mat4;
		break;
	case eUniformSemantic::diffuseColorRGBA:
		dataType= eUniformDataType::datatype_float4;
		break;
	case eUniformSemantic::lightColorRGB:
	case eUniformSemantic::ambientColorRGB:
	case eUniformSemantic::diffuseColorRGB:
	case eUniformSemantic::specularColorRGB:
	case eUniformSemantic::cameraPosition:
	case eUniformSemantic::lightDirection:
	case eUniformSemantic::shCoefficient0:
	case eUniformSemantic::shCoefficient1:
	case eUniformSemantic::shCoefficient2:
	case eUniformSemantic::shCoefficient3:
	case eUniformSemantic::shCoefficient4:
	case eUniformSemantic::shCoefficient5:
	case eUniformSemantic::shCoefficient6:
	case eUniformSemantic::shCoefficient7:
	case eUniformSemantic::shCoefficient8:
		dataType= eUniformDataType::datatype_float3;
		break;
	case eUniformSemantic::screenPosition:
	case eUniformSemantic::screenSize:
		dataType= eUniformDataType::datatype_float2;
		break;
	case eUniformSemantic::specularHighlights:
	case eUniformSemantic::opticalDensity:
	case eUniformSemantic::dissolve:
	case eUniformSemantic::zNear:
	case eUniformSemantic::zFar:
	case eUniformSemantic::floatConstant0:
	case eUniformSemantic::floatConstant1:
	case eUniformSemantic::floatConstant2:
	case eUniformSemantic::floatConstant3:
	case eUniformSemantic::ambientStrength:
		dataType= eUniformDataType::datatype_float;
		break;
	case eUniformSemantic::ambientTexture:
	case eUniformSemantic::diffuseTexture:
	case eUniformSemantic::specularTexture:
	case eUniformSemantic::specularHightlightTexture:
	case eUniformSemantic::alphaTexture:
	case eUniformSemantic::bumpTexture:
	case eUniformSemantic::rgbTexture:
	case eUniformSemantic::rgbaTexture:
	case eUniformSemantic::distortionTexture:
	case eUniformSemantic::depthTexture:
		dataType= eUniformDataType::datatype_texture;
		break;
	default:
		assert(false);
	}

	return dataType;
}

std::string getUniformSemanticName(eUniformSemantic semantic)
{
	int semanticIndex= (int)semantic;

	return (semanticIndex > (int)eUniformSemantic::INVALID && semanticIndex < (int)eUniformSemantic::COUNT)
			   ? g_UniformSemanticName[semanticIndex]
			   : "";
}