#include "App.h"
#include "CalibrationRenderHelpers.h"
#include "FontManager.h"
#include "GlCamera.h"
#include "MkError.h"
#include "SdlCommon.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "MikanShaderCache.h"
#include "GlStateStack.h"
#include "GlStateModifiers.h"
#include "MikanTextRenderer.h"
#include "IMkTexture.h"
#include "GlViewport.h"
#include "IMkWindow.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MathGLM.h"

#include "glm/ext/matrix_projection.hpp"

//-- Drawing Methods -----
void drawTextAtWorldPosition(
	const TextStyle& style,
	const glm::vec3& position,
	const wchar_t* format,
	...)
{
	IMkWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	IMkTextRenderer * textRenderer = window->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	IMkCameraPtr camera = window->getRenderingViewport()->getCurrentCamera();
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

	IMkWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	IMkTextRenderer* textRenderer = window->getTextRenderer();
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
	IMkWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	IMkTextRenderer* textRenderer = window->getTextRenderer();
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

	IMkWindow* window = App::getInstance()->getCurrentlyRenderingWindow();
	assert(window != nullptr);

	IMkTextRenderer* textRenderer = window->getTextRenderer();
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