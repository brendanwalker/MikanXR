#pragma once

#include "FunctionInterface.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using MikanFunctionDatabasePtr= std::shared_ptr<class MikanFunctionDatabase>;
using MikanFunctionDatabaseConstPtr= std::shared_ptr<const class MikanFunctionDatabase>;
using MikanFunctionDatabaseWeakPtr= std::weak_ptr<class MikanFunctionDatabase>;

struct MikanFunctionEntry
{
	int propertyIndex;
	std::string systemName;
	std::string componentClassName;
	FunctionDescriptorConstPtr descriptor;

	MikanFunctionEntry(
		int inPropertyIndex,
		const std::string& inSystemName,
		const std::string& inComponentClassName,
		FunctionDescriptorConstPtr inDescriptor)
		: propertyIndex(inPropertyIndex)
		, systemName(inSystemName)
		, componentClassName(inComponentClassName)
		, descriptor(inDescriptor)
	{
	}
};

class MikanFunctionDatabase
{
public:
	MikanFunctionDatabase();

	template <class t_system_class>
	void registerFunctionsForSystem()
	{
		std::vector<FunctionDescriptorConstPtr> descriptors;
		t_system_class::getFunctionDescriptors(descriptors);

		for (const FunctionDescriptorConstPtr& descriptor : descriptors)
		{
			registerFunction(t_system_class::k_objectSystemClassName, "", descriptor);
		}
	}

	template <class t_system_class, class t_component_class>
	void registerFunctionsForComponent()
	{
		std::vector<FunctionDescriptorConstPtr> descriptors;
		t_component_class::getFunctionDescriptors(descriptors);

		for (const FunctionDescriptorConstPtr& descriptor : descriptors)
		{
			registerFunction(
				t_system_class::k_objectSystemClassName,
				t_component_class::k_componentClassName,
				descriptor);
		}
	}

	void clear();
	void registerFunction(
		const std::string& systemName,
		const std::string& componentClassName,
		FunctionDescriptorConstPtr descriptor);

	const std::vector<MikanFunctionEntry>& getAllFunctions() const { return m_functions; }

	int findFunctionIndex(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& functionName) const;
	const MikanFunctionEntry* getFunctionByIndex(int functionIndex) const;

	FunctionDescriptorConstPtr findFunctionDescriptor(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& functionName) const;

private:
	static std::string makeFunctionKey(
		const std::string& systemName,
		const std::string& componentClassName,
		const std::string& functionName);

private:
	std::vector<MikanFunctionEntry> m_functions;
	std::unordered_map<std::string, int> m_functionKeyToIndexMap;
};
