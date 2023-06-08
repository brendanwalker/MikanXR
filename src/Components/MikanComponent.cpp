#include "MikanComponent.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"

#include "RmlUi/Core/Variant.h"
#include "RmlUi/Config/Config.h"


// -- MikanComponentConfig -----
const std::string MikanComponentDefinition::k_componentNamePropertyId = "name";

MikanComponentDefinition::MikanComponentDefinition()
	: m_componentName()
{

}

MikanComponentDefinition::MikanComponentDefinition(const std::string& componentName)
	: m_componentName(componentName)
{
}

configuru::Config MikanComponentDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt[k_componentNamePropertyId] = m_componentName;

	return pt;
}

void MikanComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_componentName = pt.get_or<std::string>(k_componentNamePropertyId, "");
}

void MikanComponentDefinition::setComponentName(const std::string& name)
{
	m_componentName= name;
	markDirty(ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentNamePropertyId));
}

// -- MikanComponent -----
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

void MikanComponent::setDefinition(MikanComponentDefinitionPtr config)
{
	assert(!m_bIsInitialized);
	m_definition = config;

	// Make the component name match the config name
	m_name = config->getComponentName();
}

void MikanComponent::setName(const std::string& name)
{
	m_name= name;

	if (m_bIsInitialized)
		m_definition->setComponentName(name);
}


// -- IPropertyInterface ----
void MikanComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	outPropertyNames.push_back(MikanComponentDefinition::k_componentNamePropertyId);
}

bool MikanComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outDescriptor= {MikanComponentDefinition::k_componentNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name};
		return true;
	}

	return false;
}

bool MikanComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
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
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		setName(inValue.Get<Rml::String>());
		return true;
	}

	return false;
}