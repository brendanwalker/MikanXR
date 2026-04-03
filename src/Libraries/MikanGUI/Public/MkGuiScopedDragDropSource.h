#pragma once

#include "imgui.h"

class MkGuiScopedDragDropSource
{
public:
	explicit MkGuiScopedDragDropSource(ImGuiDragDropFlags flags = 0);
	~MkGuiScopedDragDropSource();

	MkGuiScopedDragDropSource(const MkGuiScopedDragDropSource&) = delete;
	MkGuiScopedDragDropSource& operator=(const MkGuiScopedDragDropSource&) = delete;

	explicit operator bool() const { return m_active; }

private:
	bool m_active = false;
};
