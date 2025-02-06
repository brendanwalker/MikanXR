#include "App.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlWireframeMesh.h"
#include "GlViewport.h"
#include "Logger.h"

GlWireframeMesh::GlWireframeMesh(IMkWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

GlWireframeMesh::GlWireframeMesh(
	IMkWindow* ownerWindow,
	std::string name,
	const uint8_t* vertexData,
	const size_t vertexSize,
	uint32_t vertexCount,
	const uint8_t* indexData,
	const size_t indexSize,
	uint32_t lineCount,
	bool bOwnsVertexData)
	: m_ownerWindow(ownerWindow)
{
	m_name = name;
	m_vertexData = vertexData;
	m_vertexSize = vertexSize;
	m_vertexCount = vertexCount;
	m_indexData = indexData;
	m_indexSize = indexSize;
	m_lineCount = lineCount;
	m_bOwnsVertexData = bOwnsVertexData;
}

GlWireframeMesh::~GlWireframeMesh()
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

void GlWireframeMesh::drawElements() const
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
	glDrawElements(GL_LINES, (int)m_lineCount * 2, indexType, nullptr);
	glBindVertexArray(0);
}

bool GlWireframeMesh::createResources()
{
	if (m_vertexData == nullptr || m_vertexCount == 0 ||
		m_indexData == nullptr || m_lineCount == 0)
	{
		return false;
	}

	GlShaderCache* shaderCache = getOwnerWindow()->getShaderCache();
	GlMaterialConstPtr material = shaderCache->getMaterialByName(INTERNAL_MATERIAL_P_WIREFRAME);
	if (!material)
	{
		return false;
	}

	const GlVertexDefinition& vertexDefinition = material->getProgram()->getVertexDefinition();
	const size_t vertexSize = vertexDefinition.getVertexSize();
	if (vertexSize != m_vertexSize)
	{
		return false;
	}

	// create a material instance from the default wireframe material
	m_materialInstance = std::make_shared<GlMaterialInstance>(material);
	m_materialInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(1.f));

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

void GlWireframeMesh::deleteResources()
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

void drawTransformedWireframeMesh(
	GlCameraConstPtr camera,
	const glm::mat4& transform,
	const GlWireframeMesh* wireframeMesh,
	const glm::vec3& color)
{
	if (camera != nullptr && wireframeMesh != nullptr)
	{
		GlMaterialInstancePtr materialInstance = wireframeMesh->getMaterialInstance();
		GlMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			const glm::mat4 vpMatrix = camera->getViewProjectionMatrix();

			materialInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(color, 1.f));
			materialInstance->setMat4BySemantic(eUniformSemantic::modelViewProjectionMatrix, vpMatrix * transform);
			
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				wireframeMesh->drawElements();
			}
		}
	}
}