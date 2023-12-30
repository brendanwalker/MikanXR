#pragma once

#include <string>

namespace NodeEditorUI
{
	bool DrawPropertySheetHeader(const std::string headerText);
	void DrawStaticTextProperty(const std::string label, const std::string text);
	void DrawCheckBoxProperty(const std::string fieldName, const std::string label, bool& inout_value);
	bool DrawSimpleComboBoxProperty(
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex);
};