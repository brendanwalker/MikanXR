#include "MkGuiScopedDragDropSource.h"

#include "imgui.h"

MkGuiScopedDragDropSource::MkGuiScopedDragDropSource(ImGuiDragDropFlags flags)
{
	m_active = ImGui::BeginDragDropSource(flags);
}

MkGuiScopedDragDropSource::~MkGuiScopedDragDropSource()
{
	if (m_active)
		ImGui::EndDragDropSource();
}
