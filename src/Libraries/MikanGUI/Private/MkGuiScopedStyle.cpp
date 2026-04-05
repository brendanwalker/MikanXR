#include "MkGuiScopedStyle.h"
#include "MkGuiContext.h"

MkGuiScopedStyle::MkGuiScopedStyle(MkGuiStyleConstPtr style, MkGuiContext* guiContext)
{
	if (!style)
		return;

	// Push font first (popped last)
	if (!style->fontName.empty() && guiContext)
	{
		if (style->fontName == "normal_icon")
		{
			m_font = std::make_unique<MkGuiScopedFont>(guiContext->getNormalIconFont());
		}
		else if (style->fontName == "big_icon")
		{
			m_font = std::make_unique<MkGuiScopedFont>(guiContext->getBigIconFont());
		}
	}

	// Push style vars
	for (const MkGuiStyleVarEntry& e : style->vars)
	{
		if (e.isVec2)
			m_vars.push(e.var, e.vec2Val);
		else
			m_vars.push(e.var, e.floatVal);
	}

	// Push colors (popped first)
	for (const MkGuiStyleColorEntry& e : style->colors)
	{
		m_colors.push(e.col, e.value);
	}
}
