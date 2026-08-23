#pragma once

#include "MkGuiExport.h"
#include "imgui.h"

class MIKAN_GUI_CLASS MkGuiScopedTabItem
{
public:
	explicit MkGuiScopedTabItem(const char* label, bool* p_open= nullptr, ImGuiTabItemFlags flags= 0);
	~MkGuiScopedTabItem();

	MkGuiScopedTabItem(const MkGuiScopedTabItem&)= delete;
	MkGuiScopedTabItem& operator=(const MkGuiScopedTabItem&)= delete;

	explicit operator bool() const { return m_selected; }

private:
	bool m_selected= false;
};
