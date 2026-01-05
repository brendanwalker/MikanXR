#pragma once

#include "RmlPropertyInterface.h"

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
	RmlPropertyDescriptorConstPtr descriptor;

	MikanPropertyEntry(
		int inPropertyIndex,
		const std::string& inSystemName,
		const std::string& inComponentClassName,
		RmlPropertyDescriptorConstPtr inDescriptor)
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
		std::vector<RmlPropertyDescriptorConstPtr> descriptors;
		t_system_class::getRmlPropertyDescriptors(descriptors);

		for (const RmlPropertyDescriptorConstPtr& descriptor : descriptors)
		{
			registerProperty(t_system_class::k_objectSystemClassName, "", descriptor);
		}
	}

	template <class t_system_class, class t_component_class>
	void registerPropertiesForComponent()
	{
		std::vector<RmlPropertyDescriptorConstPtr> descriptors;
		t_component_class::getRmlPropertyDescriptors(descriptors);

		for (const RmlPropertyDescriptorConstPtr& descriptor : descriptors)
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
		RmlPropertyDescriptorConstPtr descriptor);

	const std::vector<MikanPropertyEntry>& getAllProperties() const { return m_properties; }

	int findPropertyIndex(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& propertyName) const;
	const MikanPropertyEntry* getPropertyByIndex(int propertyIndex) const;

	RmlPropertyDescriptorConstPtr findPropertyDescriptor(
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
