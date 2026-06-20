#pragma once

#include "MkGuiExport.h"
#include "IMkGuiStyle.h"

class MIKAN_GUI_CLASS MkGuiScopedStyle
{
public:
	MkGuiScopedStyle()= default;
	MkGuiScopedStyle(MkGuiStyleConstPtr style);
	~MkGuiScopedStyle();

	MkGuiScopedStyle(const MkGuiScopedStyle&)= delete;
	MkGuiScopedStyle& operator=(const MkGuiScopedStyle&)= delete;

private:
	struct Impl;
	Impl* m_impl= nullptr;
};
