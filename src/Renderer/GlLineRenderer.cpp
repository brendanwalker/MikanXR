#include "App.h"
#include "Colors.h"
#include "GlCamera.h"
#include "GlCommon.h"
#include "GlLineRenderer.h"
#include "GlProgram.h"
#include "GlStateStack.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "Logger.h"
#include "MathGLM.h"
#include "Renderer.h"

#include "glm/ext/matrix_clip_space.hpp"

const int k_max_segments = 0x8000;
const int k_max_points = 0x8000;

GlLineRenderer::GlLineRenderer()
	: m_program(nullptr)
	, m_points3d(k_max_points)
	, m_lines3d(k_max_segments*2)
	, m_points2d(k_max_points)
	, m_lines2d(k_max_segments * 2)
{
}

GlLineRenderer::~GlLineRenderer()
{
	m_program= nullptr;
}

const GlProgramCode* GlLineRenderer::getShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"line shader",
		// vertex shader
		R""""(
		#version 410 
		uniform mat4 mvpMatrix; 
		layout(location = 0) in vec3 in_position; 
		layout(location = 1) in vec4 in_colorPointSize; 
		out vec4 v_Color; 
		void main() 
		{ 
			gl_Position = mvpMatrix * vec4(in_position.xyz, 1); 
			gl_PointSize = in_colorPointSize.w;
			v_Color = vec4(in_colorPointSize.xyz, 1.0);
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
		.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);

	return &x_shaderCode;
}

const GlVertexDefinition* GlLineRenderer::getVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize = (int32_t)sizeof(GlLineRenderer::Point);
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position3f, false, vertexSize, offsetof(GlLineRenderer::Point, position)));
		attribs.push_back(GlVertexAttribute(1, eVertexSemantic::colorAndSize4f, false, vertexSize, offsetof(GlLineRenderer::Point, colorAndSize)));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return &x_vertexDefinition;
}

bool GlLineRenderer::startup()
{
	m_program = GlShaderCache::getInstance()->fetchCompiledGlProgram(getShaderCode());
	if (m_program == nullptr)
	{
		MIKAN_LOG_ERROR("GlLineRenderer::startup") << "Failed to build shader program";
		return false;
	}

	if (!m_program->getFirstUniformNameOfSemantic(eUniformSemantic::modelViewProjectionMatrix, m_modelViewUniformName))
	{
		MIKAN_LOG_ERROR("GlLineRenderer::startup") << "Failed to find model view projection uniform";
		return false;
	}

	m_points2d.createGlBufferState();
	m_lines2d.createGlBufferState();

	m_points3d.createGlBufferState();
	m_lines3d.createGlBufferState();

	return true;
}


void GlLineRenderer::render(Renderer* renderer)
{
	if (m_points3d.hasPoints() || m_lines3d.hasPoints() ||
		m_points2d.hasPoints() || m_lines2d.hasPoints())
	{
		m_program->bindProgram();

		if (m_points3d.hasPoints() || m_lines3d.hasPoints())
		{
			GlCamera* camera = renderer->getCurrentCamera();

			if (camera != nullptr)
			{
				const glm::mat4 cameraVPMatrix = camera->getViewProjectionMatrix();

				GlScopedState scopedState = renderer->getGlStateStack()->createScopedState();
				if (m_bDisable3dDepth)
				{
					scopedState.getStackState().disableFlag(eGlStateFlagType::depthTest);
				}

				m_program->setMatrix4x4Uniform(m_modelViewUniformName, cameraVPMatrix);

				m_points3d.drawGlBufferState(GL_POINTS);
				m_lines3d.drawGlBufferState(GL_LINES);
			}
		}

		if (m_points2d.hasPoints() || m_lines2d.hasPoints())
		{
			const float windowWidth = renderer->getSDLWindowWidth();
			const float windowHeight = renderer->getSDLWindowHeight();
			const glm::mat4 orthoMat = glm::ortho(0.f, windowWidth, windowHeight, 0.0f, 1.0f, -1.0f);

			m_program->setMatrix4x4Uniform(m_modelViewUniformName, orthoMat);

			{
				// disable the depth buffer to allow overdraw 
				GlScopedState scopedState = renderer->getGlStateStack()->createScopedState();
				scopedState.getStackState().disableFlag(eGlStateFlagType::depthTest);

				m_points2d.drawGlBufferState(GL_POINTS);
				m_lines2d.drawGlBufferState(GL_LINES);
			}
		}

		m_program->unbindProgram();
	}
}

void GlLineRenderer::shutdown()
{
	m_points2d.destroyGlBufferState();
	m_lines2d.destroyGlBufferState();

	m_points3d.destroyGlBufferState();
	m_lines3d.destroyGlBufferState();

	m_program= nullptr;
}

void GlLineRenderer::addPoint3d(
	const glm::mat4& xform,
	const glm::vec3& pos, 
	const glm::vec3& color,
	float size)
{
	m_points3d.addPoint3d(xform, pos, color, size);
}

void GlLineRenderer::addSegment3d(
	const glm::mat4& xform,
	const glm::vec3& pos0, const glm::vec3& color0,
	const glm::vec3& pos1, const glm::vec3& color1,
	float size0, float size1)
{
	m_lines3d.addPoint3d(xform, pos0, color0, size0);
	m_lines3d.addPoint3d(xform, pos1, color1, size1);
}

void GlLineRenderer::addPoint2d(
	const glm::vec2& pos, const glm::vec3& color,
	float size)
{
	m_points2d.addPoint2d(pos, color, size);
}

void GlLineRenderer::addSegment2d(
	const glm::vec2& pos0, const glm::vec3& color0,
	const glm::vec2& pos1, const glm::vec3& color1,
	float size0, float size1)
{
	m_lines2d.addPoint2d(pos0, color0, size0);
	m_lines2d.addPoint2d(pos1, color1, size1);
}

//-- Point Buffer State -----
GlLineRenderer::PointBufferState::PointBufferState(int maxPoints)
	: m_points(new Point[maxPoints])
	, m_maxPoints(maxPoints)
	, m_pointCount(0)
	, m_pointVAO(0)
	, m_pointVBO(0)
{
}

GlLineRenderer::PointBufferState::~PointBufferState()
{
	delete[] m_points;
}

void GlLineRenderer::PointBufferState::createGlBufferState()
{
	glGenVertexArrays(1, &m_pointVAO);
	glGenBuffers(1, &m_pointVBO);
	checkGLError(__FILE__, __LINE__);

	glBindVertexArray(m_pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);

	glBufferData(GL_ARRAY_BUFFER, m_maxPoints * sizeof(Point), nullptr, GL_DYNAMIC_DRAW);
	checkGLError(__FILE__, __LINE__);

	getVertexDefinition()->applyVertexDefintion();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GlLineRenderer::PointBufferState::drawGlBufferState(unsigned int glEnumMode)
{
	assert(m_points != nullptr);
	assert(m_pointCount <= k_max_points);
	if (m_pointCount > 0)
	{
		glBindVertexArray(m_pointVAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_pointCount * sizeof(Point), m_points);

		glDrawArrays(glEnumMode, 0, m_pointCount);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		checkGLError(__FILE__, __LINE__);
	}

	m_pointCount= 0;
}

void GlLineRenderer::PointBufferState::destroyGlBufferState()
{
	glDeleteVertexArrays(1, &m_pointVAO);
	glDeleteBuffers(1, &m_pointVBO);

	m_pointVAO = 0;
	m_pointVBO = 0;
	m_pointCount = 0;
}

void GlLineRenderer::PointBufferState::addPoint3d(
	const glm::mat4& xform,
	const glm::vec3& pos,
	const glm::vec3& color,
	float size)
{
	if (m_pointCount < k_max_points)
	{
		const glm::vec3 xformedPos = glm::vec3(xform * glm::vec4(pos, 1.0f));

		m_points[m_pointCount] = { xformedPos, glm::vec4(color.r, color.g, color.b, size) };
		++m_pointCount;
	}
}

void GlLineRenderer::PointBufferState::addPoint2d(
	const glm::vec2& pos,
	const glm::vec3& color,
	float size)
{
	if (m_pointCount < k_max_points)
	{
		const glm::vec3 pos3d = glm::vec3(pos.x, pos.y, 0.0f);

		m_points[m_pointCount] = { pos3d, glm::vec4(color.r, color.g, color.b, size) };
		++m_pointCount;
	}
}

//-- Drawing Methods -----
void drawPoint(
	const glm::mat4& transform, 
	const glm::vec3& point, 
	const glm::vec3& color,
	const float size)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	lineRenderer->addPoint3d(transform, point, color, size);
}

void drawSegment(
	const glm::mat4& transform,
	const glm::vec3& start,
	const glm::vec3& end,
	const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	lineRenderer->addSegment3d(transform, start, color, end, color);
}

void drawSegment(const glm::mat4& transform,
				 const glm::vec3& start, const glm::vec3& end,
				 const glm::vec3& colorStart, const glm::vec3& colorEnd)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	lineRenderer->addSegment3d(transform, start, colorStart, end, colorEnd);
}

void drawArrow(
	const glm::mat4& transform,
	const glm::vec3& start,
	const glm::vec3& end,
	const float headFraction,
	const glm::vec3& color)
{
	Renderer* renderer= Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	const glm::vec3 headAxis = end - start;
	const float headSize = headAxis.length() * headFraction * 0.1f;
	const glm::vec3 headOrigin = glm_vec3_lerp(end, start, headFraction);

	const glm::vec3 worldUp = glm::vec3(0, 1, 0);
	const glm::vec3 headForward = glm::normalize(headAxis);
	const glm::vec3 headLeft = glm::normalize(glm::cross(worldUp, headForward));
	const glm::vec3 headUp = glm::normalize(glm::cross(headForward, headLeft));

	const glm::vec3 headXPos = headOrigin - headLeft * headSize;
	const glm::vec3 headXNeg = headOrigin + headLeft * headSize;
	const glm::vec3 headYPos = headOrigin + headUp * headSize;
	const glm::vec3 headYNeg = headOrigin - headUp * headSize;

	lineRenderer->addSegment3d(transform, start, color, end, color);
	
	lineRenderer->addSegment3d(transform, headXPos, color, headYPos, color);
	lineRenderer->addSegment3d(transform, headYPos, color, headXNeg, color);
	lineRenderer->addSegment3d(transform, headXNeg, color, headYNeg, color);
	lineRenderer->addSegment3d(transform, headYNeg, color, headXPos, color);

	lineRenderer->addSegment3d(transform, headXPos, color, end, color);
	lineRenderer->addSegment3d(transform, headYPos, color, end, color);
	lineRenderer->addSegment3d(transform, headXNeg, color, end, color);
	lineRenderer->addSegment3d(transform, headYNeg, color, end, color);

	lineRenderer->addSegment3d(transform, headXPos, color, headXNeg, color);
	lineRenderer->addSegment3d(transform, headYPos, color, headYNeg, color);
}

void drawTransformedAxes(const glm::mat4& transform, float scale)
{
	drawTransformedAxes(transform, scale, scale, scale);
}

void drawTransformedAxes(const glm::mat4& transform, float xScale, float yScale, float zScale)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	glm::vec3 origin(0.f, 0.f, 0.f);
	glm::vec3 xAxis(xScale, 0.f, 0.f);
	glm::vec3 yAxis(0.f, yScale, 0.f);
	glm::vec3 zAxis(0.f, 0.f, zScale);

	lineRenderer->addSegment3d(transform, origin, Colors::Red, xAxis, Colors::Red);
	lineRenderer->addSegment3d(transform, origin, Colors::Green, yAxis, Colors::Green);
	lineRenderer->addSegment3d(transform, origin, Colors::Blue, zAxis, Colors::Blue);
}

void drawGrid(const glm::mat4& transform, float xSize, float zSize, int xSubDiv, int zSubDiv, const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	int x0 = -xSize / 2.f;
	int x1 = xSize / 2.f;
	for (float z= -zSize/2.f; z <= zSize/2.f; z+=(zSize/(float)zSubDiv))
	{
		lineRenderer->addSegment3d(transform, glm::vec3(x0, 0.f, z), color, glm::vec3(x1, 0.f, z), color);
	}

	int z0 = -zSize / 2.f;
	int z1 = zSize / 2.f;
	for (float x = -xSize / 2.f; x <= xSize / 2.f; x += (xSize / (float)xSubDiv))
	{
		lineRenderer->addSegment3d(transform, glm::vec3(x, 0.f, z0), color, glm::vec3(x, 0.f, z1), color);
	}
}

void drawTransformedQuad(const glm::mat4& transform, float xSize, float ySize, const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	const glm::vec3 p0(xSize / 2.f, ySize / 2.f, 0.f);
	const glm::vec3 p1(xSize / 2.f, -ySize / 2.f, 0.f);
	const glm::vec3 p2(-xSize / 2.f, -ySize / 2.f, 0.f);
	const glm::vec3 p3(-xSize / 2.f, ySize / 2.f, 0.f);

	lineRenderer->addSegment3d(transform, p0, color, p1, color);
	lineRenderer->addSegment3d(transform, p1, color, p2, color);
	lineRenderer->addSegment3d(transform, p2, color, p3, color);
	lineRenderer->addSegment3d(transform, p3, color, p0, color);	
}

void drawTransformedBox(const glm::mat4& transform, const glm::vec3& half_extents, const glm::vec3& color)
{
	drawTransformedBox(transform, -half_extents, half_extents, color);
}

void drawTransformedBox(const glm::mat4& transform, const glm::vec3& box_min, const glm::vec3& box_max, const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	const glm::vec3 v0(box_max.x, box_max.y, box_max.z);
	const glm::vec3 v1(box_min.x, box_max.y, box_max.z);
	const glm::vec3 v2(box_min.x, box_max.y, box_min.z);
	const glm::vec3 v3(box_max.x, box_max.y, box_min.z);
	const glm::vec3 v4(box_max.x, box_min.y, box_max.z);
	const glm::vec3 v5(box_min.x, box_min.y, box_max.z);
	const glm::vec3 v6(box_min.x, box_min.y, box_min.z);
	const glm::vec3 v7(box_max.x, box_min.y, box_min.z);

	lineRenderer->addSegment3d(transform, v0, color, v1, color);
	lineRenderer->addSegment3d(transform, v1, color, v2, color);
	lineRenderer->addSegment3d(transform, v2, color, v3, color);
	lineRenderer->addSegment3d(transform, v3, color, v0, color);

	lineRenderer->addSegment3d(transform, v4, color, v5, color);
	lineRenderer->addSegment3d(transform, v5, color, v6, color);
	lineRenderer->addSegment3d(transform, v6, color, v7, color);
	lineRenderer->addSegment3d(transform, v7, color, v4, color);

	lineRenderer->addSegment3d(transform, v0, color, v4, color);
	lineRenderer->addSegment3d(transform, v1, color, v5, color);
	lineRenderer->addSegment3d(transform, v2, color, v6, color);
	lineRenderer->addSegment3d(transform, v3, color, v7, color);
}

void drawTransformedFrustum(
	const glm::mat4& transform, 
	const float hfov_radians, 
	const float vfov_radians,
	const float zNear,
	const float zFar,
	const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	const float HRatio = tanf(hfov_radians / 2.f);
	const float VRatio = tanf(vfov_radians / 2.f);

	const glm::vec3 cameraRight(1.f, 0.f, 0.f);
	const glm::vec3 cameraUp(0.f, 1.f, 0.f);
	const glm::vec3 cameraForward(0.f, 0.f, -1.f);
	const glm::vec3 cameraOrigin(0.f);

	const glm::vec3 nearX = cameraRight * zNear * HRatio;
	const glm::vec3 farX = cameraRight * zFar * HRatio;

	const glm::vec3 nearY = cameraUp * zNear * VRatio;
	const glm::vec3 farY = cameraUp * zFar * VRatio;

	const glm::vec3 nearZ = cameraForward * zNear;
	const glm::vec3 farZ = cameraForward * zFar;

	const glm::vec3 nearCenter = cameraOrigin + nearZ;
	const glm::vec3 near0 = cameraOrigin + nearX + nearY + nearZ;
	const glm::vec3 near1 = cameraOrigin - nearX + nearY + nearZ;
	const glm::vec3 near2 = cameraOrigin - nearX - nearY + nearZ;
	const glm::vec3 near3 = cameraOrigin + nearX - nearY + nearZ;

	const glm::vec3 far0 = cameraOrigin + farX + farY + farZ;
	const glm::vec3 far1 = cameraOrigin - farX + farY + farZ;
	const glm::vec3 far2 = cameraOrigin - farX - farY + farZ;
	const glm::vec3 far3 = cameraOrigin + farX - farY + farZ;

	lineRenderer->addSegment3d(transform, near0, color, near1, color);
	lineRenderer->addSegment3d(transform, near1, color, near2, color);
	lineRenderer->addSegment3d(transform, near2, color, near3, color);
	lineRenderer->addSegment3d(transform, near3, color, near0, color);

	lineRenderer->addSegment3d(transform, far0, color, far1, color);
	lineRenderer->addSegment3d(transform, far1, color, far2, color);
	lineRenderer->addSegment3d(transform, far2, color, far3, color);
	lineRenderer->addSegment3d(transform, far3, color, far0, color);

	lineRenderer->addSegment3d(transform, cameraOrigin, color, far0, color);
	lineRenderer->addSegment3d(transform, cameraOrigin, color, far1, color);
	lineRenderer->addSegment3d(transform, cameraOrigin, color, far2, color);
	lineRenderer->addSegment3d(transform, cameraOrigin, color, far3, color);

	lineRenderer->addSegment3d(transform, cameraOrigin, color, nearCenter, color);
}