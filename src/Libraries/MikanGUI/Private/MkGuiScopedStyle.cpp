#include "MkGuiScopedStyle.h"
#include "MkGuiScopedFont.h"
#include "MkGuiScopedStyleVar.h"
#include "MkGuiScopedStyleColor.h"

#include <memory>

struct MkGuiScopedStyle::Impl
{
	// Destruction order is reverse of declaration order (LIFO):
	// m_colors pops first, then m_vars, then m_font
	MkGuiScopedStyleColor colors;
	MkGuiScopedStyleVar vars;
	std::unique_ptr<MkGuiScopedFont> font;
};

MkGuiScopedStyle::MkGuiScopedStyle(MkGuiStyleConstPtr style)
{
	if (!style)
		return;

	m_impl = new Impl();

	// Push font first (popped last)
	if (style->getFont())
	{
		m_impl->font = std::make_unique<MkGuiScopedFont>(style->getFont());
	}

	// Push style floats
	for (int i = 0; i < style->getFloatVarCount(); ++i)
	{
		const MkGuiStyleFloatEntry& e = style->getFloatVar(i);
		m_impl->vars.push(e.var, e.floatVal);
	}

	// Push style vecs
	for (int i = 0; i < style->getVec2VarCount(); ++i)
	{
		const MkGuiStyleVec2Entry& e = style->getVec2Var(i);
		m_impl->vars.push(e.var, e.vec2Val);
	}

	// Push colors (popped first)
	for (int i = 0; i < style->getColorCount(); ++i)
	{
		const MkGuiStyleColorEntry& e = style->getColor(i);
		m_impl->colors.push(e.col, e.value);
	}
}

MkGuiScopedStyle::~MkGuiScopedStyle()
{
	delete m_impl;
}
