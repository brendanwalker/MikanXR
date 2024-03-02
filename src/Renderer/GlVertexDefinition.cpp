#include "GlCommon.h"
#include "GlVertexDefinition.h"
#include "GlProgram.h"
#include "Logger.h"
#include "StringUtils.h"

const std::string g_VertexDataTypeNames[(int)eVertexDataType::COUNT] = {
	"float",
	"vec2f",
	"vec3f",
	"vec4f",
	"ubyte",
	"vec2ub",
	"vec3ub",
	"vec4ub",
	"int",
	"vec2i",
	"vec3i",
	"vec4i",
	"uint",
	"vec2ui",
	"vec3ui",
	"vec4ui",
	"double",
	"vec2d",
	"vec3d",
	"vec4d",
};
const std::string* k_VertexDataTypeNames = g_VertexDataTypeNames;

namespace VertexDefinitionUtils
{
	GLenum determineComponentType(eVertexDataType dataType);
	GLint determineNumComponents(eVertexDataType dataType);
	eVertexDataType determineDataType(GLenum glAttribType);
	size_t getDataTypeSize(eVertexDataType dataType);
	eVertexSemantic determineSemanticFromName(const std::string& name);
};

// -- GlVertexAttribute ------
GlVertexAttribute::GlVertexAttribute()
	: m_name("")
	, m_numComponents()
	, m_componentType(GL_INVALID_ENUM)
	, m_location(0)
	, m_attributeSize(0)
	, m_offset(0)
	, m_semantic(eVertexSemantic::INVALID)
	, m_dataType(eVertexDataType::INVALID)
{
}

GlVertexAttribute::GlVertexAttribute(const std::string& name, eVertexDataType dataType)
	: m_name(name)
	, m_numComponents(VertexDefinitionUtils::determineNumComponents(dataType))
	, m_componentType(VertexDefinitionUtils::determineComponentType(dataType))
	, m_location(0)
	, m_attributeSize(VertexDefinitionUtils::getDataTypeSize(dataType))
	, m_offset(0)
	, m_semantic(VertexDefinitionUtils::determineSemanticFromName(name))
	, m_dataType(dataType)
{
}

bool GlVertexAttribute::isCompatibleAttribute(const GlVertexAttribute& other) const
{
	return 
		this->m_location == other.m_location &&
		this->m_numComponents == other.m_numComponents &&
		this->m_componentType == other.m_componentType &&
		this->m_offset == other.m_offset;
}

// -- GlVertexDefinition ------
GlVertexDefinition::GlVertexDefinition(const GlVertexDefinition& vertexDefinition)
{
	this->m_attributes = vertexDefinition.m_attributes;
	this->m_description = vertexDefinition.m_description;
	this->m_vertexSize = vertexDefinition.m_vertexSize;
	this->m_bIsValid = vertexDefinition.m_bIsValid;
}

GlVertexDefinition::GlVertexDefinition(const std::vector<GlVertexAttribute>& attribtes)
	: m_attributes(attribtes)
	, m_description("")
	, m_vertexSize(0)
	, m_bIsValid(true)
{
	for (int location= 0; location < m_attributes.size(); location++)
	{
		GlVertexAttribute& attrib = m_attributes[location];

		attrib.m_location = location;
		attrib.m_offset = m_vertexSize;

		std::string dataTypeName= 
			attrib.m_dataType != eVertexDataType::INVALID
			? g_VertexDataTypeNames[(int)attrib.m_dataType]
			: "INVALID";
		std::string attribDesc= 
			StringUtils::stringify(
				"[", attrib.m_location, "_", attrib.m_name, "_", dataTypeName, "]");
		m_description += attribDesc;

		m_vertexSize += attrib.m_attributeSize;
		m_bIsValid&= attrib.m_dataType != eVertexDataType::INVALID;
	}
}

void GlVertexDefinition::applyVertexDefintion() const
{
	// Identify the components in the vertex buffer
	for (uint32_t attribIndex = 0; attribIndex < m_attributes.size(); ++attribIndex)
	{
		GlVertexAttribute attrib = m_attributes[attribIndex];
		
		// Specifies the byte offset between consecutive generic vertex attributes. 
		// If stride is 0, the generic vertex attributes are understood to be tightly packed in the array. 
		// The initial value is 0.
		GLsizei stride= 0; 

		glEnableVertexAttribArray(attribIndex);
		glVertexAttribPointer(
			attrib.m_location, 
			attrib.m_numComponents, 
			attrib.m_componentType, 
			false, // not using fixed-point data
			stride,
			(GLvoid*)attrib.m_offset);
	}
}

const GlVertexAttribute* GlVertexDefinition::getFirstAttributeBySemantic(eVertexSemantic semantic) const
{
	for (const GlVertexAttribute& attrib : m_attributes)
	{
		if (attrib.m_semantic == semantic)
		{
			return &attrib;
		}
	}

	return nullptr;
}

const GlVertexAttribute* GlVertexDefinition::getAttributeByName(const std::string& name) const
{
	for (const GlVertexAttribute& attrib : m_attributes)
	{
		if (attrib.m_name == name)
		{
			return &attrib;
		}
	}

	return nullptr;
}

bool GlVertexDefinition::isCompatibleDefinition(const GlVertexDefinition& other) const
{
	if (this->m_vertexSize != other.m_vertexSize)
		return false;

	if (this->m_attributes.size() != other.m_attributes.size())
		return false;

	for (size_t index = 0; index < m_attributes.size(); index++)
	{
		auto& thisAttrib= this->m_attributes[index];
		auto& otherAttrib= other.m_attributes[index];

		if (!thisAttrib.isCompatibleAttribute(otherAttrib))
		{
			return false;
		}
	}

	return true;
}

GlVertexDefinition GlVertexDefinition::extractFromGlProgram(const GlProgram& program)
{
	uint32_t programId = program.getGlProgramId();
	const std::string programName = program.getProgramCode().getProgramName();

	if (programId == 0)
		return GlVertexDefinition();

	GLint numAttributes;
	glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &numAttributes);

	// Extract the attributes from the program
	bool bSuccess = true;
	std::vector<GlVertexAttribute> attributes(numAttributes);
	for (int attribIndex = 0; attribIndex < numAttributes; ++attribIndex)
	{
		GLchar attribName[256];
		GLint attribArraySize;
		GLenum attribType;

		glGetActiveAttrib(
			programId,
			GLuint(attribIndex),
			(GLsizei)sizeof(attribName),
			nullptr,
			&attribArraySize,
			&attribType,
			attribName);
		if (attribArraySize != 1)
		{
			MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
				"Program " << programName <<
				" has unsupported array[" << attribArraySize <<"] attribute " << attribName;
			bSuccess = false;
			break;
		}

		GLint location = glGetAttribLocation(programId, attribName);
		if (location != -1)
		{
			GLint componentStride;
			glGetVertexAttribiv(location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &componentStride);
			if (componentStride != 0)
			{
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
					"Program " << programName <<
					" has unsupported non-zero stride for attribute " << attribName;
				bSuccess = false;
				break;
			}

			GLint componentNormalized = 0;
			glGetVertexAttribiv(location, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &componentNormalized);
			if (componentNormalized != 0)
			{
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
					"Program " << programName <<
					" has attribute " << attribName <<
					" using fixed point arithmetic";
				bSuccess = false;
				break;
			}

			eVertexDataType dataType = VertexDefinitionUtils::determineDataType(attribType);
			if (dataType == eVertexDataType::INVALID)
			{
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
					"Program " << programName <<
					" has attribute " << attribName <<
					" using fixed point arithmetic";
				bSuccess = false;
				break;
			}

			// Slot the attribute into the specified location
			if (location < attributes.size())
			{
				attributes[location] = GlVertexAttribute(attribName, dataType);
			}
			else
			{
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
					"Program " << programName <<
					" has out of range attribute " << attribName <<
					". One or more attributes not being used?";
				bSuccess = false;
				break;
			}
		}
		else
		{
			bSuccess = false;
		}
	}

	return GlVertexDefinition(attributes);
}

namespace VertexDefinitionUtils
{
	GLenum determineComponentType(eVertexDataType dataType)
	{
		switch (dataType)
		{
			case eVertexDataType::datatype_ubyte:
			case eVertexDataType::datatype_vec2ub:
			case eVertexDataType::datatype_vec3ub:
			case eVertexDataType::datatype_vec4ub:
				return GL_UNSIGNED_BYTE;

			case eVertexDataType::datatype_int:
			case eVertexDataType::datatype_vec2i:
			case eVertexDataType::datatype_vec3i:
			case eVertexDataType::datatype_vec4i:
				return GL_INT;

			case eVertexDataType::datatype_uint:
			case eVertexDataType::datatype_vec2ui:
			case eVertexDataType::datatype_vec3ui:
			case eVertexDataType::datatype_vec4ui:
				return GL_UNSIGNED_INT;

			case eVertexDataType::datatype_float:
			case eVertexDataType::datatype_vec2f:
			case eVertexDataType::datatype_vec3f:
			case eVertexDataType::datatype_vec4f:
				return GL_FLOAT;

			case eVertexDataType::datatype_double:
			case eVertexDataType::datatype_vec2d:
			case eVertexDataType::datatype_vec3d:
			case eVertexDataType::datatype_vec4d:
				return GL_DOUBLE;
		}

		return GL_INVALID_ENUM;
	}

	GLint determineNumComponents(eVertexDataType dataType)
	{
		switch (dataType)
		{
			case eVertexDataType::datatype_ubyte:
			case eVertexDataType::datatype_int:
			case eVertexDataType::datatype_uint:
			case eVertexDataType::datatype_float:
			case eVertexDataType::datatype_double:
				return 1;

			case eVertexDataType::datatype_vec2ub:
			case eVertexDataType::datatype_vec2i:
			case eVertexDataType::datatype_vec2ui:
			case eVertexDataType::datatype_vec2f:
			case eVertexDataType::datatype_vec2d:
				return 2;

			case eVertexDataType::datatype_vec3ub:
			case eVertexDataType::datatype_vec3i:
			case eVertexDataType::datatype_vec3ui:
			case eVertexDataType::datatype_vec3f:
			case eVertexDataType::datatype_vec3d:
				return 3;

			case eVertexDataType::datatype_vec4ub:
			case eVertexDataType::datatype_vec4i:
			case eVertexDataType::datatype_vec4ui:
			case eVertexDataType::datatype_vec4f:
			case eVertexDataType::datatype_vec4d:
				return 4;
		}

		return 0;
	}

	eVertexDataType determineDataType(GLenum glAttribType)
	{
		switch (glAttribType)
		{
			case GL_FLOAT:
				return eVertexDataType::datatype_float;
			case GL_FLOAT_VEC2:
				return eVertexDataType::datatype_vec2f;
			case GL_FLOAT_VEC3:
				return eVertexDataType::datatype_vec3f;
			case GL_FLOAT_VEC4:
				return eVertexDataType::datatype_vec4f;
			case GL_INT:
				return eVertexDataType::datatype_int;
			case GL_INT_VEC2:
				return eVertexDataType::datatype_vec2i;
			case GL_INT_VEC3:
				return eVertexDataType::datatype_vec3i;
			case GL_INT_VEC4:
				return eVertexDataType::datatype_vec4i;
			case GL_UNSIGNED_INT:
				return eVertexDataType::datatype_uint;
			case GL_UNSIGNED_INT_VEC2:
				return eVertexDataType::datatype_vec2ui;
			case GL_UNSIGNED_INT_VEC3:
				return eVertexDataType::datatype_vec3ui;
			case GL_UNSIGNED_INT_VEC4:
				return eVertexDataType::datatype_vec4ui;
			case GL_DOUBLE:
				return eVertexDataType::datatype_double;
			case GL_DOUBLE_VEC2:
				return eVertexDataType::datatype_vec2d;
			case GL_DOUBLE_VEC3:
				return eVertexDataType::datatype_vec3d;
			case GL_DOUBLE_VEC4:
				return eVertexDataType::datatype_vec4d;
		}

		return eVertexDataType::INVALID;
	}

	size_t getDataTypeSize(eVertexDataType dataType)
	{
		switch (dataType)
		{
			case eVertexDataType::datatype_float:
				return sizeof(GLfloat);
			case eVertexDataType::datatype_vec2f:
				return sizeof(GLfloat)*2;
			case eVertexDataType::datatype_vec3f:
				return sizeof(GLfloat)*3;
			case eVertexDataType::datatype_vec4f:
				return sizeof(GLfloat)*4;
			case eVertexDataType::datatype_ubyte:
				return sizeof(GLubyte);
			case eVertexDataType::datatype_vec2ub:
				return sizeof(GLubyte)*2;
			case eVertexDataType::datatype_vec3ub:
				return sizeof(GLubyte)*3;
			case eVertexDataType::datatype_vec4ub:
				return sizeof(GLubyte)*4;
			case eVertexDataType::datatype_int:
				return sizeof(GLint);
			case eVertexDataType::datatype_vec2i:
				return sizeof(GLint)*2;
			case eVertexDataType::datatype_vec3i:
				return sizeof(GLint)*3;
			case eVertexDataType::datatype_vec4i:
				return sizeof(GLint)*4;
			case eVertexDataType::datatype_uint:
				return sizeof(GLuint);
			case eVertexDataType::datatype_vec2ui:
				return sizeof(GLuint)*2;
			case eVertexDataType::datatype_vec3ui:
				return sizeof(GLuint)*3;
			case eVertexDataType::datatype_vec4ui:
				return sizeof(GLuint)*4;
			case eVertexDataType::datatype_double:
				return sizeof(GLdouble);
			case eVertexDataType::datatype_vec2d:
				return sizeof(GLdouble)*2;
			case eVertexDataType::datatype_vec3d:
				return sizeof(GLdouble)*3;
			case eVertexDataType::datatype_vec4d:
				return sizeof(GLdouble)*4;
			default:
				return 0;
		}
	}

	eVertexSemantic determineSemanticFromName(const std::string& name)
	{
		// TODO: Standardize the names
		if (name == "position" || name == "in_position" || name == "aPos" || name == "inPosition")
			return eVertexSemantic::position;
		else if (name == "normal" || name == "v3NormalIn")
			return eVertexSemantic::normal;
		else if (name == "texcoord" || name == "aTexCoords" || name == "v2TexCoordsIn" || name == "inTexCoord0")
			return eVertexSemantic::texCoord;
		else if (name == "color" || name == "inColor0")
			return eVertexSemantic::color;
		else if (name == "colorAndSize" || name == "in_colorPointSize")
			return eVertexSemantic::colorAndSize;
		else
			return eVertexSemantic::INVALID;
	}
};