#include "MkGuiScopedTabBar.h"

#include "imgui.h"

MkGuiScopedTabBar::MkGuiScopedTabBar(const char* str_id, ImGuiTabBarFlags flags)
{
	m_open= ImGui::BeginTabBar(str_id, flags);
}

MkGuiScopedTabBar::~MkGuiScopedTabBar()
{
	if (m_open)
		ImGui::EndTabBar();
}
