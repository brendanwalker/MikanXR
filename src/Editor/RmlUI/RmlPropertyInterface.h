#pragma once

#include "RmlFwd.h"

#include <string>
#include <memory>

class RmlPropertyDescriptor;
using RmlPropertyDescriptorPtr = std::shared_ptr<RmlPropertyDescriptor>;
using RmlPropertyDescriptorConstPtr = std::shared_ptr<const RmlPropertyDescriptor>;

class RmlPropertyDescriptor : public std::enable_shared_from_this<RmlPropertyDescriptor>
{
public:
	RmlPropertyDescriptor();
	RmlPropertyDescriptor(const std::string& name);

	const std::string& getName() const { return m_propertyName; }
	bool isReadOnly() const { return m_isReadOnly; }
	RmlPropertyDescriptorPtr setReadOnly()
	{ 
		m_isReadOnly = true; 
		return shared_from_this();
	}

	template <typename t_value_type>
	RmlPropertyDescriptorPtr setDefaultValue(t_value_type value)
	{
		*m_defaultValue = value;
		return shared_from_this();
	}
	const Rml::Variant& getDefaultValue() const { return *(m_defaultValue.get()); }
	
private:
	std::string m_propertyName;
	bool m_isReadOnly = false;
	std::unique_ptr<Rml::Variant> m_defaultValue;
};

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