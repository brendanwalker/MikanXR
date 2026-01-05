#include "PropertyDatabaseEnumerator.h"
#include "MikanPropertyDatabase.h"
#include "StringUtils.h"

#include <algorithm>
#include <cctype>

PropertyDatabaseEnumerator::PropertyDatabaseEnumerator(
	MikanPropertyDatabaseConstPtr database,
	const std::string& systemFilter,
	const std::string& componentFilter,
	const std::string& propertyFilter)
	: m_database(database)
	, m_systemFilter(systemFilter)
	, m_componentFilter(componentFilter)
	, m_propertyFilter(propertyFilter)
	, m_currentIndex(0)
{
	// Find the first valid entry matching the filters
	if (!matchesFilters(m_currentIndex))
	{
		next();
	}
}

bool PropertyDatabaseEnumerator::isValid() const
{
	if (!m_database)
		return false;

	const int totalPropertyCount = (int)m_database->getAllProperties().size();
	return m_currentIndex < totalPropertyCount;
}

void PropertyDatabaseEnumerator::next()
{
	if (!m_database)
		return;

	const int totalPropertyCount = (int)m_database->getAllProperties().size();

	// Advance to the next index
	m_currentIndex++;

	// Keep advancing until we find a match or reach the end
	while (m_currentIndex < totalPropertyCount)
	{
		if (matchesFilters(m_currentIndex))
		{
			break;
		}
		m_currentIndex++;
	}
}

bool PropertyDatabaseEnumerator::matchesFilters(int propertyIndex) const
{
	if (!m_database)
		return false;

	const MikanPropertyEntry* entry = m_database->getPropertyByIndex(propertyIndex);
	if (!entry)
		return false;

	// Check system filter
	if (!matchesFilter(entry->systemName, m_systemFilter))
		return false;

	// Check component filter
	if (!matchesFilter(entry->componentClassName, m_componentFilter))
		return false;

	// Check property filter
	if (!matchesFilter(entry->descriptor->getName(), m_propertyFilter))
		return false;

	return true;
}

bool PropertyDatabaseEnumerator::matchesFilter(const std::string& value, const std::string& filter)
{
	// Empty filter matches everything
	if (filter.empty())
		return true;

	// Convert both to lowercase for case-insensitive comparison
	std::string lowerValue = value;
	std::string lowerFilter = filter;

	std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(),
		[](unsigned char c) { return std::tolower(c); });
	std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(),
		[](unsigned char c) { return std::tolower(c); });

	return lowerValue == lowerFilter;
}
