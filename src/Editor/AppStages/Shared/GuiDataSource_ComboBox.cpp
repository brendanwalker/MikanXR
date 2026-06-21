#include "GuiDataSource_ComboBox.h"
#include "MikanObjectSystem.h"
#include "MikanComponent.h"
#include "ProjectManager.h"

GuiDataSource_ComboBox::GuiDataSource_ComboBox(ProjectManagerPtr projectManager,
											   const std::vector<SystemComponentPair>& systemNameComponentPairs)
	: m_projectManager(projectManager)
	, m_systemComponentPairs(systemNameComponentPairs)
{
}

void GuiDataSource_ComboBox::setFilter(ComponentFilter filter) { m_filter= filter; }

void GuiDataSource_ComboBox::setDisplayStringBuilder(DisplayStringBuilder builder) { m_displayStringBuilder= builder; }

void GuiDataSource_ComboBox::refreshEntries()
{
	m_comboEntrieValues.clear();
	m_displayStrings.clear();

	if (auto projectManager= m_projectManager.lock())
	{
		for (const auto& pair : m_systemComponentPairs)
		{
			const std::string& systemName= pair.first;
			const std::string& componentName= pair.second;
			MikanObjectSystemPtr objectSystem= projectManager->getSystemByName(systemName);
			if (!objectSystem)
				continue;

			std::vector<MikanComponentPtr> batch;
			objectSystem->getComponentList(componentName, batch);

			for (MikanComponentPtr comp : batch)
			{
				if (m_filter && !m_filter(comp))
					continue;

				m_comboEntrieValues.push_back(comp);
				m_displayStrings.push_back(m_displayStringBuilder ? m_displayStringBuilder(comp) : comp->getName());
			}
		}
	}
}

int GuiDataSource_ComboBox::getEntryIndexByComponent(MikanComponentPtr component) const
{
	auto it= std::find(m_comboEntrieValues.begin(), m_comboEntrieValues.end(), component);
	if (it != m_comboEntrieValues.end())
	{
		return static_cast<int>(std::distance(m_comboEntrieValues.begin(), it));
	}

	return -1;
}

int GuiDataSource_ComboBox::getEntryIndexByComponentId(MikanComponentID componentId) const
{
	for (size_t i= 0; i < m_comboEntrieValues.size(); ++i)
	{
		if (m_comboEntrieValues[i]->getComponentId() == componentId)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

MikanComponentPtr GuiDataSource_ComboBox::getEntryAtIndex(int index) const { return m_comboEntrieValues[index]; }

int GuiDataSource_ComboBox::getEntryCount() const { return (int)m_comboEntrieValues.size(); }

const std::string& GuiDataSource_ComboBox::getEntryDisplayString(int index) const { return m_displayStrings[index]; }
