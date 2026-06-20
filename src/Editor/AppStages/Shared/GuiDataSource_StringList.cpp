#include "Shared/GuiDataSource_StringList.h"

#include <algorithm>

const std::string GuiDataSource_StringList::k_emptyString;

void GuiDataSource_StringList::setEntries(const std::vector<std::string>& entries)
{
	m_entries= entries;
}

int GuiDataSource_StringList::getEntryIndexByString(const std::string& value) const
{
	auto it= std::find(m_entries.begin(), m_entries.end(), value);
	return (it != m_entries.end()) ? (int)(it - m_entries.begin()) : -1;
}

int GuiDataSource_StringList::getEntryCount() const
{
	return (int)m_entries.size();
}

const std::string& GuiDataSource_StringList::getEntryDisplayString(int index) const
{
	if (index >= 0 && index < (int)m_entries.size())
		return m_entries[index];
	return k_emptyString;
}
