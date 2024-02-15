#pragma once

#include "TextStyle.h"

#include <vector>

#include "glm/ext/vector_float2.hpp"

//-- macros -----
#if defined(__clang__) || defined(__GNUC__)
#define GLYPH_PRINTFARGS(FMT) __attribute__((format(printf, FMT, (FMT+1))))
#else
#define GLYPH_PRINTFARGS(FMT)
#endif

class GlTextRenderer
{
public:
	GlTextRenderer();

	void render(class IGlWindow* window);
	void addTextAtScreenPosition(const TextStyle& style, const glm::vec2& screenCoords, const std::wstring& text);

protected:
	struct BakedTextQuad
	{
		glm::vec2 screenCoords;
		class GlTexture* texture;
		eHorizontalTextAlignment horizontalAlignment;
		eVerticalTextAlignment verticalAlignment;
	};

	std::vector<BakedTextQuad> m_bakedTextQuads;
};

//-- drawing methods -----
void drawTextAtWorldPosition(
	const TextStyle& style, 
	const glm::vec3& position, 
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);
void drawTextAtScreenPosition(
	const TextStyle& style, 
	const glm::vec2& screenCoords, 
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);
void drawTextAtCameraPosition(
	const TextStyle& style,
	const float cameraWidth, const float cameraHeight,
	const glm::vec2& cameraCoords,
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);