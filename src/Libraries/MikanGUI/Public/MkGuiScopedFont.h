#pragma once

#include "MkGuiFwd.h"

class MkGuiScopedFont
{
public:
	explicit MkGuiScopedFont(ImFont* font);
	~MkGuiScopedFont();

	MkGuiScopedFont(const MkGuiScopedFont&) = delete;
	MkGuiScopedFont& operator=(const MkGuiScopedFont&) = delete;
};
