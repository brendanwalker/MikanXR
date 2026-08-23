#include "MkGuiScopedPopup.h"

#include "imgui.h"

// -- MkGuiScopedPopup --

MkGuiScopedPopup::MkGuiScopedPopup(const char* str_id, ImGuiWindowFlags flags)
{
	m_open= ImGui::BeginPopup(str_id, flags);
}

MkGuiScopedPopup::~MkGuiScopedPopup()
{
	if (m_open)
		ImGui::EndPopup();
}

// -- MkGuiScopedPopupContextItem --

MkGuiScopedPopupContextItem::MkGuiScopedPopupContextItem(const char* str_id, ImGuiPopupFlags popup_flags)
{
	m_open= ImGui::BeginPopupContextItem(str_id, popup_flags);
}

MkGuiScopedPopupContextItem::~MkGuiScopedPopupContextItem()
{
	if (m_open)
		ImGui::EndPopup();
}
