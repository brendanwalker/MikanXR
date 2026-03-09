#pragma once

#include <string>
#include <memory>

class FunctionDescriptor
{
public:
	FunctionDescriptor() = default;
	FunctionDescriptor(
		const std::string& functionName, 
		const std::string& displayName)
		: m_functionName(functionName)
		, m_displayName(displayName)
	{ }

	inline const std::string& getFunctionName() const { return m_functionName; }
	inline const std::string& getDisplayName() const { return m_displayName; }

private:
	std::string m_functionName;
	std::string m_displayName;
};
using FunctionDescriptorConstPtr = std::shared_ptr<const FunctionDescriptor>;

class IFunctionInterface
{
public:
	virtual bool invokeFunction(const std::string& functionName) = 0;
};
using IFunctionInterfacePtr = std::shared_ptr<IFunctionInterface>;
using IFunctionInterfaceConstPtr = std::shared_ptr<const IFunctionInterface>;
using IFunctionInterfaceWeakPtr = std::weak_ptr<IFunctionInterface>;