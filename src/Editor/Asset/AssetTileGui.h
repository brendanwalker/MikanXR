#pragma once

#include "AssetFwd.h"

#include <string>

// The asset tile the Assets panels draw: a fixed frame carrying the asset's
// preview texture (or its type glyph) over its name. Split in two so the caller
// can attach a drag source or context menu to the frame, which is the last ImGui
// item when beginTile returns, before endTile draws the content over it.
namespace AssetTileGui
{
constexpr float k_tileWidth= 120.f;
constexpr float k_tileHeight= 140.f;

struct TileResult
{
	bool bClicked= false;
	bool bDoubleClicked= false;
	bool bHovered= false;
};

// Opens the tile group and draws its frame. bSelected tints the frame.
TileResult beginTile(const std::string& idStr, bool bSelected);

// Draws the preview or glyph and the clipped name, closes the group, and
// wraps to a new row when the next tile would not fit
void endTile(AssetReferencePtr assetRef, const std::string& displayName);

// Shortens UTF-8 text to fit maxWidth in the current font, appending an ellipsis
std::string truncateTextWithEllipsis(const std::string& text, float maxWidth);
} // namespace AssetTileGui
