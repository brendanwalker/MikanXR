#pragma once

#include "MikanVariantTypes.h"

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
	RmlPropertyDescriptor(const std::string& name, MikanVariantType type);
	RmlPropertyDescriptor(const RmlPropertyDescriptor& other);

	const std::string& getName() const { return m_propertyName; }
	const MikanVariantType getDataType() const { return m_dataType; }
	bool isReadable() const { return m_bIsReadable; }
	bool isWritable() const { return m_bIsWritable; }
	bool isReadOnly() const { return m_bIsReadable && !m_bIsWritable; }
	RmlPropertyDescriptorPtr setReadOnly();

	template <typename t_value_type>
	RmlPropertyDescriptorPtr setDefaultValue(t_value_type value)
	{
		assert(m_dataType != MikanVariantType::INVALID);
		*m_defaultValue = value;
		return shared_from_this();
	}
	const MikanVariant& getDefaultValue() const { return *(m_defaultValue.get()); }
	
private:
	std::string m_propertyName;
	MikanVariantType m_dataType = MikanVariantType::INVALID;
	bool m_bIsReadable = true;
	bool m_bIsWritable = true;
	std::unique_ptr<MikanVariant> m_defaultValue;
};

class IRmlPropertyInterface
{
public:
	virtual bool getPropertyValueFromRml(
		RmlPropertyDescriptorConstPtr propertyDesc,
		MikanVariant& outValue) const 
	{
		return false;
	}
	virtual bool setPropertyValueFromRml(
		RmlPropertyDescriptorConstPtr propertyDesc,
		const MikanVariant& inValue)
	{
		return false;
	}
};
using IRmlPropertyInterfacePtr = std::shared_ptr<IRmlPropertyInterface>;
using IRmlPropertyInterfaceConstPtr = std::shared_ptr<const IRmlPropertyInterface>;
using IRmlPropertyInterfaceWeakPtr = std::weak_ptr<IRmlPropertyInterface>;