#include "MkError.h"
#include "GlCommon.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "IMkShader.h"
#include "IMkShaderCache.h"
#include "IMkState.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "IMkTextRenderer.h"
#include "IMkTexture.h"
#include "IMkViewport.h"
#include "IMkWindow.h"
#include "Logger.h"

#include "glm/ext/matrix_projection.hpp"

class MikanTextRenderer : public IMkTextRenderer
{
public:
	MikanTextRenderer() = delete;
	MikanTextRenderer(IMkWindow* ownerWindow, IMkFontManager* fontManager)
		: m_ownerWindow(ownerWindow)
		, m_fontManager(fontManager)
		, m_maxTextQuadVertexCount(kMaxTextQuads * 6) // 6 vertices per quad
		, m_textQuadVertices(new TextQuadVertex[m_maxTextQuadVertexCount])
	{}

	virtual ~MikanTextRenderer()
	{
		delete[] m_textQuadVertices;
	}

	virtual bool startup() override
	{
		m_textMaterial = m_ownerWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_TEXT);

		if (m_textMaterial == nullptr)
		{
			MIKAN_LOG_ERROR("MikanTextRenderer::startup") << "Failed to fetch text material";
			return false;
		}

		m_textMaterialInstance = std::make_shared<MkMaterialInstance>(m_textMaterial);

		glGenVertexArrays(1, &m_textQuadVAO);
		glGenBuffers(1, &m_textQuadVBO);
		checkHasAnyMkError("MikanTextRenderer::startup()", __FILE__, __LINE__);

		glBindVertexArray(m_textQuadVAO);
		glObjectLabel(GL_VERTEX_ARRAY, m_textQuadVAO, -1, "TextRendererQuads");
		glBindBuffer(GL_ARRAY_BUFFER, m_textQuadVBO);

		glBufferData(GL_ARRAY_BUFFER, m_maxTextQuadVertexCount * sizeof(TextQuadVertex), nullptr, GL_DYNAMIC_DRAW);
		checkHasAnyMkError("MikanTextRenderer::startup()", __FILE__, __LINE__);

		m_textMaterial->getProgram()->getVertexDefinition()->applyVertexDefintion();

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return true;
	}

	virtual void shutdown() override
	{
		if (m_textQuadVAO != 0)
		{
			glDeleteVertexArrays(1, &m_textQuadVAO);
		}

		if (m_textQuadVBO != 0)
		{
			glDeleteBuffers(1, &m_textQuadVBO);
		}

		m_textQuadVAO = 0;
		m_textQuadVBO = 0;
		m_textQuadVertexCount = 0;
	}

	virtual void render() override
	{
		if (m_textMaterial == nullptr)
			return;

		if (m_textQuadVertexCount == 0)
			return;

		// Same material used for all text quads
		if (auto materialBinding = m_textMaterial->bindMaterial())
		{
			MkScopedState stateScope = m_ownerWindow->getMkStateStack().createScopedState("MikanTextRenderer");
			IMkState* mkState = stateScope.getStackState();

			// Render text over top of everything with alpha blending
			mkState->disableFlag(eMkStateFlagType::depthTest);
			mkState->enableFlag(eMkStateFlagType::blend);
			mkStateSetBlendFunc(mkState, eMkBlendFunction::SRC_ALPHA, eMkBlendFunction::ONE_MINUS_SRC_ALPHA);

			// Bind the vertex array and buffer
			glBindVertexArray(m_textQuadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, m_textQuadVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_textQuadVertexCount * sizeof(TextQuadVertex), m_textQuadVertices);

			// Get the screen dimensions
			const float screenWidth = m_ownerWindow->getWidth();
			const float screenHeight = m_ownerWindow->getHeight();
			const glm::vec2 screenSize(screenWidth, screenHeight);

			// Draw all of the baked text quads (one unique texture per quad)
			for (auto& bakedTextQuad : m_bakedTextQuads)
			{
				// Bind the color texture
				m_textMaterialInstance->setTextureBySemantic(eUniformSemantic::rgbaTexture, bakedTextQuad.texture);
				m_textMaterialInstance->setVec2BySemantic(eUniformSemantic::screenSize, screenSize);

				// Draw the color texture
				if (auto materialInstanceBinding = m_textMaterialInstance->bindMaterialInstance(materialBinding))
				{
					// Draw the quad (two triangles)
					glDrawArrays(GL_TRIANGLES, bakedTextQuad.startVertexIndex, 6);
				}
			}

			// Unbind the vertex array and buffer
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			checkHasAnyMkError("GlLineRenderer::PointBufferState::drawGlBufferState", __FILE__, __LINE__);
		}

		m_bakedTextQuads.clear();
		m_textQuadVertexCount = 0;
	}

	virtual void addTextAtScreenPosition(
		const TextStyle& style,
		const glm::vec2& screenCoords,
		const std::wstring& text) override
	{
		BakedTextQuad bakedQuad;
		bakedQuad.texture = m_fontManager->fetchBakedText(style, text);
		bakedQuad.startVertexIndex = allocateTextQuadVertices(6);

		if (bakedQuad.texture != nullptr && bakedQuad.startVertexIndex != -1)
		{
			const float x = screenCoords.x;
			const float y = screenCoords.y;
			const float w = (float)bakedQuad.texture->getTextureWidth();
			const float h = (float)bakedQuad.texture->getTextureHeight();

			float xOffset = 0;
			switch (style.horizontalAlignment)
			{
				case eHorizontalTextAlignment::Left:
					xOffset = 0;
					break;
				case eHorizontalTextAlignment::Middle:
					xOffset = -w / 2;
					break;
				case eHorizontalTextAlignment::Right:
					xOffset = -w;
					break;
			}

			float yOffset = 0;
			switch (style.verticalAlignment)
			{
				case eVerticalTextAlignment::Top:
					yOffset = 0;
					break;
				case eVerticalTextAlignment::Middle:
					yOffset = -h / 2;
					break;
				case eVerticalTextAlignment::Bottom:
					yOffset = -h;
					break;
			}

			// Top Triangle
			setTextQuadVertex(bakedQuad.startVertexIndex + 0, glm::vec2(x + xOffset, y + yOffset), glm::vec2(0, 0));
			setTextQuadVertex(bakedQuad.startVertexIndex + 1, glm::vec2(x + w + xOffset, y + yOffset), glm::vec2(1, 0));
			setTextQuadVertex(bakedQuad.startVertexIndex + 2, glm::vec2(x + w + xOffset, y + h + yOffset), glm::vec2(1, 1));

			// Bottom Triangle
			setTextQuadVertex(bakedQuad.startVertexIndex + 3, glm::vec2(x + xOffset, y + yOffset), glm::vec2(0, 0));
			setTextQuadVertex(bakedQuad.startVertexIndex + 4, glm::vec2(x + w + xOffset, y + h + yOffset), glm::vec2(1, 1));
			setTextQuadVertex(bakedQuad.startVertexIndex + 5, glm::vec2(x + xOffset, y + h + yOffset), glm::vec2(0, 1));

			// Record the baked quad
			m_bakedTextQuads.push_back(bakedQuad);
		}
	}

protected:
	struct BakedTextQuad
	{
		IMkTexturePtr texture;
		int startVertexIndex;
	};

	struct TextQuadVertex
	{
		glm::vec2 position;
		glm::vec2 texCoords;
	};

	int allocateTextQuadVertices(int vertexCount)
	{
		if (m_textQuadVertexCount + vertexCount < m_maxTextQuadVertexCount)
		{
			int startVertexIndex = m_textQuadVertexCount;

			m_textQuadVertexCount += vertexCount;

			return startVertexIndex;
		}
		else
		{
			MIKAN_LOG_ERROR("MikanTextRenderer::allocateTextQuadVertices") << "Exceeded maximum text quad vertex count";

			return -1;
		}
	}

	void setTextQuadVertex(int index, const glm::vec2& position, const glm::vec2& texCoords)
	{
		if (index >= 0 && index < m_textQuadVertexCount)
		{
			m_textQuadVertices[index].position = position;
			m_textQuadVertices[index].texCoords = texCoords;
		}
	}

private:
	static const int kMaxTextQuads = 1024;
	IMkWindow* m_ownerWindow = nullptr;
	IMkFontManager* m_fontManager = nullptr;

	std::vector<BakedTextQuad> m_bakedTextQuads;
	unsigned int m_textQuadVAO = 0;
	unsigned int m_textQuadVBO = 0;
	int m_textQuadVertexCount = 0;
	int m_maxTextQuadVertexCount;
	TextQuadVertex* m_textQuadVertices;
	MkMaterialConstPtr m_textMaterial;
	MkMaterialInstancePtr m_textMaterialInstance;
};

IMkTextRendererPtr createMkTextRenderer(
	IMkWindow* ownerWindow,
	IMkFontManager* fontManager)
{
	return std::make_shared<MikanTextRenderer>(ownerWindow, fontManager);
}