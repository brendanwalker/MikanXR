#pragma once

#include "imgui.h"

class MkGuiScopedPopup
{
public:
	explicit MkGuiScopedPopup(const char* str_id, ImGuiWindowFlags flags = 0);
	~MkGuiScopedPopup();

	MkGuiScopedPopup(const MkGuiScopedPopup&) = delete;
	MkGuiScopedPopup& operator=(const MkGuiScopedPopup&) = delete;

	explicit operator bool() const { return m_open; }

private:
	bool m_open = false;
};

class MkGuiScopedPopupContextItem
{
public:
	explicit MkGuiScopedPopupContextItem(const char* str_id = nullptr,
	                                     ImGuiPopupFlags popup_flags = 1);
	~MkGuiScopedPopupContextItem();

	MkGuiScopedPopupContextItem(const MkGuiScopedPopupContextItem&) = delete;
	MkGuiScopedPopupContextItem& operator=(const MkGuiScopedPopupContextItem&) = delete;

	explicit operator bool() const { return m_open; }

private:
	bool m_open = false;
};
