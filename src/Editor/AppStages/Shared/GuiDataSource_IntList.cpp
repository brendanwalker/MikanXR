#include "Shared/GuiDataSource_IntList.h"

#include <algorithm>

const std::string GuiDataSource_IntList::k_emptyString;

void GuiDataSource_IntList::setEntries(const std::vector<int>& entries)
{
	m_entries= entries;
	m_displayStrings.clear();
	m_displayStrings.reserve(entries.size());
	for (int v : entries)
		m_displayStrings.push_back(std::to_string(v));
}

int GuiDataSource_IntList::getEntryIndexByValue(int value) const
{
	auto it= std::find(m_entries.begin(), m_entries.end(), value);
	return (it != m_entries.end()) ? (int)(it - m_entries.begin()) : -1;
}

int GuiDataSource_IntList::getEntryValue(int index) const
{
	if (index >= 0 && index < (int)m_entries.size())
		return m_entries[index];
	return -1;
}

int GuiDataSource_IntList::getEntryCount() const
{
	return (int)m_entries.size();
}

const std::string& GuiDataSource_IntList::getEntryDisplayString(int index) const
{
	if (index >= 0 && index < (int)m_displayStrings.size())
		return m_displayStrings[index];
	return k_emptyString;
}
