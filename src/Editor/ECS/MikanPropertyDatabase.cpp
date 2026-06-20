#include "MikanPropertyDatabase.h"

MikanPropertyDatabase::MikanPropertyDatabase()
{
}

void MikanPropertyDatabase::clear()
{
	m_properties.clear();
	m_propertyKeyToIndexMap.clear();
}

void MikanPropertyDatabase::registerProperty(
	const std::string& systemName,
	const std::string& componentClassName,
	PropertyDescriptorConstPtr descriptor)
{
	const int propertyIndex= (int)m_properties.size();
	m_properties.push_back(MikanPropertyEntry(propertyIndex, systemName, componentClassName, descriptor));

	// Add to the lookup map for fast indexing
	const std::string key= makePropertyKey(systemName, componentClassName, descriptor->getName());
	m_propertyKeyToIndexMap[key]= propertyIndex;
}

int MikanPropertyDatabase::findPropertyIndex(
	const std::string& systemName,
	const std::string& componentClassName,
	const std::string& propertyName) const
{
	// Use the hash map for O(1) lookup
	const std::string key= makePropertyKey(systemName, componentClassName, propertyName);
	auto it= m_propertyKeyToIndexMap.find(key);

	if (it != m_propertyKeyToIndexMap.end())
	{
		return it->second;
	}

	return -1;
}

std::string MikanPropertyDatabase::makePropertyKey(
	const std::string& systemName,
	const std::string& componentClassName,
	const std::string& propertyName)
{
	// Create a composite key using delimiter that won't appear in names
	// Format: "systemName|componentClassName|propertyName"
	return systemName + "|" + componentClassName + "|" + propertyName;
}

const MikanPropertyEntry* MikanPropertyDatabase::getPropertyByIndex(int propertyIndex) const
{
	if (propertyIndex >= 0 && propertyIndex < (int)m_properties.size())
	{
		return &m_properties[propertyIndex];
	}

	return nullptr;
}

PropertyDescriptorConstPtr MikanPropertyDatabase::findPropertyDescriptor(
	const std::string& systemName,
	const std::string& componentClassName,
	const std::string& propertyName) const
{
	int propertyIndex= findPropertyIndex(systemName, componentClassName, propertyName);
	if (propertyIndex != -1)
	{
		const MikanPropertyEntry* entry= getPropertyByIndex(propertyIndex);

		if (entry)
		{
			return entry->descriptor;
		}
	}

	return nullptr;
}