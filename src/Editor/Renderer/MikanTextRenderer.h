#pragma once

#include "MikanRendererFwd.h"
#include "TextStyle.h"

#include <vector>

#include "glm/ext/vector_float2.hpp"

//-- macros -----
#if defined(__clang__) || defined(__GNUC__)
#define GLYPH_PRINTFARGS(FMT) __attribute__((format(printf, FMT, (FMT+1))))
#else
#define GLYPH_PRINTFARGS(FMT)
#endif

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
void drawTextAtTrackerPosition(
	const TextStyle& style,
	const float trackerWidth, const float trackerHeight,
	const glm::vec2& trackerCoords,
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);