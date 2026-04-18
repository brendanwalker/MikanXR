#include "CalibrationRenderHelpers.h"
#include "IMkGraphicsContext.h"
#include "IMkTextRenderer.h"
#include "MikanCamera.h"
#include "MkError.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "IMkShader.h"
#include "MikanShaderCache.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "MikanTextRenderer.h"
#include "IMkTexture.h"
#include "MikanViewport.h"
#include "IMkWindow.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MathGLM.h"

#include "glm/ext/matrix_projection.hpp"

#include <stdarg.h>

//-- Drawing Methods -----
void drawTextAtWorldPosition(
	IMkGraphicsContext* graphicsContext,
	const TextStyle& style,
	const glm::vec3& position,
	const wchar_t* format,
	...)
{
	IMkTextRenderer * textRenderer = graphicsContext->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	IMkCameraPtr camera = graphicsContext->getRenderingViewport()->getCurrentCamera();
	if (camera == nullptr)
		return;

	// Convert the world space coordinates into screen space
	const int screenWidth = (int)graphicsContext->getWidth();
	const int screenHeight = (int)graphicsContext->getHeight();
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
	IMkGraphicsContext* graphicsContext,
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

	IMkTextRenderer* textRenderer = graphicsContext->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	textRenderer->addTextAtScreenPosition(style, glm::vec2(screenCoords.x, screenCoords.y), text);
}

void drawTextAtTrackerPosition(
	IMkGraphicsContext* graphicsContext,
	const TextStyle& style,
	const float trackerWidth, const float trackerHeight,
	const glm::vec2& trackerCoords,
	const wchar_t* format,
	...)
{
	IMkTextRenderer* textRenderer = graphicsContext->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	wchar_t text[1024];
	va_list args;
	va_start(args, format);
	int w = vswprintf(text, sizeof(text), format, args);
	text[(sizeof(text) / sizeof(wchar_t)) - 1] = L'\0';
	va_end(args);

	// Convert the tracker space coordinates into screen space
	const float windowWidth = graphicsContext->getWidth();
	const float windowHeight = graphicsContext->getHeight();
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
	IMkGraphicsContext* graphicsContext,
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

	IMkTextRenderer* textRenderer = graphicsContext->getTextRenderer();
	if (textRenderer == nullptr)
		return;

	const float windowWidth = graphicsContext->getWidth();
	const float windowHeight = graphicsContext->getHeight();
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