#pragma once

#include "MikanVariantTypes.h"

#include <string>
#include <memory>
#include <vector>

class PropertyMetaData
{
public:
	virtual ~PropertyMetaData() = default;
};
using PropertyMetaDataPtr = std::shared_ptr<PropertyMetaData>;

class PropertyDescriptor;
using PropertyDescriptorPtr = std::shared_ptr<PropertyDescriptor>;
using PropertyDescriptorConstPtr = std::shared_ptr<const PropertyDescriptor>;

class PropertyDescriptor : public std::enable_shared_from_this<PropertyDescriptor>
{
public:
	PropertyDescriptor();
	PropertyDescriptor(const std::string& name);
	PropertyDescriptor(const std::string& name, MikanVariantType type);
	PropertyDescriptor(const PropertyDescriptor& other);

	const std::string& getName() const { return m_propertyName; }
	const MikanVariantType getDataType() const { return m_dataType; }
	bool isReadable() const { return m_bIsReadable; }
	bool isWritable() const { return m_bIsWritable; }
	bool isReadOnly() const { return m_bIsReadable && !m_bIsWritable; }
	bool isUIHidden() const { return m_bIsUIHidden; }
	PropertyDescriptorPtr setReadOnly();
	PropertyDescriptorPtr setUIHidden();
	PropertyDescriptorPtr addMetaData(PropertyMetaDataPtr metaData);

	template <typename t_value_type>
	PropertyDescriptorPtr setDefaultValue(t_value_type value)
	{
		assert(m_dataType != MikanVariantType::INVALID);
		*m_defaultValue = value;
		return shared_from_this();
	}
	const MikanVariant& getDefaultValue() const { return *(m_defaultValue.get()); }

	template <class t_meta_type>
	const t_meta_type* getMetaDataOfType() const
	{
		for (const PropertyMetaDataPtr& meta : m_metaDataList)
		{
			if (const t_meta_type* typed = dynamic_cast<const t_meta_type*>(meta.get()))
				return typed;
		}
		return nullptr;
	}

private:
	std::string m_propertyName;
	MikanVariantType m_dataType = MikanVariantType::INVALID;
	bool m_bIsReadable = true;
	bool m_bIsWritable = true;
	bool m_bIsUIHidden = false;
	std::unique_ptr<MikanVariant> m_defaultValue;
	std::vector<PropertyMetaDataPtr> m_metaDataList;
};

class IPropertyInterface
{
public:
	virtual bool getPropertyValue(
		const std::string& propertyName,
		MikanVariant& outValue) const
	{
		return false;
	}
	virtual bool setPropertyValue(
		const std::string& propertyName,
		const MikanVariant& inValue)
	{
		return false;
	}
};
using IPropertyInterfacePtr = std::shared_ptr<IPropertyInterface>;
using IPropertyInterfaceConstPtr = std::shared_ptr<const IPropertyInterface>;
using IPropertyInterfaceWeakPtr = std::weak_ptr<IPropertyInterface>;