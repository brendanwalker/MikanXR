#pragma once

#include <string>
#include <memory>

class RmlFunctionDescriptor
{
public:
	RmlFunctionDescriptor() = default;
	RmlFunctionDescriptor(
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
using RmlFunctionDescriptorConstPtr = std::shared_ptr<const RmlFunctionDescriptor>;

class IRmlFunctionInterface
{
public:
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) = 0;
};
using IRmlFunctionInterfacePtr = std::shared_ptr<IRmlFunctionInterface>;
using IRmlFunctionInterfaceConstPtr = std::shared_ptr<const IRmlFunctionInterface>;
using IRmlFunctionInterfaceWeakPtr = std::weak_ptr<IRmlFunctionInterface>;