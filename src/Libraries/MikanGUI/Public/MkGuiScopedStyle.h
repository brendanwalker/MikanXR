#pragma once

#include "MkGuiStyle.h"
#include "MkGuiScopedStyleVar.h"
#include "MkGuiScopedStyleColor.h"
#include "MkGuiScopedFont.h"

#include <memory>

class MkGuiScopedStyle
{
public:
	MkGuiScopedStyle() = default;
	MkGuiScopedStyle(MkGuiStyleConstPtr style);
	~MkGuiScopedStyle() = default;

	MkGuiScopedStyle(const MkGuiScopedStyle&) = delete;
	MkGuiScopedStyle& operator=(const MkGuiScopedStyle&) = delete;

private:
	// Destruction order is reverse of declaration order (LIFO):
	// m_colors pops first, then m_vars, then m_font
	std::unique_ptr<MkGuiScopedFont> m_font;
	MkGuiScopedStyleVar m_vars;
	MkGuiScopedStyleColor m_colors;
};
