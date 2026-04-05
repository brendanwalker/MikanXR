#pragma once

#include "imgui.h"

class MkGuiScopedStyleVar
{
public:
	MkGuiScopedStyleVar() = default;
	~MkGuiScopedStyleVar();

	MkGuiScopedStyleVar(const MkGuiScopedStyleVar&) = delete;
	MkGuiScopedStyleVar& operator=(const MkGuiScopedStyleVar&) = delete;

	MkGuiScopedStyleVar& push(ImGuiStyleVar idx, float val);
	MkGuiScopedStyleVar& push(ImGuiStyleVar idx, const ImVec2& val);

private:
	int m_count = 0;
};
