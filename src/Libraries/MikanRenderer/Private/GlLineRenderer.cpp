#include "MkError.h"
#include "GlCommon.h"
#include "IMkLineRenderer.h"
#include "IMkCamera.h"
#include "IMkShader.h"
#include "GlStateStack.h"
#include "IMkShaderCache.h"
#include "IMkVertexDefinition.h"
#include "IMkViewport.h"
#include "IMkWindow.h"
#include "Logger.h"

#include "glm/ext/matrix_clip_space.hpp"

const int k_max_segments = 0x8000;
const int k_max_points = 0x8000;

class GlLineRenderer : public IMkLineRenderer
{
protected:
	struct Point
	{
		glm::vec3 position;
		glm::vec4 colorAndSize;
	};

	class PointBufferState
	{
	public:
		PointBufferState(int maxPoints)
			: m_points(new Point[maxPoints])
			, m_maxPoints(maxPoints)
			, m_pointCount(0)
			, m_pointVAO(0)
			, m_pointVBO(0)
		{}

		~PointBufferState()
		{
			delete[] m_points;
		}

		void createGlBufferState(IMkShaderPtr program)
		{
			glGenVertexArrays(1, &m_pointVAO);
			glGenBuffers(1, &m_pointVBO);
			checkHasAnyMkError("GlLineRenderer::PointBufferState::createGlBufferState()", __FILE__, __LINE__);

			glBindVertexArray(m_pointVAO);
			glObjectLabel(GL_VERTEX_ARRAY, m_pointVAO, -1, "LineRendererPoints");
			glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);

			glBufferData(GL_ARRAY_BUFFER, m_maxPoints * sizeof(Point), nullptr, GL_DYNAMIC_DRAW);
			checkHasAnyMkError("GlLineRenderer::PointBufferState::createGlBufferState()", __FILE__, __LINE__);

			program->getVertexDefinition().applyVertexDefintion();

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void PointBufferState::drawGlBufferState(unsigned int glEnumMode)
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
				checkHasAnyMkError("GlLineRenderer::PointBufferState::drawGlBufferState", __FILE__, __LINE__);
			}

			m_pointCount = 0;
		}

		void destroyGlBufferState()
		{
			glDeleteVertexArrays(1, &m_pointVAO);
			glDeleteBuffers(1, &m_pointVBO);

			m_pointVAO = 0;
			m_pointVBO = 0;
			m_pointCount = 0;
		}

		inline bool hasPoints() const { return m_pointCount > 0; }

		void addPoint3d(
			const glm::mat4& xform,
			const glm::vec3& pos,
			const glm::vec3& color,
			float size)
		{
			if (m_pointCount < k_max_points)
			{
				const glm::vec3 xformedPos = glm::vec3(xform * glm::vec4(pos, 1.0f));

				m_points[m_pointCount] = {xformedPos, glm::vec4(color.r, color.g, color.b, size)};
				++m_pointCount;
			}
		}

		void addPoint2d(
			const glm::vec2& pos,
			const glm::vec3& color,
			float size)
		{
			if (m_pointCount < k_max_points)
			{
				const glm::vec3 pos3d = glm::vec3(pos.x, pos.y, 0.0f);

				m_points[m_pointCount] = {pos3d, glm::vec4(color.r, color.g, color.b, size)};
				++m_pointCount;
			}
		}

	private:
		Point* m_points;
		int m_maxPoints;
		int m_pointCount;
		unsigned int m_pointVAO;
		unsigned int m_pointVBO;
	};

	static const IMkShaderCode* getShaderCode()
	{
		static IMkShaderCode x_shaderCode = IMkShaderCode(
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
			.addVertexAttribute("in_position", eVertexDataType::datatype_vec3, eVertexSemantic::position)
			.addVertexAttribute("in_colorPointSize", eVertexDataType::datatype_vec4, eVertexSemantic::colorAndSize)
			.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);

		return &x_shaderCode;
	}

private:
	class IMkWindow* m_ownerWindow = nullptr;

	IMkShaderPtr m_program = nullptr;
	std::string m_modelViewUniformName;

	PointBufferState m_points3d;
	PointBufferState m_lines3d;
	PointBufferState m_points2d;
	PointBufferState m_lines2d;

	bool m_bDisable3dDepth = false;

public:
	GlLineRenderer(IMkWindow* m_ownerWindow)
		: m_ownerWindow(m_ownerWindow)
		, m_program(nullptr)
		, m_points3d(k_max_points)
		, m_lines3d(k_max_segments * 2)
		, m_points2d(k_max_points)
		, m_lines2d(k_max_segments * 2)
	{}

	virtual ~GlLineRenderer()
	{
		m_program = nullptr;
	}

	virtual bool startup() override
	{
		m_program = m_ownerWindow->getShaderCache()->fetchCompiledIMkShader(getShaderCode());
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

		m_points2d.createGlBufferState(m_program);
		m_lines2d.createGlBufferState(m_program);

		m_points3d.createGlBufferState(m_program);
		m_lines3d.createGlBufferState(m_program);

		return true;
	}


	virtual void render() override
	{
		if (m_ownerWindow == nullptr)
			return;

		if (m_points3d.hasPoints() || m_lines3d.hasPoints() ||
			m_points2d.hasPoints() || m_lines2d.hasPoints())
		{
			GlScopedState stateScope = m_ownerWindow->getGlStateStack().createScopedState("GlLineRenderer");
			GlState& glState = stateScope.getStackState();

			// This has to be enabled since the point drawing shader will use gl_PointSize.
			glState.enableFlag(eGlStateFlagType::programPointSize);

			m_program->bindProgram();

			if (m_points3d.hasPoints() || m_lines3d.hasPoints())
			{
				IMkViewportConstPtr viewport = m_ownerWindow->getRenderingViewport();
				IMkCameraPtr camera = (viewport != nullptr) ? viewport->getCurrentCamera() : nullptr;

				if (camera != nullptr)
				{
					const glm::mat4 cameraVPMatrix = camera->getViewProjectionMatrix();

					GlScopedState scopedState = m_ownerWindow->getGlStateStack().createScopedState("GlLineRenderer_3dLines");
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
				float left = 0;
				float right = 0;
				float top = 0;
				float bottom = 0;

				glm::i32vec2 renderingOrigin;
				glm::i32vec2 renderingSize;
				IMkViewportConstPtr viewport = m_ownerWindow->getRenderingViewport();
				if (viewport != nullptr &&
					viewport->getRenderingViewport(renderingOrigin, renderingSize))
				{
					left = renderingOrigin.x;
					right = renderingOrigin.x + renderingSize.x;
					top = renderingOrigin.y;
					bottom = renderingOrigin.y + renderingSize.y;
				}
				else
				{
					left = 0;
					right = m_ownerWindow->getWidth();
					top = 0;
					bottom = m_ownerWindow->getHeight();
				}

				const glm::mat4 orthoMat = glm::ortho(left, right, bottom, top, 1.0f, -1.0f);

				m_program->setMatrix4x4Uniform(m_modelViewUniformName, orthoMat);

				{
					// disable the depth buffer to allow overdraw 
					GlScopedState scopedState = m_ownerWindow->getGlStateStack().createScopedState("GlLineRenderer_2dLines");
					scopedState.getStackState().disableFlag(eGlStateFlagType::depthTest);

					m_points2d.drawGlBufferState(GL_POINTS);
					m_lines2d.drawGlBufferState(GL_LINES);
				}
			}

			m_program->unbindProgram();
		}
	}

	virtual void shutdown() override
	{
		m_points2d.destroyGlBufferState();
		m_lines2d.destroyGlBufferState();

		m_points3d.destroyGlBufferState();
		m_lines3d.destroyGlBufferState();

		m_program = nullptr;
	}

	virtual void setDisable3dDepth(bool bFlag) override
	{
		m_bDisable3dDepth = bFlag;
	}

	virtual void addPoint3d(
		const glm::mat4& xform,
		const glm::vec3& pos,
		const glm::vec3& color,
		float size) override
	{
		m_points3d.addPoint3d(xform, pos, color, size);
	}

	virtual void addSegment3d(
		const glm::mat4& xform,
		const glm::vec3& pos0, const glm::vec3& color0,
		const glm::vec3& pos1, const glm::vec3& color1,
		float size0, float size1) override
	{
		m_lines3d.addPoint3d(xform, pos0, color0, size0);
		m_lines3d.addPoint3d(xform, pos1, color1, size1);
	}

	virtual void addPoint2d(
		const glm::vec2& pos, const glm::vec3& color,
		float size) override
	{
		m_points2d.addPoint2d(pos, color, size);
	}

	virtual void addSegment2d(
		const glm::vec2& pos0, const glm::vec3& color0,
		const glm::vec2& pos1, const glm::vec3& color1,
		float size0, float size1) override
	{
		m_lines2d.addPoint2d(pos0, color0, size0);
		m_lines2d.addPoint2d(pos1, color1, size1);
	}
};

IMkLineRendererPtr createMkLineRenderer(IMkWindow* ownerWindow)
{
	return std::make_shared<GlLineRenderer>(ownerWindow);
}