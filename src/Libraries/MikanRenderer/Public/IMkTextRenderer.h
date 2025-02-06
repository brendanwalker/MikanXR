#pragma once

#include "MkRendererFwd.h"

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"

#include <string>

#define TEXT_STYLE_NORMAL        0x00
#define TEXT_STYLE_BOLD          0x01
#define TEXT_STYLE_ITALIC        0x02
#define TEXT_STYLE_UNDERLINE     0x04
#define TEXT_STYLE_STRIKETHROUGH 0x08

enum class eHorizontalTextAlignment : int
{
	Left,
	Middle,
	Right
};

enum class eVerticalTextAlignment : int
{
	Top,
	Middle,
	Bottom
};

struct TextStyle
{
	std::string fontName;
	int pointSize;
	unsigned int styleBitmask; // TEXT_STYLE_BOLD | TEXT_STYLE_ITALIC | ...
	eHorizontalTextAlignment horizontalAlignment;
	eVerticalTextAlignment verticalAlignment;
	glm::vec3 color;
};

class IMkTextRenderer
{
public:
	virtual ~IMkTextRenderer() {}

	virtual bool startup() = 0;
	virtual void render() = 0;
	virtual void shutdown() = 0;

	virtual void addTextAtScreenPosition(
		const TextStyle& style, 
		const glm::vec2& screenCoords, 
		const std::wstring& text) = 0;
};