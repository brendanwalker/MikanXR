#pragma once

#include "ObjectFwd.h"

#include <string>

using MikanFunctionDatabaseConstPtr= std::shared_ptr<const class MikanFunctionDatabase>;

class FunctionDatabaseEnumerator
{
public:
	FunctionDatabaseEnumerator(MikanFunctionDatabaseConstPtr database, const std::string& systemFilter= "",
							   const std::string& componentFilter= "", const std::string& functionFilter= "");
	bool isValid() const;
	void next();
	int getCurrentFunctionIndex() const { return m_currentIndex; }

private:
	bool matchesFilters(int functionIndex) const;
	static bool matchesFilter(const std::string& value, const std::string& filter);

private:
	MikanFunctionDatabaseConstPtr m_database;
	std::string m_systemFilter;
	std::string m_componentFilter;
	std::string m_functionFilter;
	int m_currentIndex;
};
