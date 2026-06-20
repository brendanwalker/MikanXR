#pragma once

#include "MkGuiExport.h"

class MIKAN_GUI_CLASS MkGuiScopedGroup
{
public:
	MkGuiScopedGroup();
	~MkGuiScopedGroup();

	MkGuiScopedGroup(const MkGuiScopedGroup&)= delete;
	MkGuiScopedGroup& operator=(const MkGuiScopedGroup&)= delete;
};
