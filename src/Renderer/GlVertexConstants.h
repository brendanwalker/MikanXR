#pragma once

#include <string>

enum class eVertexSemantic : int
{
	INVALID = -1,

	// Specific Semantic Types
	generic,
	position,
	normal,
	texCoord,
	color,
	colorAndSize,

	COUNT
};
extern const std::string* k_VertexSemanticNames;

enum class eVertexDataType : int
{
	INVALID = -1,

	// Generic Unsigned Byte Types
	datatype_ubyte,
	datatype_ubvec2,
	datatype_ubvec3,
	datatype_ubvec4,

	// Generic Int Types
	datatype_int,
	datatype_ivec2,
	datatype_ivec3,
	datatype_ivec4,

	// Generic Unsigned Int Types
	datatype_uint,
	datatype_uvec2,
	datatype_uvec3,
	datatype_uvec4,

	// Generic Float Types
	datatype_float,
	datatype_vec2,
	datatype_vec3,
	datatype_vec4,

	// Generic Double Types
	datatype_double,
	datatype_dvec2,
	datatype_dvec3,
	datatype_dvec4,

	COUNT
};
extern const std::string* k_VertexDataTypeNames;

namespace VertexConstantUtils
{
	const std::string& vertexSemanticToString(eVertexSemantic semantic);
	eVertexSemantic vertexSemanticFromString(const std::string& semanticName);

	const std::string& vertexDataTypeToString(eVertexDataType dataType);
	eVertexDataType vertexDataTypeFromString(const std::string& dataTypeName);
};