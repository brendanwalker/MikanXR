#pragma once

#include "RmlFwd.h"

#include <string>
#include <memory>

class RmlPropertyDescriptor
{
public:
	RmlPropertyDescriptor() = default;
	RmlPropertyDescriptor(const std::string& name, bool readOnly = false) 
		: m_propertyName(name)
		, m_isReadOnly(readOnly) 
	{}

	const std::string& getName() const { return m_propertyName; }
	bool isReadOnly() const { return m_isReadOnly; }

private:
	std::string m_propertyName;
	bool m_isReadOnly = false;
};
using RmlPropertyDescriptorConstPtr = std::shared_ptr<const RmlPropertyDescriptor>;

class IRmlPropertyInterface
{
public:
	virtual bool getPropertyValueFromRml(
		RmlPropertyDescriptorConstPtr propertyDesc,
		Rml::Variant& outValue) const 
	{
		return false;
	}
	virtual bool setPropertyValueFromRml(
		RmlPropertyDescriptorConstPtr propertyDesc,
		const Rml::Variant& inValue)
	{
		return false;
	}
};
using IRmlPropertyInterfacePtr = std::shared_ptr<IRmlPropertyInterface>;
using IRmlPropertyInterfaceConstPtr = std::shared_ptr<const IRmlPropertyInterface>;
using IRmlPropertyInterfaceWeakPtr = std::weak_ptr<IRmlPropertyInterface>;