#include "App.h"
#include "CalibrationRenderHelpers.h"
#include "FontManager.h"
#include "GlCamera.h"
#include "GlCommon.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlStateStack.h"
#include "GlStateModifiers.h"
#include "GlTextRenderer.h"
#include "GlTexture.h"
#include "GlViewport.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MathGLM.h"

#include "glm/ext/matrix_projection.hpp"

GlTextRenderer::GlTextRenderer(IGlWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
	, m_maxTextQuadVertexCount(kMaxTextQuads*6) // 6 vertices per quad
	, m_textQuadVertices(new TextQuadVertex[m_maxTextQuadVertexCount])
{
}

GlTextRenderer::~GlTextRenderer()
{
	delete[] m_textQuadVertices;
}

bool GlTextRenderer::startup()
{
	m_textMaterial = m_ownerWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_TEXT);

	if (m_textMaterial == nullptr)
	{
		MIKAN_LOG_ERROR("GlTextRenderer::startup") << "Failed to fetch text material";
		return false;
	}

	m_textMaterialInstance= std::make_shared<GlMaterialInstance>(m_textMaterial);

	glGenVertexArrays(1, &m_textQuadVAO);
	glGenBuffers(1, &m_textQuadVBO);
	checkHasAnyGLError("GlTextRenderer::startup()", __FILE__, __LINE__);

	glBindVertexArray(m_textQuadVAO);
	glObjectLabel(GL_VERTEX_ARRAY, m_textQuadVAO, -1, "TextRendererQuads");
	glBindBuffer(GL_ARRAY_BUFFER, m_textQuadVBO);

	glBufferData(GL_ARRAY_BUFFER, m_maxTextQuadVertexCount * sizeof(TextQuadVertex), nullptr, GL_DYNAMIC_DRAW);
	checkHasAnyGLError("GlTextRenderer::startup()", __FILE__, __LINE__);

	m_textMaterial->getProgram()->getVertexDefinition().applyVertexDefintion();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void GlTextRenderer::shutdown()
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

void GlTextRenderer::render()
{
	if (m_textMaterial == nullptr)
		return;

	if (m_textQuadVertexCount == 0)
		return;

	// Same material used for all text quads
	if (auto materialBinding = m_textMaterial->bindMaterial())
	{
		GlScopedState stateScope = m_ownerWindow->getGlStateStack().createScopedState("GlTextRenderer");
		GlState& glState= stateScope.getStackState();

		// Render text ove rtop of everything with alpha blending
		glState.disableFlag(eGlStateFlagType::depthTest);
		glState.enableFlag(eGlStateFlagType::blend);
		glStateSetBlendFunc(glState, eGlBlendFunction::SRC_ALPHA, eGlBlendFunction::ONE_MINUS_SRC_ALPHA);

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
		checkHasAnyGLError("GlLineRenderer::PointBufferState::drawGlBufferState", __FILE__, __LINE__);
	}

	m_bakedTextQuads.clear();
	m_textQuadVertexCount= 0;
}

int GlTextRenderer::allocateTextQuadVertices(int vertexCount)
{
	if (m_textQuadVertexCount + vertexCount < m_maxTextQuadVertexCount)
	{
		int startVertexIndex = m_textQuadVertexCount;

		m_textQuadVertexCount += vertexCount;

		return startVertexIndex;
	}
	else
	{
		MIKAN_LOG_ERROR("GlTextRenderer::allocateTextQuadVertices") << "Exceeded maximum text quad vertex count";

		return -1;
	}
}

void GlTextRenderer::setTextQuadVertex(int index, const glm::vec2& position, const glm::vec2& texCoords)
{
	if (index >= 0 && index < m_textQuadVertexCount)
	{
		m_textQuadVertices[index].position = position;
		m_textQuadVertices[index].texCoords = texCoords;
	}
}

void GlTextRenderer::addTextAtScreenPosition(
	const TextStyle& style,
	const glm::vec2& screenCoords, 
	const std::wstring& text)
{
	BakedTextQuad bakedQuad;
	bakedQuad.texture= MainWindow::getInstance()->getFontManager()->fetchBakedText(style, text);
	bakedQuad.startVertexIndex= allocateTextQuadVertices(6);

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

//-- Drawing Methods -----
void drawTextAtWorldPosition(
	const TextStyle& style,
	const glm::vec3& position,
	const wchar_t* format,
	...)
{
	IGlWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	GlTextRenderer * textRenderer = window->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	GlCameraPtr camera = window->getRenderingViewport()->getCurrentCamera();
	if (camera == nullptr)
		return;

	// Convert the world space coordinates into screen space
	const int screenWidth = (int)window->getWidth();
	const int screenHeight = (int)window->getHeight();
	glm::vec3 screenCoords =
		glm::project(
			position,
			camera->getViewMatrix(),
			camera->getProjectionMatrix(),
			glm::vec4(0, screenHeight, screenWidth, -screenHeight));

	// Bake out the text string
	wchar_t text[1024];
	va_list args;
	va_start(args, format);
	int w = vswprintf(text, sizeof(text), format, args);
	text[(sizeof(text) / sizeof(wchar_t)) - 1] = L'\0';
	va_end(args);

	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}

void drawTextAtScreenPosition(
	const TextStyle& style,
	const glm::vec2& screenCoords,
	const wchar_t* format,
	...)
{
	// Bake out the text string
	wchar_t text[1024];
	va_list args;
	va_start(args, format);
	int w = vswprintf(text, sizeof(text), format, args);
	text[(sizeof(text) / sizeof(wchar_t)) - 1] = L'\0';
	va_end(args);

	IGlWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	GlTextRenderer* textRenderer = window->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}

void drawTextAtTrackerPosition(
	const TextStyle& style,
	const float trackerWidth, const float trackerHeight,
	const glm::vec2& trackerCoords,
	const wchar_t* format,
	...)
{
	IGlWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	GlTextRenderer* textRenderer = window->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	wchar_t text[1024];
	va_list args;
	va_start(args, format);
	int w = vswprintf(text, sizeof(text), format, args);
	text[(sizeof(text) / sizeof(wchar_t)) - 1] = L'\0';
	va_end(args);

	// Convert the tracker space coordinates into screen space
	const float windowWidth = window->getWidth();
	const float windowHeight = window->getHeight();
	const float windowX0 = 0.0f, windowY0 = 0.f;
	const float windowX1 = windowWidth - 1.f, windowY1 = windowHeight - 1.f;
	glm::vec2 screenCoords =
		remapPointIntoTarget(
			trackerWidth, trackerHeight,
			windowX0, windowY0,
			windowX1, windowY1,
			trackerCoords);

	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}

void drawTextAtCameraPosition(
	const TextStyle& style,
	const float cameraWidth, const float cameraHeight,
	const glm::vec2& cameraCoords,
	const wchar_t* format,
	...)
{
	// Bake out the text string
	wchar_t text[1024];
	va_list args;
	va_start(args, format);
	int w = vswprintf(text, sizeof(text), format, args);
	text[(sizeof(text) / sizeof(wchar_t)) - 1] = L'\0';
	va_end(args);

	IGlWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	GlTextRenderer* textRenderer = window->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	const float windowWidth = window->getWidth();
	const float windowHeight = window->getHeight();
	const float windowX0 = 0.0f, windowY0 = 0.f;
	const float windowX1 = windowWidth - 1.f, windowY1 = windowHeight - 1.f;

	// Remaps the camera relative segment to window relative coordinates
	const glm::vec2 screenCoords =
		remapPointIntoTarget(
			cameraWidth, cameraHeight,
			windowX0, windowY0,
			windowX1, windowY1,
			cameraCoords);

	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}