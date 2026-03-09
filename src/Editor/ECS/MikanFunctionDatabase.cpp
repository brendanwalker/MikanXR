#include "MikanFunctionDatabase.h"

MikanFunctionDatabase::MikanFunctionDatabase()
{
}

void MikanFunctionDatabase::clear()
{
	m_functions.clear();
	m_functionKeyToIndexMap.clear();
}

void MikanFunctionDatabase::registerFunction(
	const std::string& systemName,
	const std::string& componentClassName,
	FunctionDescriptorConstPtr descriptor)
{
	const int functionIndex = (int)m_functions.size();
	m_functions.push_back(MikanFunctionEntry(functionIndex, systemName, componentClassName, descriptor));

	// Add to the lookup map for fast indexing
	const std::string key = makeFunctionKey(systemName, componentClassName, descriptor->getFunctionName());
	m_functionKeyToIndexMap[key] = functionIndex;
}

int MikanFunctionDatabase::findFunctionIndex(
	const std::string& systemName,
	const std::string& componentClassName,
	const std::string& functionName) const
{
	// Use the hash map for O(1) lookup
	const std::string key = makeFunctionKey(systemName, componentClassName, functionName);
	auto it = m_functionKeyToIndexMap.find(key);
	if (it != m_functionKeyToIndexMap.end())
	{
		return it->second;
	}

	return -1;
}

std::string MikanFunctionDatabase::makeFunctionKey(
	const std::string& systemName,
	const std::string& componentClassName,
	const std::string& functionName)
{
	// Create a composite key using delimiter that won't appear in names
	// Format: "systemName|componentClassName|functionName"
	return systemName + "|" + componentClassName + "|" + functionName;
}

const MikanFunctionEntry* MikanFunctionDatabase::getFunctionByIndex(int functionIndex) const
{
	if (functionIndex >= 0 && functionIndex < (int)m_functions.size())
	{
		return &m_functions[functionIndex];
	}

	return nullptr;
}

FunctionDescriptorConstPtr MikanFunctionDatabase::findFunctionDescriptor(
	const std::string& systemName,
	const std::string& componentClassName,
	const std::string& functionName) const
{
	int functionIndex = findFunctionIndex(systemName, componentClassName, functionName);
	if (functionIndex != -1)
	{
		const MikanFunctionEntry* entry = getFunctionByIndex(functionIndex);

		if (entry)
		{
			return entry->descriptor;
		}
	}

	return nullptr;
}