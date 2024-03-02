#pragma once

#include "GlTypesFwd.h"

#include "stdint.h"
#include <string>
#include <vector>

enum class eVertexSemantic : int
{
	INVALID = -1,

	// Specific Semantic Types
	position,
	normal,
	texCoord,
	color,
	colorAndSize,

	COUNT
};

enum class eVertexDataType : int
{
	INVALID = -1,

	// Generic Float Types
	datatype_float,
	datatype_vec2f,
	datatype_vec3f,
	datatype_vec4f,

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

	COUNT
};
extern const std::string* k_VertexDataTypeNames;

class GlVertexAttribute
{
public:
	GlVertexAttribute();
	GlVertexAttribute(const std::string& name, eVertexDataType dataType);

	bool isCompatibleAttribute(const GlVertexAttribute& other) const;
	const std::string& getName() const { return m_name; }
	int getLocation() const { return m_location; }
	size_t getAttributeSize() const { return m_attributeSize; }
	size_t getOffset() const { return m_offset; }
	eVertexSemantic getSemantic() const { return m_semantic; }
	eVertexDataType getDataType() const { return m_dataType; }

private:
	// OpenGL specific
	std::string m_name;
	GLint m_numComponents; // 1, 2, 3, 4
	GLenum m_componentType; // GL_FLOAT, GL_UNSIGNED_BYTE, GL_INT, GL_UNSIGNED_INT, GL_DOUBLE

	// Derived data
	GLuint m_location; // Attribute index in the vertex definition
	size_t m_attributeSize;
	size_t m_offset;
	eVertexSemantic m_semantic;
	eVertexDataType m_dataType;

	friend class GlVertexDefinition;
};

class GlVertexDefinition
{
public:
	GlVertexDefinition() = default;
	GlVertexDefinition(const GlVertexDefinition& vertexDefinition);
	GlVertexDefinition(const std::vector<GlVertexAttribute>&_attribtes);

	bool getIsValid() const { return m_bIsValid; }
	const std::vector<GlVertexAttribute>& getAttributes() const { return m_attributes; }
	size_t getVertexSize() const { return m_vertexSize; }

	void applyVertexDefintion() const;
	const std::string& getVertexDefinitionDesc() const { return m_description; }
	const GlVertexAttribute* getFirstAttributeBySemantic(eVertexSemantic semantic) const;
	const GlVertexAttribute* getAttributeByName(const std::string& name) const;
	bool isCompatibleDefinition(const GlVertexDefinition& other) const;

	static GlVertexDefinition extractFromGlProgram(const class GlProgram& program);

private:
	std::vector<GlVertexAttribute> m_attributes;
	std::string m_description;
	size_t m_vertexSize;
	bool m_bIsValid;
};