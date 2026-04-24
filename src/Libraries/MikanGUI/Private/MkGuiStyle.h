#pragma once

#include "IMkGuiStyle.h"

#include <map>
#include <string>
#include <vector>

class MkGuiStyle : public IMkGuiStyle
{
public:
	// -- IMkGuiStyle
	const std::string& getName() const override { return m_name; }
	int getLabelWidth() const override { return m_labelWidth; }
	int getValueWidth() const override { return m_valueWidth; }
	struct ImFont* getFont() const override { return m_font; }

	int getFloatVarCount() const override { return (int)m_floatVars.size(); }
	const MkGuiStyleFloatEntry& getFloatVar(int index) const override { return m_floatVars[index]; }

	int getVec2VarCount() const override { return (int)m_vec2Vars.size(); }
	const MkGuiStyleVec2Entry& getVec2Var(int index) const override { return m_vec2Vars[index]; }

	int getColorCount() const override { return (int)m_colors.size(); }
	const MkGuiStyleColorEntry& getColor(int index) const override { return m_colors[index]; }

	const MkGuiStyleTextureEntry* findTexture(const std::string& name) const override;

	// -- Mutable accessors (for use by MkGuiStyleManager only)
	std::string& name() { return m_name; }
	int& labelWidth() { return m_labelWidth; }
	int& valueWidth() { return m_valueWidth; }
	struct ImFont*& font() { return m_font; }
	std::vector<MkGuiStyleFloatEntry>& floatVars() { return m_floatVars; }
	std::vector<MkGuiStyleVec2Entry>& vec2Vars() { return m_vec2Vars; }
	std::vector<MkGuiStyleColorEntry>& colors() { return m_colors; }
	std::map<std::string, MkGuiStyleTextureEntry>& textures() { return m_textures; }

private:
	std::string m_name;
	int m_labelWidth = IMkGuiStyle::k_defaultLabelWidth;
	int m_valueWidth = IMkGuiStyle::k_defaultValueWidth;
	std::vector<MkGuiStyleFloatEntry> m_floatVars;
	std::vector<MkGuiStyleVec2Entry> m_vec2Vars;
	std::vector<MkGuiStyleColorEntry> m_colors;
	std::map<std::string, MkGuiStyleTextureEntry> m_textures;
	struct ImFont* m_font = nullptr;
};
