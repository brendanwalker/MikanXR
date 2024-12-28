#include "GlCommon.h"
#include "GlVertexDefinition.h"
#include "GlProgram.h"
#include "Logger.h"
#include "StringUtils.h"

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
	, m_bIsNormalized(GL_FALSE)
	, m_location(0)
	, m_attributeSize(0)
	, m_offset(0)
	, m_semantic(eVertexSemantic::INVALID)
	, m_dataType(eVertexDataType::INVALID)
{
}

GlVertexAttribute::GlVertexAttribute(
	const std::string& name, 
	eVertexDataType dataType, 
	eVertexSemantic semantic,
	bool isNormalized)
	: m_name(name)
	, m_numComponents(VertexDefinitionUtils::determineNumComponents(dataType))
	, m_componentType(VertexDefinitionUtils::determineComponentType(dataType))
	, m_bIsNormalized(isNormalized)
	, m_location(0)
	, m_attributeSize(VertexDefinitionUtils::getDataTypeSize(dataType))
	, m_offset(0)
	, m_semantic(semantic)
	, m_dataType(dataType)
{
}

bool GlVertexAttribute::isCompatibleAttribute(const GlVertexAttribute& other) const
{
	return 
		this->m_location == other.m_location &&
		this->m_numComponents == other.m_numComponents &&
		this->m_componentType == other.m_componentType &&
		this->m_bIsNormalized == other.m_bIsNormalized &&
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
			? k_VertexDataTypeNames[(int)attrib.m_dataType]
			: "INVALID";
		std::string normalizedStr= attrib.m_bIsNormalized ? "_norm" : "";
		std::string attribDesc= 
			StringUtils::stringify(
				"[", attrib.m_location, "_", attrib.m_name, "_", dataTypeName, normalizedStr, "]");
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
		GLsizei stride= (GLsizei)m_vertexSize; 

		glEnableVertexAttribArray(attribIndex);
		glVertexAttribPointer(
			attrib.m_location, 
			attrib.m_numComponents, 
			attrib.m_componentType, 
			attrib.m_bIsNormalized ? GL_TRUE : GL_FALSE, 
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

bool GlVertexDefinition::isCompatibleProgram(const GlProgram& program) const
{
	uint32_t programId = program.getGlProgramId();
	const std::string programName = program.getProgramCode().getProgramName();

	if (programId == 0)
		return false;

	GLint numAttributes;
	glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &numAttributes);

	// Extract the attributes from the program
	bool bSuccess = true;
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
			// Slot the attribute into the specified location
			if (location < m_attributes.size())
			{
				const GlVertexAttribute& attrib = m_attributes[location];
				eVertexDataType expectedDataType = attrib.getDataType();

				eVertexDataType actualDataType = VertexDefinitionUtils::determineDataType(attribType);
				if (actualDataType == eVertexDataType::INVALID)
				{
					MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
						"Program " << programName <<
						" has attribute " << attribName <<
						" using unsupported GL datatype 0x" << std::hex << attribType;
					bSuccess = false;
					break;
				}

				// See if the data type matches
				// or if using normalization on a fixed point type
				// see if the actual data type is a compatible float type
				bool bHasCompatibleTypes = expectedDataType == actualDataType;
				if (!bHasCompatibleTypes && attrib.getIsNormalized())
				{
					switch (expectedDataType)
					{
						case eVertexDataType::datatype_ubyte:
						case eVertexDataType::datatype_uint:
						case eVertexDataType::datatype_int:
							bHasCompatibleTypes = actualDataType == eVertexDataType::datatype_float;
							break;
						case eVertexDataType::datatype_ubvec2:
						case eVertexDataType::datatype_uvec2:
						case eVertexDataType::datatype_ivec2:
							bHasCompatibleTypes = actualDataType == eVertexDataType::datatype_vec2;
							break;
						case eVertexDataType::datatype_ubvec3:
						case eVertexDataType::datatype_uvec3:
						case eVertexDataType::datatype_ivec3:
							bHasCompatibleTypes = actualDataType == eVertexDataType::datatype_vec3;
							break;
						case eVertexDataType::datatype_ubvec4:
						case eVertexDataType::datatype_uvec4:
						case eVertexDataType::datatype_ivec4:
							bHasCompatibleTypes = actualDataType == eVertexDataType::datatype_vec4;
							break;
					}
				}

				if (!bHasCompatibleTypes)
				{
					MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
						"Program " << programName <<
						" has attribute " << attribName <<
						" with mismatched data type " <<
						"(expected: "  << k_VertexDataTypeNames[(int)expectedDataType] <<
						", actual: " << k_VertexDataTypeNames[(int)actualDataType] << ")";
					bSuccess = false;
					break;
				}
			}
			else
			{
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
					"Program " << programName <<
					" has out of range attribute " << attribName;
				bSuccess = false;
				break;
			}
		}
		else
		{
			MIKAN_LOG_ERROR("GlVertexDefinition::extractFromGlProgram") <<
				"Program " << programName <<
				" has missing location for attrib " << attribName;
			bSuccess = false;
		}
	}

	return bSuccess;
}

namespace VertexDefinitionUtils
{
	GLenum determineComponentType(eVertexDataType dataType)
	{
		switch (dataType)
		{
			case eVertexDataType::datatype_ubyte:
			case eVertexDataType::datatype_ubvec2:
			case eVertexDataType::datatype_ubvec3:
			case eVertexDataType::datatype_ubvec4:
				return GL_UNSIGNED_BYTE;

			case eVertexDataType::datatype_int:
			case eVertexDataType::datatype_ivec2:
			case eVertexDataType::datatype_ivec3:
			case eVertexDataType::datatype_ivec4:
				return GL_INT;

			case eVertexDataType::datatype_uint:
			case eVertexDataType::datatype_uvec2:
			case eVertexDataType::datatype_uvec3:
			case eVertexDataType::datatype_uvec4:
				return GL_UNSIGNED_INT;

			case eVertexDataType::datatype_float:
			case eVertexDataType::datatype_vec2:
			case eVertexDataType::datatype_vec3:
			case eVertexDataType::datatype_vec4:
				return GL_FLOAT;

			case eVertexDataType::datatype_double:
			case eVertexDataType::datatype_dvec2:
			case eVertexDataType::datatype_dvec3:
			case eVertexDataType::datatype_dvec4:
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

			case eVertexDataType::datatype_ubvec2:
			case eVertexDataType::datatype_ivec2:
			case eVertexDataType::datatype_uvec2:
			case eVertexDataType::datatype_vec2:
			case eVertexDataType::datatype_dvec2:
				return 2;

			case eVertexDataType::datatype_ubvec3:
			case eVertexDataType::datatype_ivec3:
			case eVertexDataType::datatype_uvec3:
			case eVertexDataType::datatype_vec3:
			case eVertexDataType::datatype_dvec3:
				return 3;

			case eVertexDataType::datatype_ubvec4:
			case eVertexDataType::datatype_ivec4:
			case eVertexDataType::datatype_uvec4:
			case eVertexDataType::datatype_vec4:
			case eVertexDataType::datatype_dvec4:
				return 4;
		}

		return 0;
	}

	eVertexDataType determineDataType(GLenum glAttribType)
	{
		switch (glAttribType)
		{
			case GL_UNSIGNED_BYTE:
				return eVertexDataType::datatype_ubyte;

			case GL_INT:
				return eVertexDataType::datatype_int;
			case GL_INT_VEC2:
				return eVertexDataType::datatype_ivec2;
			case GL_INT_VEC3:
				return eVertexDataType::datatype_ivec3;
			case GL_INT_VEC4:
				return eVertexDataType::datatype_ivec4;

			case GL_UNSIGNED_INT:
				return eVertexDataType::datatype_uint;
			case GL_UNSIGNED_INT_VEC2:
				return eVertexDataType::datatype_uvec2;
			case GL_UNSIGNED_INT_VEC3:
				return eVertexDataType::datatype_uvec3;
			case GL_UNSIGNED_INT_VEC4:
				return eVertexDataType::datatype_uvec4;

			case GL_FLOAT:
				return eVertexDataType::datatype_float;
			case GL_FLOAT_VEC2:
				return eVertexDataType::datatype_vec2;
			case GL_FLOAT_VEC3:
				return eVertexDataType::datatype_vec3;
			case GL_FLOAT_VEC4:
				return eVertexDataType::datatype_vec4;

			case GL_DOUBLE:
				return eVertexDataType::datatype_double;
			case GL_DOUBLE_VEC2:
				return eVertexDataType::datatype_dvec2;
			case GL_DOUBLE_VEC3:
				return eVertexDataType::datatype_dvec3;
			case GL_DOUBLE_VEC4:
				return eVertexDataType::datatype_dvec4;
		}

		return eVertexDataType::INVALID;
	}

	size_t getDataTypeSize(eVertexDataType dataType)
	{
		switch (dataType)
		{
			case eVertexDataType::datatype_ubyte:
				return sizeof(GLbyte);
			case eVertexDataType::datatype_ubvec2:
				return sizeof(GLbyte) * 2;
			case eVertexDataType::datatype_ubvec3:
				return sizeof(GLbyte) * 3;
			case eVertexDataType::datatype_ubvec4:
				return sizeof(GLbyte) * 4;

			case eVertexDataType::datatype_int:
				return sizeof(GLint);
			case eVertexDataType::datatype_ivec2:
				return sizeof(GLint) * 2;
			case eVertexDataType::datatype_ivec3:
				return sizeof(GLint) * 3;
			case eVertexDataType::datatype_ivec4:
				return sizeof(GLint) * 4;

			case eVertexDataType::datatype_uint:
				return sizeof(GLuint);
			case eVertexDataType::datatype_uvec2:
				return sizeof(GLuint) * 2;
			case eVertexDataType::datatype_uvec3:
				return sizeof(GLuint) * 3;
			case eVertexDataType::datatype_uvec4:
				return sizeof(GLuint) * 4;

			case eVertexDataType::datatype_float:
				return sizeof(GLfloat);
			case eVertexDataType::datatype_vec2:
				return sizeof(GLfloat)*2;
			case eVertexDataType::datatype_vec3:
				return sizeof(GLfloat)*3;
			case eVertexDataType::datatype_vec4:
				return sizeof(GLfloat)*4;

			case eVertexDataType::datatype_double:
				return sizeof(GLdouble);
			case eVertexDataType::datatype_dvec2:
				return sizeof(GLdouble)*2;
			case eVertexDataType::datatype_dvec3:
				return sizeof(GLdouble)*3;
			case eVertexDataType::datatype_dvec4:
				return sizeof(GLdouble)*4;
			default:
				return 0;
		}
	}
};