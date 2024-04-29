#pragma once

#include "NodeEditorUI.h"
#include <vector>

class ClientListDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ClientListDataSource();

	inline int getClientIndex(int index)
	{
		return comboEntries[index].clientIndex;
	}

	virtual int getEntryCount() override
	{
		return (int)comboEntries.size();
	}

	virtual const std::string& getEntryDisplayString(int index) override
	{
		return comboEntries[index].clientEntryName;
	}

private:
	struct ComboEntry
	{
		int clientIndex;
		std::string clientEntryName;
	};

	std::vector<ComboEntry> comboEntries;
};