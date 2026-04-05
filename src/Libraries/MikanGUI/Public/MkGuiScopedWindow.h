#pragma once

#include "imgui.h"

class MkGuiScopedWindow
{
public:
	MkGuiScopedWindow(const char* name,
	                  bool* p_open = nullptr,
	                  ImGuiWindowFlags flags = 0);
	~MkGuiScopedWindow();

	MkGuiScopedWindow(const MkGuiScopedWindow&) = delete;
	MkGuiScopedWindow& operator=(const MkGuiScopedWindow&) = delete;

	bool isVisible() const { return m_visible; }
	explicit operator bool() const { return m_visible; }

private:
	bool m_visible = false;
};
