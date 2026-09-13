#pragma once

#include "AssetFwd.h"

#include <string>

// The two ways the Assets panels draw an entry. A preview tile is a bordered
// frame carrying the asset's texture (or type glyph) over its name, for assets a
// picture tells apart. A list item is one glyph-and-name row in a multi-column
// list, for everything else, so many entries fit in the panel. Both leave the
// clickable item as the last ImGui item so the caller can attach a drag source
// or context menu to it.
namespace AssetTileGui
{
struct ItemResult
{
	bool bClicked= false;
	bool bDoubleClicked= false;
	bool bHovered= false;
};

// -- Preview tiles ----
constexpr float k_tileWidth= 120.f;

// The tile height for the current font, since the name row sits inside the frame
float getTileHeight();

// Opens the tile group and draws its bordered frame. bSelected tints the frame.
ItemResult beginTile(const std::string& idStr, bool bSelected);

// Draws the preview or glyph and the clipped name inside the frame, closes the
// group, and wraps to a new row when the next tile would not fit
void endTile(AssetReferencePtr assetRef, const std::string& displayName);

// -- List items ----
// Opens a multi-column list sized to the available width. Returns false when
// the list could not open, in which case no items may be drawn and endList is
// not called.
bool beginList(const char* id);

// Draws one glyph-and-name row in the next list cell. The row is the last ImGui
// item on return.
ItemResult drawListItem(const std::string& idStr, const char* glyph, const std::string& displayName, bool bSelected);

void endList();

// Shortens UTF-8 text to fit maxWidth in the current font, appending an ellipsis
std::string truncateTextWithEllipsis(const std::string& text, float maxWidth);
} // namespace AssetTileGui
