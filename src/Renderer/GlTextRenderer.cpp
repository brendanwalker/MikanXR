#include "App.h"
#include "CalibrationRenderHelpers.h"
#include "FontManager.h"
#include "GlCamera.h"
#include "GlCommon.h"
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

GlTextRenderer::GlTextRenderer()
{
}

void GlTextRenderer::render(IGlWindow* window)
{
	if (m_bakedTextQuads.size() > 0)
	{
		// Fetch the window resolution
		const float screenWidth = window->getWidth();
		const float screenHeight = window->getHeight();

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

		{
			GlScopedState stateScope = window->getGlStateStack().createScopedState();
			GlState& glState= stateScope.getStackState();

			glState
				.disableFlag(eGlStateFlagType::depthTest)
				.enableFlag(eGlStateFlagType::blend);

			// Turn on alpha blending for text rendering text
			glStateSetBlendFunc(glState, eGlBlendFunction::SRC_ALPHA, eGlBlendFunction::ONE_MINUS_SRC_ALPHA);

			// Render all of the baked quads
			for (const BakedTextQuad& bakedQuad : m_bakedTextQuads)
			{
				const float x = bakedQuad.screenCoords.x;
				const float y = bakedQuad.screenCoords.y;
				const float w = (float)bakedQuad.texture->getTextureWidth();
				const float h = (float)bakedQuad.texture->getTextureHeight();

				float xOffset = 0;
				switch (bakedQuad.horizontalAlignment)
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
		}

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
	GlTexture* texture= MainWindow::getInstance()->getFontManager()->fetchBakedText(style, text);

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
		remapPointIntoSubWindow(
			cameraWidth, cameraHeight,
			windowX0, windowY0,
			windowX1, windowY1,
			cameraCoords);

	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}