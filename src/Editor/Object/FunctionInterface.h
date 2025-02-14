#pragma once

#include <string>
#include <vector>

struct FunctionDescriptor
{
	std::string functionName;
	std::string displayName;
};

class IFunctionInterface
{
public:
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const = 0;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const = 0;
	virtual bool invokeFunction(const std::string& propertyName) = 0;
};