#pragma once

#include "MkGuiDrawUtils.h"

#include <string>
#include <vector>

class GuiDataSource_StringList : public MkGui::ComboBoxDataSource
{
public:
	void setEntries(const std::vector<std::string>& entries);
	int getEntryIndexByString(const std::string& value) const;

	// MkGui::ComboBoxDataSource
	virtual int getEntryCount() const override;
	virtual const std::string& getEntryDisplayString(int index) const override;

private:
	std::vector<std::string> m_entries;
	static const std::string k_emptyString;
};
