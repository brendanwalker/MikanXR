#include "MkVertexConstants.h"
#include "StringUtils.h"

const std::string g_VertexSemanticNames[(int)eVertexSemantic::COUNT] = {
	"generic",
	"position",
	"normal",
	"texCoord",
	"color",
	"colorAndSize"
};
const std::string* k_VertexSemanticNames = g_VertexSemanticNames;

const std::string g_VertexDataTypeNames[(int)eVertexDataType::COUNT] = {
	"ubyte",
	"ubvec2",
	"ubvec3",
	"ubvec4",
	"int",
	"ivec2",
	"ivec3",
	"ivec4",
	"uint",
	"uvec2",
	"uvec3",
	"uvec4",
	"float",
	"vec2",
	"vec3",
	"vec4",
	"double",
	"dvec2",
	"dvec3",
	"dvec4",
};
const std::string* k_VertexDataTypeNames = g_VertexDataTypeNames;

namespace VertexConstantUtils
{
	const std::string& vertexSemanticToString(eVertexSemantic semantic)
	{
		static const std::string invalidSemantic = "INVALID";

		return
			semantic != eVertexSemantic::INVALID
			? g_VertexSemanticNames[(int)semantic]
			: invalidSemantic;
	}

	eVertexSemantic vertexSemanticFromString(const std::string& semanticName)
	{
		return StringUtils::FindEnumValue<eVertexSemantic>(semanticName, g_VertexSemanticNames);
	}

	const std::string& vertexDataTypeToString(eVertexDataType dataType)
	{
		static const std::string invalidDataType = "INVALID";

		return
			dataType != eVertexDataType::INVALID
			? g_VertexDataTypeNames[(int)dataType]
			: invalidDataType;
	}

	eVertexDataType vertexDataTypeFromString(const std::string& dataTypeName)
	{
		return StringUtils::FindEnumValue<eVertexDataType>(dataTypeName, g_VertexDataTypeNames);
	}
};