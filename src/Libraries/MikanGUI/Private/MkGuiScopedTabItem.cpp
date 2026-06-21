#include "MkGuiScopedTabItem.h"

#include "imgui.h"

MkGuiScopedTabItem::MkGuiScopedTabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags)
{
	m_selected= ImGui::BeginTabItem(label, p_open, flags);
}

MkGuiScopedTabItem::~MkGuiScopedTabItem()
{
	if (m_selected)
		ImGui::EndTabItem();
}
