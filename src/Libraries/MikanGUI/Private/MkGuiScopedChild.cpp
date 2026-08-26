#include "MkGuiScopedChild.h"

#include "imgui.h"

MkGuiScopedChild::MkGuiScopedChild(const char* str_id, const ImVec2& size, ImGuiChildFlags childFlags,
								   ImGuiWindowFlags flags)
{
	m_visible= ImGui::BeginChild(str_id, size, childFlags, flags);
}

MkGuiScopedChild::MkGuiScopedChild(unsigned int id, const ImVec2& size, ImGuiChildFlags childFlags,
								   ImGuiWindowFlags flags)
{
	m_visible= ImGui::BeginChild(static_cast<ImGuiID>(id), size, childFlags, flags);
}

MkGuiScopedChild::~MkGuiScopedChild() { ImGui::EndChild(); }
