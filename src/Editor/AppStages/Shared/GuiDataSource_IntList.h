#pragma once

#include "MkGuiDrawUtils.h"

#include <string>
#include <vector>

class GuiDataSource_IntList : public MkGui::ComboBoxDataSource
{
public:
	void setEntries(const std::vector<int>& entries);
	int getEntryIndexByValue(int value) const;
	int getEntryValue(int index) const;

	// MkGui::ComboBoxDataSource
	virtual int getEntryCount() const override;
	virtual const std::string& getEntryDisplayString(int index) const override;

private:
	std::vector<int> m_entries;
	std::vector<std::string> m_displayStrings;
	static const std::string k_emptyString;
};
