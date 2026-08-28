#include "MkError.h"
#include "GlCommon.h"
#include "IMkLineRenderer.h"
#include "IMkCamera.h"
#include "IMkShader.h"
#include "IMkState.h"
#include "MkStateStack.h"
#include "IMkShaderCache.h"
#include "IMkShaderCode.h"
#include "IMkVertexDefinition.h"
#include "IMkViewport.h"
#include "IMkGraphicsContext.h"
#include "Logger.h"

#include "glm/ext/matrix_clip_space.hpp"

const int k_max_segments= 0x8000;
const int k_max_points= 0x8000;

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
		{
		}

		~PointBufferState() { delete[] m_points; }

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

			program->getVertexDefinition()->applyVertexDefintion();

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void drawGlBufferState(unsigned int glEnumMode)
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

			m_pointCount= 0;
		}

		void destroyGlBufferState()
		{
			glDeleteVertexArrays(1, &m_pointVAO);
			glDeleteBuffers(1, &m_pointVBO);

			m_pointVAO= 0;
			m_pointVBO= 0;
			m_pointCount= 0;
		}

		inline bool hasPoints() const { return m_pointCount > 0; }

		void addPoint3d(const glm::mat4& xform, const glm::vec3& pos, const glm::vec3& color, float size)
		{
			if (m_pointCount < k_max_points)
			{
				const glm::vec3 xformedPos= glm::vec3(xform * glm::vec4(pos, 1.0f));

				m_points[m_pointCount]= {xformedPos, glm::vec4(color.r, color.g, color.b, size)};
				++m_pointCount;
			}
		}

		void addPoint2d(const glm::vec2& pos, const glm::vec3& color, float size)
		{
			if (m_pointCount < k_max_points)
			{
				const glm::vec3 pos3d= glm::vec3(pos.x, pos.y, 0.0f);

				m_points[m_pointCount]= {pos3d, glm::vec4(color.r, color.g, color.b, size)};
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

	static IMkShaderCodeConstPtr getPointShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode= nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode= createIMkShaderCode("point shader",
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
											  // fragment shader
											  R""""(
				#version 410 core
				in vec4 v_Color;
				out vec4 out_FragColor;
				void main()
				{
					out_FragColor = v_Color;
				}
				)"""");
			x_shaderCode->addVertexAttribute("in_position", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("in_colorPointSize", eVertexDataType::datatype_vec4,
											 eVertexSemantic::colorAndSize);
			x_shaderCode->addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);
		}

		return x_shaderCode;
	}

	static IMkShaderCodeConstPtr getLineShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode= nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode= createIMkShaderCode("line shader",
											  // vertex shader
											  R""""(
				#version 410
				uniform mat4 mvpMatrix;
				layout(location = 0) in vec3 in_position;
				layout(location = 1) in vec4 in_colorPointSize;
				out VertexData
				{
					vec3 color;
					float size;
				} v_out;
				void main()
				{
					gl_Position = mvpMatrix * vec4(in_position.xyz, 1);
					v_out.color = in_colorPointSize.xyz;
					v_out.size = in_colorPointSize.w;
				}
				)"""",
											  // geometry shader
											  // Expands each segment into a screen space quad so the per vertex
											  // size can select a line width in pixels (core profile wide lines
											  // are unreliable across drivers)
											  R""""(
				#version 410 core
				layout(lines) in;
				layout(triangle_strip, max_vertices = 4) out;
				uniform vec2 viewportSize;
				in VertexData
				{
					vec3 color;
					float size;
				} v_in[];
				out vec4 v_Color;
				void main()
				{
					vec4 p0 = gl_in[0].gl_Position;
					vec4 p1 = gl_in[1].gl_Position;

					// Clip against the near plane so an endpoint behind the camera
					// doesn't project to a flipped screen position
					const float wEpsilon = 0.0001;
					if (p0.w < wEpsilon && p1.w < wEpsilon)
						return;
					if (p0.w < wEpsilon)
						p0 = mix(p0, p1, (wEpsilon - p0.w) / (p1.w - p0.w));
					else if (p1.w < wEpsilon)
						p1 = mix(p1, p0, (wEpsilon - p1.w) / (p0.w - p1.w));

					vec2 s0 = p0.xy / p0.w;
					vec2 s1 = p1.xy / p1.w;

					// Unit direction in pixel space so the width is correct at any aspect ratio
					vec2 dirPx = (s1 - s0) * viewportSize;
					float lenPx = length(dirPx);
					dirPx = (lenPx > 0.0) ? dirPx / lenPx : vec2(1.0, 0.0);
					vec2 normalPx = vec2(-dirPx.y, dirPx.x);

					// Half width offset per endpoint: size is the total width in pixels
					vec2 offset0 = normalPx * v_in[0].size / viewportSize;
					vec2 offset1 = normalPx * v_in[1].size / viewportSize;

					v_Color = vec4(v_in[0].color, 1.0);
					gl_Position = vec4((s0 + offset0) * p0.w, p0.z, p0.w);
					EmitVertex();
					v_Color = vec4(v_in[0].color, 1.0);
					gl_Position = vec4((s0 - offset0) * p0.w, p0.z, p0.w);
					EmitVertex();
					v_Color = vec4(v_in[1].color, 1.0);
					gl_Position = vec4((s1 + offset1) * p1.w, p1.z, p1.w);
					EmitVertex();
					v_Color = vec4(v_in[1].color, 1.0);
					gl_Position = vec4((s1 - offset1) * p1.w, p1.z, p1.w);
					EmitVertex();
					EndPrimitive();
				}
				)"""",
											  // fragment shader
											  R""""(
				#version 410 core
				in vec4 v_Color;
				out vec4 out_FragColor;
				void main()
				{
					out_FragColor = v_Color;
				}
				)"""");
			x_shaderCode->addVertexAttribute("in_position", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("in_colorPointSize", eVertexDataType::datatype_vec4,
											 eVertexSemantic::colorAndSize);
			x_shaderCode->addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);
			x_shaderCode->addUniform("viewportSize", eUniformSemantic::screenSize);
		}

		return x_shaderCode;
	}

private:
	class IMkGraphicsContext* m_ownerContext= nullptr;

	IMkShaderPtr m_pointProgram= nullptr;
	IMkShaderPtr m_lineProgram= nullptr;
	std::string m_pointMvpUniformName;
	std::string m_lineMvpUniformName;
	std::string m_lineViewportUniformName;

	// 3d points and segments split by the depth test flag captured at queue time
	PointBufferState m_points3dDepth;
	PointBufferState m_lines3dDepth;
	PointBufferState m_points3dOverlay;
	PointBufferState m_lines3dOverlay;
	PointBufferState m_points2d;
	PointBufferState m_lines2d;

public:
	GlLineRenderer(IMkGraphicsContext* ownerContext)
		: m_ownerContext(ownerContext)
		, m_points3dDepth(k_max_points)
		, m_lines3dDepth(k_max_segments * 2)
		, m_points3dOverlay(k_max_points)
		, m_lines3dOverlay(k_max_segments * 2)
		, m_points2d(k_max_points)
		, m_lines2d(k_max_segments * 2)
	{
	}

	virtual ~GlLineRenderer()
	{
		m_pointProgram= nullptr;
		m_lineProgram= nullptr;
	}

	virtual bool startup() override
	{
		m_pointProgram= m_ownerContext->getShaderCache()->fetchCompiledIMkShader(getPointShaderCode());
		if (m_pointProgram == nullptr)
		{
			MIKAN_LOG_ERROR("GlLineRenderer::startup") << "Failed to build point shader program";
			return false;
		}

		m_lineProgram= m_ownerContext->getShaderCache()->fetchCompiledIMkShader(getLineShaderCode());
		if (m_lineProgram == nullptr)
		{
			MIKAN_LOG_ERROR("GlLineRenderer::startup") << "Failed to build line shader program";
			return false;
		}

		if (!m_pointProgram->getFirstUniformNameOfSemantic(eUniformSemantic::modelViewProjectionMatrix,
														   m_pointMvpUniformName)
			|| !m_lineProgram->getFirstUniformNameOfSemantic(eUniformSemantic::modelViewProjectionMatrix,
															 m_lineMvpUniformName))
		{
			MIKAN_LOG_ERROR("GlLineRenderer::startup") << "Failed to find model view projection uniform";
			return false;
		}

		if (!m_lineProgram->getFirstUniformNameOfSemantic(eUniformSemantic::screenSize, m_lineViewportUniformName))
		{
			MIKAN_LOG_ERROR("GlLineRenderer::startup") << "Failed to find viewport size uniform";
			return false;
		}

		m_points2d.createGlBufferState(m_pointProgram);
		m_lines2d.createGlBufferState(m_lineProgram);

		m_points3dDepth.createGlBufferState(m_pointProgram);
		m_lines3dDepth.createGlBufferState(m_lineProgram);
		m_points3dOverlay.createGlBufferState(m_pointProgram);
		m_lines3dOverlay.createGlBufferState(m_lineProgram);

		return true;
	}

	virtual void render(bool bDisable3dDepth) override
	{
		if (m_ownerContext == nullptr)
			return;

		const bool bAny3d= m_points3dDepth.hasPoints() || m_lines3dDepth.hasPoints() || m_points3dOverlay.hasPoints()
						   || m_lines3dOverlay.hasPoints();
		const bool bAny2d= m_points2d.hasPoints() || m_lines2d.hasPoints();

		if (!bAny3d && !bAny2d)
			return;

		MkScopedState stateScope= m_ownerContext->getMkStateStack().createScopedState("GlLineRenderer");
		IMkState* mkState= stateScope.getStackState();

		// This has to be enabled since the point drawing shader will use gl_PointSize.
		mkState->enableFlag(eMkStateFlagType::programPointSize);

		glm::i32vec2 renderingOrigin;
		glm::i32vec2 renderingSize;
		getRenderingViewportBounds(renderingOrigin, renderingSize);
		const glm::vec2 viewportSize= glm::vec2((float)renderingSize.x, (float)renderingSize.y);

		if (bAny3d)
		{
			IMkViewportConstPtr viewport= m_ownerContext->getRenderingViewport();
			IMkCameraPtr camera= (viewport != nullptr) ? viewport->getCurrentCamera() : nullptr;

			if (camera != nullptr)
			{
				const glm::mat4 cameraVPMatrix= camera->getViewProjectionMatrix();

				// The overlay batch draws first with the depth test off, so a depth
				// tested pass of the same geometry can overwrite it where it is
				// visible (how the gizmo dims only its occluded portion)
				{
					MkScopedState scopedState=
						m_ownerContext->getMkStateStack().createScopedState("GlLineRenderer_3dOverlay");
					scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

					drawPointsAndLines(m_points3dOverlay, m_lines3dOverlay, cameraVPMatrix, viewportSize);
				}

				{
					MkScopedState scopedState=
						m_ownerContext->getMkStateStack().createScopedState("GlLineRenderer_3dDepth");
					if (bDisable3dDepth)
					{
						scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);
					}
					else
					{
						scopedState.getStackState()->enableFlag(eMkStateFlagType::depthTest);
					}

					drawPointsAndLines(m_points3dDepth, m_lines3dDepth, cameraVPMatrix, viewportSize);
				}
			}
		}

		if (bAny2d)
		{
			const float left= renderingOrigin.x;
			const float right= renderingOrigin.x + renderingSize.x;
			const float top= renderingOrigin.y;
			const float bottom= renderingOrigin.y + renderingSize.y;
			const glm::mat4 orthoMat= glm::ortho(left, right, bottom, top, 1.0f, -1.0f);

			{
				// disable the depth buffer to allow overdraw
				MkScopedState scopedState=
					m_ownerContext->getMkStateStack().createScopedState("GlLineRenderer_2dLines");
				scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

				drawPointsAndLines(m_points2d, m_lines2d, orthoMat, viewportSize);
			}
		}
	}

	virtual void shutdown() override
	{
		m_points2d.destroyGlBufferState();
		m_lines2d.destroyGlBufferState();

		m_points3dDepth.destroyGlBufferState();
		m_lines3dDepth.destroyGlBufferState();
		m_points3dOverlay.destroyGlBufferState();
		m_lines3dOverlay.destroyGlBufferState();

		m_pointProgram= nullptr;
		m_lineProgram= nullptr;
	}

	virtual void addPoint3d(const glm::mat4& xform, const glm::vec3& pos, const glm::vec3& color, float size) override
	{
		point3dBufferForCurrentState().addPoint3d(xform, pos, color, size);
	}

	virtual void addSegment3d(const glm::mat4& xform, const glm::vec3& pos0, const glm::vec3& color0,
							  const glm::vec3& pos1, const glm::vec3& color1, float size0, float size1) override
	{
		PointBufferState& lineBuffer= line3dBufferForCurrentState();

		lineBuffer.addPoint3d(xform, pos0, color0, size0);
		lineBuffer.addPoint3d(xform, pos1, color1, size1);
	}

	virtual void addPoint2d(const glm::vec2& pos, const glm::vec3& color, float size) override
	{
		m_points2d.addPoint2d(pos, color, size);
	}

	virtual void addSegment2d(const glm::vec2& pos0, const glm::vec3& color0, const glm::vec2& pos1,
							  const glm::vec3& color1, float size0, float size1) override
	{
		m_lines2d.addPoint2d(pos0, color0, size0);
		m_lines2d.addPoint2d(pos1, color1, size1);
	}

private:
	// 3d primitives batch by the depth test flag in effect when they are queued,
	// so the MkScopedState wrapping the draw call decides how the batch renders
	bool isQueueTimeDepthTestEnabled() const
	{
		IMkState* mkState= m_ownerContext->getMkStateStack().getCurrentState();

		return mkState != nullptr && mkState->isFlagEnabled(eMkStateFlagType::depthTest);
	}

	PointBufferState& point3dBufferForCurrentState()
	{
		return isQueueTimeDepthTestEnabled() ? m_points3dDepth : m_points3dOverlay;
	}

	PointBufferState& line3dBufferForCurrentState()
	{
		return isQueueTimeDepthTestEnabled() ? m_lines3dDepth : m_lines3dOverlay;
	}

	void getRenderingViewportBounds(glm::i32vec2& outOrigin, glm::i32vec2& outSize) const
	{
		IMkViewportConstPtr viewport= m_ownerContext->getRenderingViewport();
		if (viewport == nullptr || !viewport->getRenderingViewport(outOrigin, outSize))
		{
			outOrigin= {0, 0};
			outSize= {(int)m_ownerContext->getWidth(), (int)m_ownerContext->getHeight()};
		}

		if (outSize.x <= 0)
			outSize.x= 1;
		if (outSize.y <= 0)
			outSize.y= 1;
	}

	void drawPointsAndLines(PointBufferState& points, PointBufferState& lines, const glm::mat4& mvpMatrix,
							const glm::vec2& viewportSize)
	{
		if (points.hasPoints())
		{
			m_pointProgram->bindProgram();
			m_pointProgram->setMatrix4x4Uniform(m_pointMvpUniformName, mvpMatrix);
			points.drawGlBufferState(GL_POINTS);
			m_pointProgram->unbindProgram();
		}

		if (lines.hasPoints())
		{
			m_lineProgram->bindProgram();
			m_lineProgram->setMatrix4x4Uniform(m_lineMvpUniformName, mvpMatrix);
			m_lineProgram->setVector2Uniform(m_lineViewportUniformName, viewportSize);
			lines.drawGlBufferState(GL_LINES);
			m_lineProgram->unbindProgram();
		}
	}
};

IMkLineRendererPtr createMkLineRenderer(IMkGraphicsContext* ownerContext)
{
	return std::make_shared<GlLineRenderer>(ownerContext);
}
