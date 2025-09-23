#include "ScriptAssetReference.h"
#include "MikanComponent.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "ObjectSystemManager.h"

#include "RmlUi/Core/Variant.h"
#include "RmlUi/Config/Config.h"


// -- MikanComponentConfig -----
const std::string MikanComponentDefinition::k_componentIdPropertyId = "component_id";
const std::string MikanComponentDefinition::k_componentNamePropertyId = "component_name";
const std::string MikanComponentDefinition::k_componentScriptPathPropertyId= "component_script";

MikanComponentDefinition::MikanComponentDefinition()
	: m_componentId(-1)
	, m_componentName()
	, m_componentScriptAssetRefConfig(std::make_shared<AssetReferenceConfig>("ComponentScript"))
{

}

MikanComponentDefinition::MikanComponentDefinition(
	int componentId,
	const std::string& componentName)
	: m_componentId(componentId)
	, m_componentName(componentName)
	, m_componentScriptAssetRefConfig(std::make_shared<AssetReferenceConfig>("ComponentScript"))
{
}

configuru::Config MikanComponentDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt[k_componentIdPropertyId] = m_componentId;
	pt[k_componentNamePropertyId] = m_componentName;
	if (m_componentScriptAssetRefConfig)
	{
		pt[k_componentScriptPathPropertyId] = m_componentScriptAssetRefConfig->writeToJSON();
	}

	return pt;
}

void MikanComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_componentId = pt.get_or<int>(k_componentIdPropertyId, -1);
	m_componentName = pt.get_or<std::string>(k_componentNamePropertyId, "");
	m_componentScriptAssetRefConfig = ScriptAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_componentScriptPathPropertyId))
	{
		m_componentScriptAssetRefConfig->readFromJSON(pt[k_componentScriptPathPropertyId]);
	}
}

void MikanComponentDefinition::setComponentName(const std::string& name)
{
	if (name != m_componentName)
	{
		m_componentName = name;
		markDirty(ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentNamePropertyId));
	}
}

bool MikanComponentDefinition::hasComponentScriptPath() const 
{ 
	return !m_componentScriptAssetRefConfig->assetPath.empty(); 
}

const std::filesystem::path MikanComponentDefinition::getComponentScriptPath() const 
{ 
	return m_componentScriptAssetRefConfig->assetPath; 
}

void MikanComponentDefinition::setComponentScriptPath(const std::filesystem::path& scriptPath)
{
	if (scriptPath.string() != m_componentScriptAssetRefConfig->assetPath)
	{
		m_componentScriptAssetRefConfig->assetPath= scriptPath.string();
		markDirty(ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentScriptPathPropertyId));
	}
}


// -- MikanComponent -----
MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
{
}

void MikanComponent::init()
{
	if (m_bWasInitialized)
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

	m_bWasInitialized= true;

	if (objectSystemPtr->OnComponentInitialized)
	{
		objectSystemPtr->OnComponentInitialized(objectSystemPtr, shared_from_this());
	}

	if (m_definition)
	{
		m_definition->OnMarkedDirty += MakeDelegate(this, &MikanComponent::onDefinitionMarkedDirty);
	}
}

void MikanComponent::dispose()
{
	if (m_bWasDisposed)
		return;

	m_bWasDisposed = true;

	MikanObjectSystemPtr objectSystemPtr= getOwnerObject()->getOwnerSystem();

	if (m_definition)
	{
		m_definition->OnMarkedDirty -= MakeDelegate(this, &MikanComponent::onDefinitionMarkedDirty);
	}

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
}

void MikanComponent::setDefinition(MikanComponentDefinitionPtr config)
{
	assert(!m_bWasInitialized);
	m_definition = config;

	// Make the component name match the config name
	m_name = config->getComponentName();
}

void MikanComponent::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
}

IMkWindow* MikanComponent::getOwnerWindow() const
{
	MikanObjectPtr ownerObject = m_ownerObject.lock();
	if (ownerObject)
	{
		return ownerObject->getOwnerSystem()->getOwnerObjectSystemManager()->getOwnerWindow();
	}

	return nullptr;
}

void MikanComponent::setName(const std::string& name)
{
	m_name= name;

	if (m_bWasInitialized)
		m_definition->setComponentName(name);
}

ObjectSystemManager* MikanComponent::getOwnerObjectSystemManager() const
{
	return getOwnerObject()->getOwnerSystem()->getOwnerObjectSystemManager();
}

// -- IRmlPropertyInterface ----
void MikanComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MikanComponentDefinition::k_componentIdPropertyId)
		->setReadOnly()
		->setDefaultInt(-1));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MikanComponentDefinition::k_componentNamePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MikanComponentDefinition::k_componentScriptPathPropertyId));
}

bool MikanComponent::getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MikanComponentDefinition::k_componentIdPropertyId)
	{
		outValue = m_definition->getComponentId();
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outValue= getName();
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		outValue = m_definition->getComponentScriptPath().string();
		return true;
	}

	return false;
}

bool MikanComponent::setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		setName(inValue.Get<Rml::String>());
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		if (inValue.GetType() == Rml::Variant::STRING)
		{
			std::filesystem::path scriptPath = inValue.Get<Rml::String>();
			m_definition->setComponentScriptPath(scriptPath);
			return true;
		}
		else if (inValue.GetType() == Rml::Variant::NONE)
		{
			m_definition->setComponentScriptPath(std::filesystem::path());
			return true;
		}
	}

	return false;
}

// -- IRmlFunctionInterface ----
void MikanComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
}

bool MikanComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	return false;
}