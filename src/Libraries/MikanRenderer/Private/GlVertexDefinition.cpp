#include "GlCommon.h"
#include "IMkVertexDefinition.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "Logger.h"
#include "StringUtils.h"

namespace VertexDefinitionUtils
{
	GLenum determineComponentType(eVertexDataType dataType);
	GLint determineNumComponents(eVertexDataType dataType);
	eVertexDataType determineDataType(GLenum glAttribType);
	size_t getDataTypeSize(eVertexDataType dataType);
};

class GlVertexAttribute : public IMkVertexAttribute
{
public:
	GlVertexAttribute()
		: m_name("")
		, m_numComponents()
		, m_componentType(GL_INVALID_ENUM)
		, m_bIsNormalized(GL_FALSE)
		, m_location(0)
		, m_attributeSize(0)
		, m_offset(0)
		, m_semantic(eVertexSemantic::INVALID)
		, m_dataType(eVertexDataType::INVALID)
	{}

	GlVertexAttribute(
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
	{}

	virtual bool isCompatibleAttribute(IMkVertexAttributeConstPtr other) const override
	{
		const auto* otherGlAttrib = static_cast<const GlVertexAttribute*>(other.get());

		return
			this->m_location == otherGlAttrib->m_location &&
			this->m_numComponents == otherGlAttrib->m_numComponents &&
			this->m_componentType == otherGlAttrib->m_componentType &&
			this->m_bIsNormalized == otherGlAttrib->m_bIsNormalized &&
			this->m_offset == otherGlAttrib->m_offset;
	}

	virtual const std::string& getName() const override { return m_name; }
	virtual int getLocation() const override { return m_location; }
	virtual size_t getAttributeSize() const override { return m_attributeSize; }
	virtual size_t getOffset() const override { return m_offset; }
	virtual eVertexSemantic getSemantic() const override { return m_semantic; }
	virtual eVertexDataType getDataType() const override { return m_dataType; }
	virtual bool getIsNormalized() const override { return m_bIsNormalized; }

	virtual void setLocation(int location) override
	{
		m_location = location;	
	}

	virtual void setOffset(size_t offset) override
	{
		m_offset = offset;
	}

private:
	// OpenGL specific
	std::string m_name;
	GLint m_numComponents; // 1, 2, 3, 4
	GLenum m_componentType; // GL_FLOAT, GL_UNSIGNED_BYTE, GL_INT, GL_UNSIGNED_INT, GL_DOUBLE
	bool m_bIsNormalized; // Fixed point data (like ubyte colors) normalized to [0, 1] or [-1, 1]

	// Derived data
	GLuint m_location; // Attribute index in the vertex definition
	size_t m_attributeSize;
	size_t m_offset;
	eVertexSemantic m_semantic;
	eVertexDataType m_dataType;

	friend class GlVertexDefinition;
};

class GlVertexDefinition : public IMkVertexDefinition
{
public:
	GlVertexDefinition() = default;

	GlVertexDefinition::GlVertexDefinition(IMkVertexDefinitionConstPtr vertexDefinition)
	{
		this->m_attributes = vertexDefinition->getAttributes();
		this->m_description = vertexDefinition->getVertexDefinitionDesc();
		this->m_vertexSize = vertexDefinition->getVertexSize();
		this->m_bIsValid = vertexDefinition->getIsValid();
	}

	GlVertexDefinition::GlVertexDefinition(const std::vector<IMkVertexAttributeConstPtr>& attribtes)
		: m_attributes(attribtes)
		, m_description("")
		, m_vertexSize(0)
		, m_bIsValid(true)
	{
		for (int location = 0; location < m_attributes.size(); location++)
		{
			IMkVertexAttributePtr attrib = 
				std::const_pointer_cast<IMkVertexAttribute>(m_attributes[location]);

			attrib->setLocation(location);
			attrib->setOffset(m_vertexSize);

			std::string dataTypeName =
				attrib->getDataType() != eVertexDataType::INVALID
				? k_VertexDataTypeNames[(int)attrib->getDataType()]
				: "INVALID";
			std::string normalizedStr = attrib->getIsNormalized() ? "_norm" : "";
			std::string attribDesc =
				StringUtils::stringify(
					"[", 
					attrib->getLocation(), "_", 
					attrib->getName(), "_",
					dataTypeName, normalizedStr, 
					"]");
			m_description += attribDesc;

			m_vertexSize += attrib->getAttributeSize();
			m_bIsValid &= attrib->getDataType() != eVertexDataType::INVALID;
		}
	}

	virtual bool getIsValid() const override { return m_bIsValid; }
	virtual const std::vector<IMkVertexAttributeConstPtr>& getAttributes() const override { return m_attributes; }
	virtual size_t getVertexSize() const override { return m_vertexSize; }

	void applyVertexDefintion() const
	{
		// Identify the components in the vertex buffer
		for (uint32_t attribIndex = 0; attribIndex < m_attributes.size(); ++attribIndex)
		{
			IMkVertexAttributeConstPtr attrib = m_attributes[attribIndex];
			const auto* glAttrib = static_cast<const GlVertexAttribute*>(attrib.get());

			// Specifies the byte offset between consecutive generic vertex attributes. 
			// If stride is 0, the generic vertex attributes are understood to be tightly packed in the array. 
			// The initial value is 0.
			GLsizei stride = (GLsizei)m_vertexSize;

			glEnableVertexAttribArray(attribIndex);
			glVertexAttribPointer(
				glAttrib->m_location,
				glAttrib->m_numComponents,
				glAttrib->m_componentType,
				glAttrib->m_bIsNormalized ? GL_TRUE : GL_FALSE,
				stride,
				(GLvoid*)glAttrib->m_offset);
		}
	}

	virtual const std::string& getVertexDefinitionDesc() const override 
	{ 
		return m_description;
	}

	virtual const IMkVertexAttribute* getFirstAttributeBySemantic(eVertexSemantic semantic) const override
	{
		for (IMkVertexAttributeConstPtr attrib : m_attributes)
		{
			if (attrib->getSemantic() == semantic)
			{
				return attrib.get();
			}
		}

		return nullptr;
	}

	virtual const IMkVertexAttribute* getAttributeByName(const std::string& name) const override
	{
		for (IMkVertexAttributeConstPtr attrib : m_attributes)
		{
			if (attrib->getName() == name)
			{
				return attrib.get();
			}
		}

		return nullptr;
	}

	virtual bool isCompatibleDefinition(IMkVertexDefinitionConstPtr other) const override
	{
		if (this->m_vertexSize != other->getVertexSize())
			return false;

		if (this->m_attributes.size() != other->getAttributes().size())
			return false;

		for (size_t index = 0; index < m_attributes.size(); index++)
		{
			auto& thisAttrib = this->m_attributes[index];
			auto& otherAttrib = other->getAttributes()[index];

			if (!thisAttrib->isCompatibleAttribute(otherAttrib))
			{
				return false;
			}
		}

		return true;
	}

	virtual bool isCompatibleProgram(IMkShaderConstPtr program) const override
	{
		uint32_t programId = program->getIMkShaderId();
		const std::string programName = program->getProgramCode()->getProgramName();

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
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromIMkShader") <<
					"Program " << programName <<
					" has unsupported array[" << attribArraySize << "] attribute " << attribName;
				bSuccess = false;
				break;
			}

			GLint location = glGetAttribLocation(programId, attribName);
			if (location != -1)
			{
				// Slot the attribute into the specified location
				if (location < m_attributes.size())
				{
					IMkVertexAttributeConstPtr attrib = m_attributes[location];
					eVertexDataType expectedDataType = attrib->getDataType();

					eVertexDataType actualDataType = VertexDefinitionUtils::determineDataType(attribType);
					if (actualDataType == eVertexDataType::INVALID)
					{
						MIKAN_LOG_ERROR("GlVertexDefinition::extractFromIMkShader") <<
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
					if (!bHasCompatibleTypes && attrib->getIsNormalized())
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
						MIKAN_LOG_ERROR("GlVertexDefinition::extractFromIMkShader") <<
							"Program " << programName <<
							" has attribute " << attribName <<
							" with mismatched data type " <<
							"(expected: " << k_VertexDataTypeNames[(int)expectedDataType] <<
							", actual: " << k_VertexDataTypeNames[(int)actualDataType] << ")";
						bSuccess = false;
						break;
					}
				}
				else
				{
					MIKAN_LOG_ERROR("GlVertexDefinition::extractFromIMkShader") <<
						"Program " << programName <<
						" has out of range attribute " << attribName;
					bSuccess = false;
					break;
				}
			}
			else
			{
				MIKAN_LOG_ERROR("GlVertexDefinition::extractFromIMkShader") <<
					"Program " << programName <<
					" has missing location for attrib " << attribName;
				bSuccess = false;
			}
		}

		return bSuccess;
	}


private:
	std::vector<IMkVertexAttributeConstPtr> m_attributes;
	std::string m_description;
	size_t m_vertexSize;
	bool m_bIsValid;
};

IMkVertexAttributePtr createMkVertexAttribute(
	const std::string& name,
	eVertexDataType dataType,
	eVertexSemantic semantic,
	bool isNormalized)
{
	return std::make_shared<GlVertexAttribute>(name, dataType, semantic, isNormalized);
}

IMkVertexDefinitionPtr createMkVertexDefinition(
	IMkVertexDefinitionConstPtr vertexDefinition)
{
	return std::make_shared<GlVertexDefinition>(vertexDefinition);
}

IMkVertexDefinitionPtr createMkVertexDefinition(
	const std::vector<IMkVertexAttributeConstPtr>& attribtes)
{
	return std::make_shared<GlVertexDefinition>(attribtes);
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