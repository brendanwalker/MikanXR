#include "GlCommon.h"
#include "IMkCamera.h"
#include "GlTriangulatedMesh.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "IMkShaderCache.h"
#include "GlVertexDefinition.h"
#include "IMkWindow.h"
#include "Logger.h"

GlTriangulatedMesh::GlTriangulatedMesh(class IMkWindow* ownerWindow) 
	: m_ownerWindow(ownerWindow)
{
}

GlTriangulatedMesh::GlTriangulatedMesh(
	class IMkWindow* ownerWindow,
	std::string name,
	const uint8_t* vertexData,
	const size_t vertexSize,
	uint32_t vertexCount,
	const uint8_t* indexData,
	const size_t indexSize,
	uint32_t triangleCount,
	bool bOwnsVertexData) 
	: m_ownerWindow(ownerWindow)
	, m_name(name)
	, m_vertexData(vertexData)
	, m_vertexSize(vertexSize)
	, m_vertexCount(vertexCount)
	, m_indexData(indexData)
	, m_indexSize(indexSize)
	, m_triangleCount(triangleCount)
	, m_bOwnsVertexData(bOwnsVertexData)
{
}

GlTriangulatedMesh::~GlTriangulatedMesh()
{
	deleteResources();

	if (m_bOwnsVertexData)
	{
		if (m_vertexData != nullptr)
			delete[] m_vertexData;

		if (m_indexData != nullptr)
			delete[] m_indexData;
	}
}

bool GlTriangulatedMesh::setMaterial(GlMaterialConstPtr material)
{
	if (material && 
		material->getProgram()->getVertexDefinition().getVertexSize() == m_vertexSize)
	{
		m_materialInstance = std::make_shared<GlMaterialInstance>(material);
		return true;
	}

	return false;
}

bool GlTriangulatedMesh::setMaterialInstance(GlMaterialInstancePtr materialInstance)
{
	if (materialInstance &&
		materialInstance->getMaterial()->getProgram()->getVertexDefinition().getVertexSize() == m_vertexSize)
	{
		m_materialInstance= materialInstance;
		return true;
	}

	return false;
}

void GlTriangulatedMesh::drawElements() const
{
	GLenum indexType = GL_UNSIGNED_SHORT;
	switch (m_indexSize)
	{
	case 4:
		indexType = GL_UNSIGNED_INT;
		break;
	case 2:
		indexType = GL_UNSIGNED_SHORT;
		break;
	case 1:
		indexType = GL_UNSIGNED_BYTE;
		break;
	}

	glBindVertexArray(m_glVertArray);
	glDrawElements(
		GL_TRIANGLES, 
		(int)m_triangleCount * 3, 
		indexType, 
		nullptr);
	glBindVertexArray(0);
}

bool GlTriangulatedMesh::createResources()
{
	if (m_vertexData == nullptr || m_vertexCount == 0 ||
		m_indexData == nullptr || m_triangleCount == 0)
	{
		return false;
	}

	if (m_materialInstance == nullptr)
	{
		return false;
	}

	GlMaterialConstPtr material = m_materialInstance->getMaterial();
	const GlVertexDefinition& vertexDefinition = material->getProgram()->getVertexDefinition();
	const size_t vertexSize = vertexDefinition.getVertexSize();
	if (vertexSize != m_vertexSize)
	{
		return false;
	}

	// create and bind a Vertex Array Object(VAO) to hold state for this model
	glGenVertexArrays(1, &m_glVertArray);
	glBindVertexArray(m_glVertArray);
	if (!m_name.empty())
	{
		glObjectLabel(GL_VERTEX_ARRAY, m_glVertArray, -1, m_name.c_str());
	}

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexSize * m_vertexCount, m_vertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	vertexDefinition.applyVertexDefintion();

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

void GlTriangulatedMesh::deleteResources()
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

// -- Drawing Helpers ---
GlTriangulatedMeshPtr createFullscreenQuadMesh(IMkWindow* ownerWindow, bool vFlipped)
{
	static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

	auto material = ownerWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE);
	assert(material);

	struct QuadVertex
	{
		glm::vec2 aPos;
		glm::vec2 aTexCoords;
	};
	size_t vertexSize = sizeof(QuadVertex);

	// Create triangulated quad mesh to draw the layer on
	static QuadVertex x_vertices[] = {
		//        positions                texCoords
		{glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f)},
		{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 0.0f)},
		{glm::vec2(1.0f,  1.0f),  glm::vec2(1.0f, 1.0f)},
	};

	static QuadVertex x_flippedVertices[] = {
		//        positions                texCoords (flipped v coordinated)
		{glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f)},
		{glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 1.0f)},
		{glm::vec2(1.0f,  1.0f),  glm::vec2(1.0f, 0.0f)},
	};

	QuadVertex* vertices= vFlipped ? x_flippedVertices : x_vertices;
	auto fullscreenQuad =
		std::make_shared<GlTriangulatedMesh>(
			ownerWindow,
			"layer_quad_mesh",
			(const uint8_t*)vertices,
			vertexSize,
			4, // 4 verts
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			2, // 2 tris
			false); // mesh doesn't own quad vert data

	if (!fullscreenQuad->setMaterial(material) ||
		!fullscreenQuad->createResources())
	{
		MIKAN_LOG_ERROR("createFullscreenQuadMesh()")
			<< "Failed to create video frame render mesh";
	}

	return fullscreenQuad;
}

void drawTransformedTriangulatedMesh(
	IMkCameraConstPtr camera,
	const glm::mat4& transform,
	GlTriangulatedMeshConstPtr triangulatedMesh)
{
	if (camera != nullptr && triangulatedMesh != nullptr)
	{
		GlMaterialInstancePtr materialInstance = triangulatedMesh->getMaterialInstance();
		GlMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			const glm::mat4 vpMatrix = camera->getViewProjectionMatrix();

			materialInstance->setMat4BySemantic(eUniformSemantic::modelViewProjectionMatrix, vpMatrix * transform);

			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				triangulatedMesh->drawElements();
			}
		}
	}
}