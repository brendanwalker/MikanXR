#pragma once

#include "MkGuiExport.h"

#include "imgui.h" // ImWchar

// The editor's ImGui look: a dark pastel palette over StyleColorsDark, rounded
// metrics, and the Mochiy Pop One UI font (with Japanese glyph coverage) with
// the ForkAwesome icon font merged in. Applied once per gui context, after
// ImGui::CreateContext and before the first frame.
namespace MkGuiTheme
{
MIKAN_GUI_FUNC(void) applyStyle();
// Returns the UI font, or nullptr when no font file could be loaded (ImGui's
// built-in font is used instead)
MIKAN_GUI_FUNC(ImFont*) loadFonts();
MIKAN_GUI_FUNC(const ImWchar*) getJapaneseGlyphRanges();
// The Japanese ranges plus the ForkAwesome icon range: every codepoint the UI
// font stack is expected to cover. Usable without a GL context.
MIKAN_GUI_FUNC(const ImWchar*) getUiGlyphRanges();
} // namespace MkGuiTheme
