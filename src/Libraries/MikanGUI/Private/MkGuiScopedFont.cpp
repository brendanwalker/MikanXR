#include "MkGuiScopedFont.h"

#include "imgui.h"

MkGuiScopedFont::MkGuiScopedFont(ImFont* font, float size)
{
	ImGui::PushFont(font, size > 0.f ? size : (font ? font->LegacySize : 0.f));
}

MkGuiScopedFont::~MkGuiScopedFont() { ImGui::PopFont(); }
