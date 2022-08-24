#include "GlCommon.h"
#include "GlVertexDefinition.h"
#include "StringUtils.h"

GlVertexAttribute::GlVertexAttribute(uint32_t _index, eVertexSemantic _semantic, bool _normalized, int32_t _stride, size_t _offset)
{
	semantic= _semantic;
	switch (semantic)
	{
	case eVertexSemantic::texel2f:
		size = 2;
		type = GL_FLOAT;
		break;
	case eVertexSemantic::position3f:
	case eVertexSemantic::normal3f:
	case eVertexSemantic::color3f:
		size = 3;
		type = GL_FLOAT;
		break;
	case eVertexSemantic::colorAndSize4f:
		size = 4;
		type = GL_FLOAT;
		break;
	default:
		size = 0;
		type = 0;
	}

	index = _index;
	normalized = _normalized;
	stride = _stride;
	offset = _offset;

	char szDesc[256];
	StringUtils::formatString(
		szDesc, sizeof(szDesc),
		"[%d_%d_%d_%c_%d_%d]",
		index, size, type, normalized ? 'n' : 'u', stride, offset);
	desc= szDesc;
}

void GlVertexDefinition::applyVertexDefintion() const
{
	// Identify the components in the vertex buffer
	for (uint32_t attribIndex = 0; attribIndex < attributes.size(); ++attribIndex)
	{
		GlVertexAttribute attrib = attributes[attribIndex];

		glEnableVertexAttribArray(attribIndex);
		glVertexAttribPointer(attrib.index, attrib.size, attrib.type, attrib.normalized, attrib.stride, (GLvoid*)attrib.offset);
	}
}

std::string GlVertexDefinition::getVertexDefinitionDesc() const
{
	std::string desc;
	for (const GlVertexAttribute& attrib : attributes)
	{
		desc+= attrib.desc;
	}

	return desc;
}