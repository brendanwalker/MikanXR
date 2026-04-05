#pragma once

#include "imgui.h"

class MkGuiScopedChild
{
public:
	MkGuiScopedChild(const char* str_id,
	                 const ImVec2& size = ImVec2(0, 0),
	                 bool border = false,
	                 ImGuiWindowFlags flags = 0);
	MkGuiScopedChild(unsigned int id,
	                 const ImVec2& size = ImVec2(0, 0),
	                 bool border = false,
	                 ImGuiWindowFlags flags = 0);
	~MkGuiScopedChild();

	MkGuiScopedChild(const MkGuiScopedChild&) = delete;
	MkGuiScopedChild& operator=(const MkGuiScopedChild&) = delete;

	bool isVisible() const { return m_visible; }

private:
	bool m_visible = false;
};
