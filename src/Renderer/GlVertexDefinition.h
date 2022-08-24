#pragma once

#include "stdint.h"
#include <string>
#include <vector>

enum class eVertexSemantic : int
{
	position3f,
	normal3f,
	color3f,
	colorAndSize4f,
	texel2f
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
	std::string desc;

	GlVertexAttribute() = default;
	GlVertexAttribute(uint32_t _index, eVertexSemantic _semantic, bool _normalized, int32_t _stride, size_t _offset);
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
};