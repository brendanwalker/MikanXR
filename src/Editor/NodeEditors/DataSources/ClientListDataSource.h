#pragma once

#include "NodeEditorUI.h"
#include <vector>

class ClientListDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ClientListDataSource();

	int getEntryIndex(const std::string& entryName) const
	{
		auto it= std::find(comboEntries.begin(), comboEntries.end(), entryName);
		if (it != comboEntries.end())
		{
			return static_cast<int>(std::distance(comboEntries.begin(), it));
		}

		return -1;
	}

	virtual int getEntryCount() override
	{
		return (int)comboEntries.size();
	}

	virtual const std::string& getEntryDisplayString(int index) override
	{
		return comboEntries[index];
	}

private:
	std::vector<std::string> comboEntries;
};