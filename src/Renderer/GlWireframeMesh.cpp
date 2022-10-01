#include "App.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlShaderCache.h"
#include "GlProgram.h"
#include "GlWireframeMesh.h"
#include "Renderer.h"
#include "Logger.h"

GlWireframeMesh::GlWireframeMesh(
	std::string name,
	const uint8_t* vertexData,
	uint32_t vertexCount,
	const uint8_t* indexData,
	uint32_t lineCount,
	bool bOwnsVertexData)
{
	m_program = GlShaderCache::getInstance()->fetchCompiledGlProgram(getShaderCode());
	if (m_program == nullptr)
	{
		MIKAN_LOG_ERROR("GlWireframeMesh") << "Failed to build shader program";
	}

	m_name = name;
	m_vertexData = vertexData;
	m_vertexCount = vertexCount;
	m_indexData = indexData;
	m_lineCount = lineCount;
	m_bOwnsVertexData = bOwnsVertexData;
}

GlWireframeMesh::~GlWireframeMesh()
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

void GlWireframeMesh::drawElements() const
{
	glBindVertexArray(m_glVertArray);
	glDrawElements(GL_LINES, (int)m_lineCount * 2, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}

bool GlWireframeMesh::createBuffers()
{
	if (m_vertexData != nullptr && m_vertexCount > 0 &&
		m_indexData != nullptr && m_lineCount > 0)
	{
		const GlVertexDefinition* vertexDefinition= getVertexDefinition();
		const uint32_t vertexSize = vertexDefinition->vertexSize;

		// create and bind a Vertex Array Object(VAO) to hold state for this model
		glGenVertexArrays(1, &m_glVertArray);
		glBindVertexArray(m_glVertArray);

		// Populate a vertex buffer
		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertexSize * m_vertexCount, m_vertexData, GL_STATIC_DRAW);

		// Identify the components in the vertex buffer
		vertexDefinition->applyVertexDefintion();

		// Create and populate the index buffer
		glGenBuffers(1, &m_glIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_lineCount * 2, m_indexData, GL_STATIC_DRAW);

		glBindVertexArray(0);

		return true;
	}

	return false;
}

void GlWireframeMesh::deleteBuffers()
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

const GlProgramCode* GlWireframeMesh::getShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"wireframe shader",
		// vertex shader
		R""""(
		#version 410 
		uniform mat4 mvpMatrix; 
		uniform vec3 wireframeColor;
		layout(location = 0) in vec3 in_position; 
		out vec4 v_Color; 
		void main() 
		{ 
			gl_Position = mvpMatrix * vec4(in_position.xyz, 1); 
			v_Color = vec4(wireframeColor, 1.0);
		}
		)"""",
		//fragment shader
		R""""(
		#version 410 core
		in vec4 v_Color;
		out vec4 out_FragColor;
		void main()
		{
			out_FragColor = v_Color;
		}
		)"""")
		.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix)
		.addUniform("wireframeColor", eUniformSemantic::diffuseColorRGB);

	return &x_shaderCode;
}

const GlVertexDefinition* GlWireframeMesh::getVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize = (int32_t)sizeof(float)*3;
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position3f, false, vertexSize, 0));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return &x_vertexDefinition;
}

void drawTransformedWireframeMesh(
	const glm::mat4& transform,
	const GlWireframeMesh* wireframeMesh,
	const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());

	GlProgram* shader= wireframeMesh->getDefaultWireframeShader();

	shader->bindProgram();

	GlCamera* camera = renderer->getCurrentCamera();

	if (camera != nullptr)
	{
		const glm::mat4 vpMatrix = camera->getViewProjectionMatrix();

		shader->setMatrix4x4Uniform(eUniformSemantic::modelViewProjectionMatrix, vpMatrix * transform);
		shader->setVector3Uniform(eUniformSemantic::diffuseColorRGB, color);

		wireframeMesh->drawElements();
	}

	shader->unbindProgram();
}