#include "MkGuiScopedStyleVar.h"

#include "imgui.h"

MkGuiScopedStyleVar::~MkGuiScopedStyleVar()
{
	if (m_count > 0)
		ImGui::PopStyleVar(m_count);
}

MkGuiScopedStyleVar& MkGuiScopedStyleVar::push(ImGuiStyleVar idx, float val)
{
	ImGui::PushStyleVar(idx, val);
	++m_count;
	return *this;
}

MkGuiScopedStyleVar& MkGuiScopedStyleVar::push(ImGuiStyleVar idx, const ImVec2& val)
{
	ImGui::PushStyleVar(idx, val);
	++m_count;
	return *this;
}
