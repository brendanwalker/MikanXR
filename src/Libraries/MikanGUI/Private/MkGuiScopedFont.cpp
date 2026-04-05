#include "MkGuiScopedFont.h"

#include "imgui.h"

MkGuiScopedFont::MkGuiScopedFont(ImFont* font)
{
	ImGui::PushFont(font);
}

MkGuiScopedFont::~MkGuiScopedFont()
{
	ImGui::PopFont();
}
