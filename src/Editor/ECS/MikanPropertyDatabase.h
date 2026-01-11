#pragma once

#include "PropertyInterface.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using MikanPropertyDatabasePtr = std::shared_ptr<class MikanPropertyDatabase>;
using MikanPropertyDatabaseConstPtr = std::shared_ptr<const class MikanPropertyDatabase>;
using MikanPropertyDatabaseWeakPtr = std::weak_ptr<class MikanPropertyDatabase>;

struct MikanPropertyEntry
{
	int propertyIndex;
	std::string systemName;
	std::string componentClassName;
	PropertyDescriptorConstPtr descriptor;

	MikanPropertyEntry(
		int inPropertyIndex,
		const std::string& inSystemName,
		const std::string& inComponentClassName,
		PropertyDescriptorConstPtr inDescriptor)
		: propertyIndex(inPropertyIndex)
		, systemName(inSystemName)
		, componentClassName(inComponentClassName)
		, descriptor(inDescriptor)
	{
	}
};

class MikanPropertyDatabase
{
public:
	MikanPropertyDatabase();

	template <class t_system_class>
	void registerPropertiesForSystem()
	{
		std::vector<PropertyDescriptorConstPtr> descriptors;
		t_system_class::getPropertyDescriptors(descriptors);

		for (const PropertyDescriptorConstPtr& descriptor : descriptors)
		{
			registerProperty(t_system_class::k_objectSystemClassName, "", descriptor);
		}
	}

	template <class t_system_class, class t_component_class>
	void registerPropertiesForComponent()
	{
		std::vector<PropertyDescriptorConstPtr> descriptors;
		t_component_class::getPropertyDescriptors(descriptors);

		for (const PropertyDescriptorConstPtr& descriptor : descriptors)
		{
			registerProperty(
				t_system_class::k_objectSystemClassName, 
				t_component_class::k_componentClassName, 
				descriptor);
		}
	}

	void clear();
	void registerProperty(
		const std::string& systemName,
		const std::string& componentClassName,
		PropertyDescriptorConstPtr descriptor);

	const std::vector<MikanPropertyEntry>& getAllProperties() const { return m_properties; }

	int findPropertyIndex(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& propertyName) const;
	const MikanPropertyEntry* getPropertyByIndex(int propertyIndex) const;

	PropertyDescriptorConstPtr findPropertyDescriptor(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& propertyName) const;

private:
	static std::string makePropertyKey(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& propertyName);

private:
	std::vector<MikanPropertyEntry> m_properties;
	std::unordered_map<std::string, int> m_propertyKeyToIndexMap;
};
