#pragma once

#include "MkGuiExport.h"
#include "imgui.h"

class MIKAN_GUI_CLASS MkGuiScopedStyleColor
{
public:
	MkGuiScopedStyleColor()= default;
	~MkGuiScopedStyleColor();

	MkGuiScopedStyleColor(const MkGuiScopedStyleColor&)= delete;
	MkGuiScopedStyleColor& operator=(const MkGuiScopedStyleColor&)= delete;

	MkGuiScopedStyleColor& push(ImGuiCol idx, ImU32 col);
	MkGuiScopedStyleColor& push(ImGuiCol idx, const ImVec4& col);

private:
	int m_count= 0;
};
