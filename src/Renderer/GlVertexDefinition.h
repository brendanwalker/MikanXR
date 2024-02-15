#pragma once

#include "stdint.h"
#include <string>
#include <vector>

enum class eVertexSemantic : int
{
	INVALID = -1,

	// Specific Semantic Types
	position2f,
	position3f,
	normal3f,
	color3f,
	color4f,
	color4b,
	colorAndSize4f,
	texel2f,
};

enum class eVertexDataType : int
{
	INVALID = -1,

	// Generic Float Types
	datatype_float,
	datatype_vec2f,
	datatype_vec3f,
	datatype_vec4f,
	datatype_mat2f,
	datatype_mat3f,
	datatype_mat4f,
	datatype_mat2x3f,
	datatype_mat2x4f,
	datatype_mat3x2f,
	datatype_mat3x4f,
	datatype_mat4x2f,
	datatype_mat4x3f,

	// Generic Unsigned Byte Types
	datatype_ubyte,
	datatype_vec2ub,
	datatype_vec3ub,
	datatype_vec4ub,

	// Generic Int Types
	datatype_int,
	datatype_vec2i,
	datatype_vec3i,
	datatype_vec4i,

	// Generic Unsigned Int Types
	datatype_uint,
	datatype_vec2ui,
	datatype_vec3ui,
	datatype_vec4ui,

	// Generic Double Types
	datatype_double,
	datatype_vec2d,
	datatype_vec3d,
	datatype_vec4d,
	datatype_mat2d,
	datatype_mat3d,
	datatype_mat4d,
	datatype_mat2x3d,
	datatype_mat2x4d,
	datatype_mat3x2d,
	datatype_mat3x4d,
	datatype_mat4x2d,
	datatype_mat4x3d
};

struct GlVertexAttribute
{
	uint32_t index;
	int32_t size;
	uint32_t type;
	bool normalized;
	int32_t stride;
	size_t offset;
	eVertexSemantic semantic;
	eVertexDataType dataType;
	std::string desc;

	GlVertexAttribute() = default;
	GlVertexAttribute(uint32_t _index, int32_t _size, uint32_t _type, bool _normalized, int32_t _stride, size_t _offset);
	GlVertexAttribute(uint32_t _index, eVertexSemantic _semantic, bool _normalized, int32_t _stride, size_t _offset);
	GlVertexAttribute(uint32_t _index, eVertexDataType _dataType, bool _normalized, int32_t _stride, size_t _offset);

	void init(uint32_t _index, int32_t _size, uint32_t _type, bool _normalized, int32_t _stride, size_t _offset);
	bool isCompatibleAttribute(const GlVertexAttribute& other) const;
};

struct GlVertexDefinition
{
	GlVertexDefinition() = default;
	GlVertexDefinition(const GlVertexDefinition& vertexDefinition)
	{
		this->attributes= vertexDefinition.attributes;
		this->vertexSize= vertexDefinition.vertexSize;
	}
	GlVertexDefinition(const std::vector<GlVertexAttribute>&_attribtes, uint32_t _vertexSize)
	{
		attributes = _attribtes;
		vertexSize = _vertexSize;
	}

	std::vector<GlVertexAttribute> attributes;
	uint32_t vertexSize;

	void applyVertexDefintion() const;
	std::string getVertexDefinitionDesc() const;
	const GlVertexAttribute* getFirstAttributeBySemantic(eVertexSemantic semantic) const;
	bool isCompatibleDefinition(const GlVertexDefinition& other) const;
};