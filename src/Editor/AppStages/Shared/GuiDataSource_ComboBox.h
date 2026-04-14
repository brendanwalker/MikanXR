#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MkGuiDrawUtils.h"
#include "ObjectSystemFwd.h"

#include <functional>
#include <vector>

class GuiDataSource_ComboBox : public MkGui::ComboBoxDataSource
{
public:
	using SystemComponentPair = std::pair<std::string, std::string>;
	using ComponentFilter = std::function<bool(MikanComponentPtr)>;
	using DisplayStringBuilder = std::function<std::string(MikanComponentPtr)>;

	GuiDataSource_ComboBox(
		ProjectManagerPtr projectManager,
		const std::vector<SystemComponentPair>& systemNameComponentPairs);

	void setFilter(ComponentFilter filter);
	void setDisplayStringBuilder(DisplayStringBuilder builder);

	void refreshEntries();
	int getEntryIndexByComponent(MikanComponentPtr component) const;
	int getEntryIndexByComponentId(MikanComponentID componentId) const;
	MikanComponentPtr getEntryAtIndex(int index) const;
	int getEntryCount() const;
	const std::string& getEntryDisplayString(int index) const;

private:
	ProjectManagerWeakPtr m_projectManager;
	std::vector<SystemComponentPair> m_systemComponentPairs;
	ComponentFilter m_filter;
	DisplayStringBuilder m_displayStringBuilder;
	std::vector<MikanComponentPtr> m_comboEntrieValues;
	std::vector<std::string> m_displayStrings;
};
