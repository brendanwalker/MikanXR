#include "RmlPropertyInterface.h"

RmlPropertyDescriptor::RmlPropertyDescriptor()
	: m_propertyName("")
	, m_dataType(MikanVariantType::INVALID)
	, m_bIsReadable(false)
	, m_bIsWritable(false)
	, m_defaultValue(std::make_unique<MikanVariant>())
{}

RmlPropertyDescriptor::RmlPropertyDescriptor(const std::string& name)
	: m_propertyName(name)
	, m_dataType(MikanVariantType::INVALID)
	, m_bIsReadable(false)
	, m_bIsWritable(false)
	, m_defaultValue(std::make_unique<MikanVariant>())
{
}

RmlPropertyDescriptor::RmlPropertyDescriptor(const std::string& name, MikanVariantType type)
	: m_propertyName(name)
	, m_dataType(type)
	, m_bIsReadable(true)
	, m_bIsWritable(true)
	, m_defaultValue(std::make_unique<MikanVariant>())
{}

RmlPropertyDescriptor::RmlPropertyDescriptor(const RmlPropertyDescriptor& other)
	: m_propertyName(other.m_propertyName)
	, m_bIsReadable(other.m_bIsReadable)
	, m_bIsWritable(other.m_bIsWritable)
	, m_defaultValue(std::make_unique<MikanVariant>(*(other.m_defaultValue.get())))
{}

RmlPropertyDescriptorPtr RmlPropertyDescriptor::setReadOnly()
{
	assert(m_dataType != MikanVariantType::INVALID);
	m_bIsReadable = true;
	m_bIsWritable = false;
	return shared_from_this();
}
