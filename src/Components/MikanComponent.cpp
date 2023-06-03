#include "MikanComponent.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"

#include "RmlUi/Core/Variant.h"
#include "RmlUi/Config/Config.h"

const std::string MikanComponent::k_componentNamePropertyId= "name";

MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
{
}

void MikanComponent::init()
{
	if (m_bIsInitialized)
		return;

	MikanObjectSystemPtr objectSystemPtr= getOwnerObject()->getOwnerSystem();

	if (m_bWantsUpdate)
	{
		objectSystemPtr->onUpdate+= MakeDelegate(this, &MikanComponent::update);
	}

	if (m_bWantsCustomRender)
	{
		objectSystemPtr->onCustomRender += MakeDelegate(this, &MikanComponent::customRender);
	}

	m_bIsInitialized= true;

	if (objectSystemPtr->OnComponentInitialized)
	{
		objectSystemPtr->OnComponentInitialized(objectSystemPtr, shared_from_this());
	}
}

void MikanComponent::dispose()
{
	if (m_bIsDisposed)
		return;

	MikanObjectSystemPtr objectSystemPtr= getOwnerObject()->getOwnerSystem();

	if (objectSystemPtr->OnComponentDisposed)
	{
		objectSystemPtr->OnComponentDisposed(objectSystemPtr, shared_from_this());
	}

	if (m_bWantsUpdate)
	{
		objectSystemPtr->onUpdate -= MakeDelegate(this, &MikanComponent::update);
	}

	if (m_bWantsCustomRender)
	{
		objectSystemPtr->onCustomRender -= MakeDelegate(this, &MikanComponent::customRender);
	}

	m_bIsDisposed= true;
}

// -- IPropertyInterface ----
void MikanComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	outPropertyNames.push_back(k_componentNamePropertyId);
}

bool MikanComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (propertyName == k_componentNamePropertyId)
	{
		outDescriptor= {k_componentNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name};
		return true;
	}

	return false;
}

bool MikanComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (propertyName == k_componentNamePropertyId)
	{
		outValue= getName();
		return true;
	}

	return false;
}

bool MikanComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	return false;
}

bool MikanComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (propertyName == k_componentNamePropertyId)
	{
		setName(inValue.Get<Rml::String>());
		return true;
	}

	return false;
}