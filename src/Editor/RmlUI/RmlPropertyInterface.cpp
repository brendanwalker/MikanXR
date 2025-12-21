#include "RmlPropertyInterface.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

RmlPropertyDescriptor::RmlPropertyDescriptor()
	: m_propertyName("")
	, m_isReadOnly(false)
	, m_defaultValue(std::make_unique<Rml::Variant>())
{

}

RmlPropertyDescriptor::RmlPropertyDescriptor(const std::string& name)
	: m_propertyName(name)
	, m_isReadOnly(false) 
	, m_defaultValue(std::make_unique<Rml::Variant>())
{}