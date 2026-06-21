#pragma once

#include "ObjectFwd.h"

#include <string>

using MikanPropertyDatabaseConstPtr= std::shared_ptr<const class MikanPropertyDatabase>;

class PropertyDatabaseEnumerator
{
public:
	PropertyDatabaseEnumerator(MikanPropertyDatabaseConstPtr database, const std::string& systemFilter= "",
							   const std::string& componentFilter= "", const std::string& propertyFilter= "");

	bool isValid() const;
	void next();
	int getCurrentPropertyIndex() const { return m_currentIndex; }

private:
	bool matchesFilters(int propertyIndex) const;
	static bool matchesFilter(const std::string& value, const std::string& filter);

private:
	MikanPropertyDatabaseConstPtr m_database;
	std::string m_systemFilter;
	std::string m_componentFilter;
	std::string m_propertyFilter;
	int m_currentIndex;
};
