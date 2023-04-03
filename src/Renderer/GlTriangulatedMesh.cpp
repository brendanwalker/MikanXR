#include "GlCommon.h"
#include "GlTriangulatedMesh.h"

GlTriangulatedMesh::GlTriangulatedMesh(
	std::string name,
	const GlVertexDefinition& vertexDefintion,
	const uint8_t* vertexData,
	uint32_t vertexCount,
	const uint8_t* indexData,
	uint32_t triangleCount,
	bool bOwnsVertexData)
{
	m_name = name;
	m_vertexDefinition = vertexDefintion;
	m_vertexData = vertexData;
	m_vertexCount = vertexCount;
	m_indexData = indexData;
	m_triangleCount = triangleCount;
	m_bOwnsVertexData = bOwnsVertexData;
}

GlTriangulatedMesh::~GlTriangulatedMesh()
{
	deleteBuffers();

	if (m_bOwnsVertexData)
	{
		if (m_vertexData != nullptr)
			delete[] m_vertexData;

		if (m_indexData != nullptr)
			delete[] m_indexData;
	}
}

void GlTriangulatedMesh::drawElements() const
{
	glBindVertexArray(m_glVertArray);
	glDrawElements(GL_TRIANGLES, (int)m_triangleCount * 3, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

bool GlTriangulatedMesh::createBuffers()
{
	if (m_vertexData != nullptr && m_vertexCount > 0 &&
		m_indexData != nullptr && m_triangleCount > 0)
	{
		uint32_t vertexSize = m_vertexDefinition.vertexSize;

		// create and bind a Vertex Array Object(VAO) to hold state for this model
		glGenVertexArrays(1, &m_glVertArray);
		glBindVertexArray(m_glVertArray);

		// Populate a vertex buffer
		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize * m_vertexCount, m_vertexData, GL_STATIC_DRAW);

		// Identify the components in the vertex buffer
		m_vertexDefinition.applyVertexDefintion();

		// Create and populate the index buffer
		glGenBuffers(1, &m_glIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			getElementCount() * getIndexPerElementCount() * getIndexSize(), // index array size in bytes
			m_indexData,
			GL_STATIC_DRAW);

		glBindVertexArray(0);

		return true;
	}

	return false;
}

void GlTriangulatedMesh::deleteBuffers()
{
	if (m_glIndexBuffer != 0)
		glDeleteBuffers(1, &m_glIndexBuffer);

	if (m_glVertArray != 0)
		glDeleteVertexArrays(1, &m_glVertArray);

	if (m_glVertBuffer != 0)
		glDeleteBuffers(1, &m_glVertBuffer);

	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_vertexCount = 0;
}