#include "GlCommon.h"
#include "GlVertexDefinition.h"

GlVertexAttribute::GlVertexAttribute(uint32_t _index, int32_t _size, uint32_t _type, bool _normalized, int32_t _stride, size_t _offset)
{
	init(_index, _size, _type, _normalized, _stride, _offset);
}

GlVertexAttribute::GlVertexAttribute(uint32_t _index, eVertexSemantic _semantic, bool _normalized, int32_t _stride, size_t _offset)
{
	int32_t _size= 0;
	uint32_t _type= 0;

	switch (_semantic)
	{
	case eVertexSemantic::position2f:
	case eVertexSemantic::texel2f:
		_size = 2;
		_type = GL_FLOAT;
		break;
	case eVertexSemantic::position3f:
	case eVertexSemantic::normal3f:
	case eVertexSemantic::color3f:
		_size = 3;
		_type = GL_FLOAT;
		break;
	case eVertexSemantic::color4f:
	case eVertexSemantic::colorAndSize4f:
		_size = 4;
		_type = GL_FLOAT;
		break;
	case eVertexSemantic::color4b:
		_size = 4;
		_type = GL_UNSIGNED_BYTE;
		break;
	}

	init(_index, _size, _type, _normalized, _stride, _offset);
	semantic = _semantic;
}

GlVertexAttribute::GlVertexAttribute(uint32_t _index, eVertexDataType _dataType, bool _normalized, int32_t _stride, size_t _offset)
{
	int32_t _size = 0;
	uint32_t _type = 0;

	switch (_dataType)
	{
		// Generic Float Types
		case eVertexDataType::datatype_float:
			_size = 1;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_vec2f:
			_size = 2;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_vec3f:
			_size = 3;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_vec4f:
			_size = 4;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat2f:
			_size = 2*2;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat3f:
			_size = 3*3;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat4f:
			_size = 4*4;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat2x3f:
			_size = 2*3;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat2x4f:
			_size = 2*4;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat3x2f:
			_size = 3*2;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat3x4f:
			_size = 3*4;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat4x2f:
			_size = 4*2;
			_type = GL_FLOAT;
			break;
		case eVertexDataType::datatype_mat4x3f:
			_size = 4*3;
			_type = GL_FLOAT;
			break;

		// Generic Unsigned Byte Types
		case eVertexDataType::datatype_ubyte:
			_size = 1;
			_type = GL_UNSIGNED_BYTE;
			break;
		case eVertexDataType::datatype_vec2ub:
			_size = 2;
			_type = GL_UNSIGNED_BYTE;
			break;
		case eVertexDataType::datatype_vec3ub:
			_size = 3;
			_type = GL_UNSIGNED_BYTE;
			break;
		case eVertexDataType::datatype_vec4ub:
			_size = 4;
			_type = GL_UNSIGNED_BYTE;
			break;

		// Generic Int Types
		case eVertexDataType::datatype_int:
			_size = 1;
			_type = GL_INT;
			break;
		case eVertexDataType::datatype_vec2i:
			_size = 2;
			_type = GL_INT;
			break;
		case eVertexDataType::datatype_vec3i:
			_size = 3;
			_type = GL_INT;
			break;
		case eVertexDataType::datatype_vec4i:
			_size = 4;
			_type = GL_INT;
			break;

		// Generic Unsigned Int Types
		case eVertexDataType::datatype_uint:
			_size = 1;
			_type = GL_UNSIGNED_INT;
			break;
		case eVertexDataType::datatype_vec2ui:
			_size = 2;
			_type = GL_UNSIGNED_INT;
			break;
		case eVertexDataType::datatype_vec3ui:
			_size = 3;
			_type = GL_UNSIGNED_INT;
			break;
		case eVertexDataType::datatype_vec4ui:
			_size = 4;
			_type = GL_UNSIGNED_INT;
			break;

		// Generic Double Types
		case eVertexDataType::datatype_double:
			_size = 1;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_vec2d:
			_size = 2;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_vec3d:
			_size = 3;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_vec4d:
			_size = 4;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat2d:
			_size = 2*2;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat3d:
			_size = 3*3;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat4d:
			_size = 4*4;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat2x3d:
			_size = 2*3;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat2x4d:
			_size = 2*4;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat3x2d:
			_size = 3*2;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat3x4d:
			_size = 2*4;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat4x2d:
			_size = 4*2;
			_type = GL_DOUBLE;
			break;
		case eVertexDataType::datatype_mat4x3d:
			_size = 4*3;
			_type = GL_DOUBLE;
			break;
	}

	init(_index, _size, _type, _normalized, _stride, _offset);
}

void GlVertexAttribute::init(uint32_t _index, int32_t _size, uint32_t _type, bool _normalized, int32_t _stride, size_t _offset)
{
	semantic = eVertexSemantic::INVALID;
	index = _index;
	size = _size;
	type = _type;
	normalized = _normalized;
	stride = _stride;
	offset = _offset;

	char szDesc[256];
	sprintf_s(
		szDesc, sizeof(szDesc),
		"[%d_%d_%d_%c_%d_%d]",
		index, size, type, normalized ? 'n' : 'u', stride, offset);
	desc = szDesc;
}

bool GlVertexAttribute::isCompatibleAttribute(const GlVertexAttribute& other) const
{
	return 
		this->index == other.index &&
		this->size == other.size &&
		this->type == other.type &&
		this->normalized == other.normalized &&
		this->stride == other.stride &&
		this->offset == other.offset;
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

const GlVertexAttribute* GlVertexDefinition::getFirstAttributeBySemantic(eVertexSemantic semantic) const
{
	for (const GlVertexAttribute& attrib : attributes)
	{
		if (attrib.semantic == semantic)
		{
			return &attrib;
		}
	}

	return nullptr;
}

bool GlVertexDefinition::isCompatibleDefinition(const GlVertexDefinition& other) const
{
	if (this->vertexSize != other.vertexSize)
		return false;

	if (this->attributes.size() != other.attributes.size())
		return false;

	for (size_t index = 0; index < attributes.size(); index++)
	{
		auto& thisAttrib= this->attributes[index];
		auto& otherAttrib= other.attributes[index];

		if (!thisAttrib.isCompatibleAttribute(otherAttrib))
		{
			return false;
		}
	}

	return true;
}