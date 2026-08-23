#pragma once

#include "MkGuiExport.h"
#include "imgui.h"

class MIKAN_GUI_CLASS MkGuiScopedTabBar
{
public:
	explicit MkGuiScopedTabBar(const char* str_id, ImGuiTabBarFlags flags= 0);
	~MkGuiScopedTabBar();

	MkGuiScopedTabBar(const MkGuiScopedTabBar&)= delete;
	MkGuiScopedTabBar& operator=(const MkGuiScopedTabBar&)= delete;

	explicit operator bool() const { return m_open; }

private:
	bool m_open= false;
};
