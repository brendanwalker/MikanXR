#include "MkGuiScopedDragDropTarget.h"

#include "imgui.h"

MkGuiScopedDragDropTarget::MkGuiScopedDragDropTarget()
{
	m_active = ImGui::BeginDragDropTarget();
}

MkGuiScopedDragDropTarget::~MkGuiScopedDragDropTarget()
{
	if (m_active)
		ImGui::EndDragDropTarget();
}
