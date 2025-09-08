#include "ScriptAssetReference.h"
#include "MikanComponent.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "ObjectSystemManager.h"

#include "RmlUi/Core/Variant.h"
#include "RmlUi/Config/Config.h"


// -- MikanComponentConfig -----
const std::string MikanComponentDefinition::k_componentNamePropertyId = "name";
const std::string MikanComponentDefinition::k_componentScriptPathPropertyId= "component_script";

MikanComponentDefinition::MikanComponentDefinition()
	: m_componentName()
	, m_componentScriptAssetRefConfig(std::make_shared<AssetReferenceConfig>("ComponentScript"))
{

}

MikanComponentDefinition::MikanComponentDefinition(const std::string& componentName)
	: m_componentName(componentName)
	, m_componentScriptAssetRefConfig(std::make_shared<AssetReferenceConfig>("ComponentScript"))
{
}

configuru::Config MikanComponentDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

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
	if (OnPropertyChanged)
	{
		// TODO: Only notify for property names that are actually exposed in getPropertyNames()
		for (const std::string& changedPropertyName : changedPropertySet.getSet())
		{
			OnPropertyChanged(changedPropertyName);
		}
	}
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

// -- IPropertyInterface ----
void MikanComponent::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	outPropertyNames.push_back(MikanComponentDefinition::k_componentNamePropertyId);
	outPropertyNames.push_back(MikanComponentDefinition::k_componentScriptPathPropertyId);
}

void MikanComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	getPropertyNamesStatic(outPropertyNames);
}

bool MikanComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outDescriptor= {MikanComponentDefinition::k_componentNamePropertyId, ePropertyDataType::datatype_string, ePropertySemantic::name};
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		outDescriptor = {MikanComponentDefinition::k_componentScriptPathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
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
	else if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		outValue = m_definition->getComponentScriptPath().string();
		return true;
	}

	return false;
}

bool MikanComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		if (attributeName == *k_PropertyAttributeFileBrowseTitle)
		{
			outValue = "Select a script";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilter)
		{
			outValue = ".lua";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilterDesc)
		{
			outValue = "Lua Script Files (.lua)";
		}
	}

	return false;
}

bool MikanComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
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

// -- IFunctionInterface ----
void MikanComponent::getFunctionNamesStatic(std::vector<std::string>& outPropertyNames)
{
}

void MikanComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	getFunctionNamesStatic(outPropertyNames);
}

bool MikanComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	return false;
}

bool MikanComponent::invokeFunction(const std::string& propertyName)
{
	return false;
}