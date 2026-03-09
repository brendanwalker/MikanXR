#include "FunctionDatabaseEnumerator.h"
#include "MikanFunctionDatabase.h"
#include "StringUtils.h"

#include <algorithm>
#include <cctype>

FunctionDatabaseEnumerator::FunctionDatabaseEnumerator(
	MikanFunctionDatabaseConstPtr database,
	const std::string& systemFilter,
	const std::string& componentFilter,
	const std::string& functionFilter)
	: m_database(database)
	, m_systemFilter(systemFilter)
	, m_componentFilter(componentFilter)
	, m_functionFilter(functionFilter)
	, m_currentIndex(0)
{
	// Find the first valid entry matching the filters
	if (!matchesFilters(m_currentIndex))
	{
		next();
	}
}

bool FunctionDatabaseEnumerator::isValid() const
{
	if (!m_database)
		return false;

	const int totalFunctionCount = (int)m_database->getAllFunctions().size();
	return m_currentIndex < totalFunctionCount;
}

void FunctionDatabaseEnumerator::next()
{
	if (!m_database)
		return;

	const int totalFunctionCount = (int)m_database->getAllFunctions().size();

	// Advance to the next index
	m_currentIndex++;

	// Keep advancing until we find a match or reach the end
	while (m_currentIndex < totalFunctionCount)
	{
		if (matchesFilters(m_currentIndex))
		{
			break;
		}
		m_currentIndex++;
	}
}

bool FunctionDatabaseEnumerator::matchesFilters(int functionIndex) const
{
	if (!m_database)
		return false;

	const MikanFunctionEntry* entry = m_database->getFunctionByIndex(functionIndex);
	if (!entry)
		return false;

	// Check system filter
	if (!matchesFilter(entry->systemName, m_systemFilter))
		return false;

	// Check component filter
	if (!matchesFilter(entry->componentClassName, m_componentFilter))
		return false;

	// Check function filter
	if (!matchesFilter(entry->descriptor->getFunctionName(), m_functionFilter))
		return false;

	return true;
}

bool FunctionDatabaseEnumerator::matchesFilter(const std::string& value, const std::string& filter)
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
