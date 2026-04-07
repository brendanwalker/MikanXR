#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MkGuiDrawUtils.h"
#include "ObjectSystemFwd.h"

#include <vector>

class GuiDataSource_ComboBox : public MkGui::ComboBoxDataSource
{
public:
	using SystemComponentPair = std::pair<std::string, std::string>;

	GuiDataSource_ComboBox(
		ProjectManagerPtr projectManager,
		const std::vector<SystemComponentPair>& systemNameComponentPairs);

	void refreshEntries();
	int getEntryIndexByComponent(MikanComponentPtr component) const;
	int getEntryIndexByComponentId(MikanComponentID componentId) const;
	MikanComponentPtr getEntryAtIndex(int index) const;
	int getEntryCount() const;
	const std::string& getEntryDisplayString(int index) const;

private:
	ProjectManagerWeakPtr m_projectManager;
	std::vector<SystemComponentPair> m_systemComponentPairs;
	std::vector<MikanComponentPtr> m_comboEntrieValues;
};