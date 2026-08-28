#pragma once

#include "MkGuiExport.h"
#include "MkGuiFwd.h"

class MIKAN_GUI_CLASS MkGuiScopedFont
{
public:
	// A size <= 0 uses the size the font was loaded at (the pre-1.92 PushFont behavior)
	explicit MkGuiScopedFont(ImFont* font, float size= 0.f);
	~MkGuiScopedFont();

	MkGuiScopedFont(const MkGuiScopedFont&)= delete;
	MkGuiScopedFont& operator=(const MkGuiScopedFont&)= delete;
};
