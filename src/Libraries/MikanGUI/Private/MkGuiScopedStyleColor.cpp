#include "MkGuiScopedStyleColor.h"

#include "imgui.h"

MkGuiScopedStyleColor::~MkGuiScopedStyleColor()
{
	if (m_count > 0)
		ImGui::PopStyleColor(m_count);
}

MkGuiScopedStyleColor& MkGuiScopedStyleColor::push(ImGuiCol idx, ImU32 col)
{
	ImGui::PushStyleColor(idx, col);
	++m_count;
	return *this;
}

MkGuiScopedStyleColor& MkGuiScopedStyleColor::push(ImGuiCol idx, const ImVec4& col)
{
	ImGui::PushStyleColor(idx, col);
	++m_count;
	return *this;
}
