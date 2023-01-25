#include "App.h"
#include "FontManager.h"
#include "GlCamera.h"
#include "GlCommon.h"
#include "GlStateStack.h"
#include "GlTextRenderer.h"
#include "GlTexture.h"
#include "Logger.h"
#include "MathGLM.h"
#include "Renderer.h"

#include "glm/ext/matrix_projection.hpp"

GlTextRenderer::GlTextRenderer()
{
}

void GlTextRenderer::render()
{
	if (m_bakedTextQuads.size() > 0)
	{
		// Fetch the window resolution
		Renderer* renderer = Renderer::getInstance();
		const float screenWidth = renderer->getSDLWindowWidth();
		const float screenHeight = renderer->getSDLWindowHeight();

		// Save a back up of the projection matrix and replace with an orthographic projection,
		// Where units = screen pixels, origin at top left
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		const glm::mat4 ortho_projection = glm::ortho(
			0.f, (float)screenWidth, // left, right
			(float)screenHeight, 0.f, // bottom, top
			-1.0f, 1.0f); // zNear, zFar
		glLoadMatrixf(glm::value_ptr(ortho_projection));

		// Save a backup of the modelview matrix and replace with the identity matrix
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		GLScopedState stateScope= renderer->getGlStateStack()->createScopedState();
		stateScope.getStackState()
			.disableFlag(eGlStateFlagType::depthTest);

		// Turn on alpha blending for text rendering text
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Render all of the baked quads
		for (const BakedTextQuad& bakedQuad : m_bakedTextQuads)
		{
			const float x = bakedQuad.screenCoords.x;
			const float y = bakedQuad.screenCoords.y;
			const float w = (float)bakedQuad.texture->getTextureWidth();
			const float h = (float)bakedQuad.texture->getTextureHeight();

			float xOffset= 0;
			switch (bakedQuad.horizontalAlignment)
			{
			case eHorizontalTextAlignment::Left:
				xOffset= 0;
				break;
			case eHorizontalTextAlignment::Middle:
				xOffset = -w/2;
				break;
			case eHorizontalTextAlignment::Right:
				xOffset = -w;
				break;
			}

			float yOffset = 0;
			switch (bakedQuad.verticalAlignment)
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

			bakedQuad.texture->bindTexture();

			glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex3d(x + xOffset, y + yOffset, 0);
			glTexCoord2d(1, 0); glVertex3d(x + w + xOffset, y + yOffset, 0);
			glTexCoord2d(1, 1); glVertex3d(x + w + xOffset, y + h + yOffset, 0);
			glTexCoord2d(0, 1); glVertex3d(x + xOffset, y + h + yOffset, 0);
			glEnd();

			bakedQuad.texture->clearTexture();
		}

		// Turn back off alpha blending
		//glDisable(GL_BLEND);

		// Restore the projection matrix
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		// Restore the modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	m_bakedTextQuads.clear();
}

void GlTextRenderer::addTextAtScreenPosition(
	const TextStyle& style,
	const glm::vec2& screenCoords, 
	const std::wstring& text)
{
	GlTexture* texture= App::getInstance()->getFontManager()->fetchBakedText(style, text);

	if (texture != nullptr)
	{
		m_bakedTextQuads.push_back({ 
			screenCoords, 
			texture, 
			style.horizontalAlignment, 
			style.verticalAlignment });
	}
}

//-- Drawing Methods -----
void drawTextAtWorldPosition(
	const TextStyle& style,
	const glm::vec3& position,
	const wchar_t* format,
	...)
{
	Renderer* renderer= Renderer::getInstance();
	GlTextRenderer* textRenderer= renderer->getTextRenderer();
	GlCamera* camera = renderer->getCurrentCamera();
	if (camera == nullptr)
		return;

	// Convert the world space coordinates into screen space
	const int screenWidth = (int)Renderer::getInstance()->getSDLWindowWidth();
	const int screenHeight = (int)Renderer::getInstance()->getSDLWindowHeight();
	glm::vec3 screenCoords =
		glm::project(
			position,
			camera->getModelViewMatrix(),
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

	Renderer* renderer = Renderer::getInstance();
	GlTextRenderer* textRenderer = renderer->getTextRenderer();
	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}