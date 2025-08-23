#include "ComponentProperty.h"

// Component Property
ComponentProperty::ComponentProperty(
	MikanComponent& ownerComponent,
	const std::string& propertyTypeString,
	const std::string& propertyName,
	ComponentPropertyAccessorPtr valueAccessor)
	: m_ownerComponent(ownerComponent)
	, m_propertyTypeString(propertyTypeString)
	, m_propertyName(propertyName)
	, m_valueAccessor(valueAccessor)
{
}