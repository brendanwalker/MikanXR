#pragma once
#include "PropertyInterface.h"
#include <string>
#include <vector>

enum class eEnumDisplayStyle
{
	ComboBox,
	RadioButtons
};

class EnumPropertyMetaData : public PropertyMetaData
{
public:
	EnumPropertyMetaData(
		const std::string* strings,
		int count,
		eEnumDisplayStyle style= eEnumDisplayStyle::ComboBox)
		: m_style(style)
	{
		for (int i= 0; i < count; i++)
			m_strings.push_back(strings[i]);
	}

	eEnumDisplayStyle getDisplayStyle() const { return m_style; }
	const std::vector<std::string>& getStrings() const { return m_strings; }

private:
	eEnumDisplayStyle m_style;
	std::vector<std::string> m_strings;
};
