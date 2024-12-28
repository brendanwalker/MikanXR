#pragma once

#include "GlTypesFwd.h"
#include "GlVertexConstants.h"

#include "stdint.h"
#include <string>
#include <vector>


class GlVertexAttribute
{
public:
	GlVertexAttribute();
	GlVertexAttribute(
		const std::string& name, 
		eVertexDataType dataType, 
		eVertexSemantic semantic, 
		bool isNormalized= false);

	bool isCompatibleAttribute(const GlVertexAttribute& other) const;
	const std::string& getName() const { return m_name; }
	int getLocation() const { return m_location; }
	size_t getAttributeSize() const { return m_attributeSize; }
	size_t getOffset() const { return m_offset; }
	eVertexSemantic getSemantic() const { return m_semantic; }
	eVertexDataType getDataType() const { return m_dataType; }
	bool getIsNormalized() const { return m_bIsNormalized; }

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
	bool isCompatibleProgram(const class GlProgram& program) const;

private:
	std::vector<GlVertexAttribute> m_attributes;
	std::string m_description;
	size_t m_vertexSize;
	bool m_bIsValid;
};