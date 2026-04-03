#include "MkGuiScopedWindow.h"

#include "imgui.h"

MkGuiScopedWindow::MkGuiScopedWindow(
	const char* name,
	bool* p_open,
	ImGuiWindowFlags flags)
{
	m_visible = ImGui::Begin(name, p_open, flags);
}

MkGuiScopedWindow::~MkGuiScopedWindow()
{
	ImGui::End();
}
