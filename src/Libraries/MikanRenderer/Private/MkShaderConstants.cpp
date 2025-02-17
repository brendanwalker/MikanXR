#include "MkShaderConstants.h"
#include "assert.h"

const std::string g_UniformSemanticName[(int)eUniformSemantic::COUNT] = {
	"transformMatrix",
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
	"specularHighlights",
	"opticalDensity",
	"dissolve",
	"zNear",
	"zFar",
	"floatConstant0",
	"floatConstant1",
	"floatConstant2",
	"floatConstant3",
	"ambientTexture",
	"diffuseTexture",
	"specularTexture",
	"specularHightlightTexture",
	"alphaTexture",
	"bumpTexture",
	"rgbTexture",
	"rgbaTexture",
	"distortionTexture",
	"depthTexture"
};

eUniformDataType getUniformSemanticDataType(eUniformSemantic semantic)
{
	eUniformDataType dataType= eUniformDataType::INVALID;

	static_assert((int)eUniformSemantic::COUNT == 34, "getUniformSemanticDataType out of date with eUniformSemantic");
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
			dataType = eUniformDataType::datatype_float;
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
	int semanticIndex = (int)semantic;

	return 
		(semanticIndex > (int)eUniformSemantic::INVALID && semanticIndex < (int)eUniformSemantic::COUNT)
		? g_UniformSemanticName[semanticIndex]
		: "";
}