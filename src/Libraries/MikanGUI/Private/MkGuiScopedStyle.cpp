#include "MkGuiScopedStyle.h"
#include "MkGuiContext.h"

MkGuiScopedStyle::MkGuiScopedStyle(MkGuiStyleConstPtr style)
{
	if (!style)
		return;

	// Push font first (popped last)
	if (style->font)
	{
		m_font = std::make_unique<MkGuiScopedFont>(style->font);
	}

	// Push style floats
	for (const MkGuiStyleFloatEntry& e : style->floatVars)
	{
		m_vars.push(e.var, e.floatVal);
	}

	// Push style vecs
	for (const MkGuiStyleVec2Entry& e : style->vec2Vars)
	{
		m_vars.push(e.var, e.vec2Val);
	}

	// Push colors (popped first)
	for (const MkGuiStyleColorEntry& e : style->colors)
	{
		m_colors.push(e.col, e.value);
	}
}
