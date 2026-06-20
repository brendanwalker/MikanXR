#pragma once

#include "MkGuiExport.h"
#include "MkGuiFwd.h"

class MIKAN_GUI_CLASS MkGuiScopedFont
{
public:
	explicit MkGuiScopedFont(ImFont* font);
	~MkGuiScopedFont();

	MkGuiScopedFont(const MkGuiScopedFont&)= delete;
	MkGuiScopedFont& operator=(const MkGuiScopedFont&)= delete;
};
