#include "PropertyInterface.h"

PropertyDescriptor::PropertyDescriptor()
	: m_propertyName("")
	, m_dataType(MikanVariantType::INVALID)
	, m_bIsReadable(false)
	, m_bIsWritable(false)
	, m_defaultValue(std::make_unique<MikanVariant>())
{}

PropertyDescriptor::PropertyDescriptor(const std::string& name)
	: m_propertyName(name)
	, m_dataType(MikanVariantType::INVALID)
	, m_bIsReadable(false)
	, m_bIsWritable(false)
	, m_defaultValue(std::make_unique<MikanVariant>())
{
}

PropertyDescriptor::PropertyDescriptor(const std::string& name, MikanVariantType type)
	: m_propertyName(name)
	, m_dataType(type)
	, m_bIsReadable(true)
	, m_bIsWritable(true)
	, m_defaultValue(std::make_unique<MikanVariant>())
{}

PropertyDescriptor::PropertyDescriptor(const PropertyDescriptor& other)
	: m_propertyName(other.m_propertyName)
	, m_bIsReadable(other.m_bIsReadable)
	, m_bIsWritable(other.m_bIsWritable)
	, m_defaultValue(std::make_unique<MikanVariant>(*(other.m_defaultValue.get())))
{}

PropertyDescriptorPtr PropertyDescriptor::setReadOnly()
{
	assert(m_dataType != MikanVariantType::INVALID);
	m_bIsReadable = true;
	m_bIsWritable = false;
	return shared_from_this();
}
