#include "AnchorObjectSystem.h"
#include "AssetReferencePropertyMetaData.h"
#include "CameraObjectSystem.h"
#include "ComponentScriptContext.h"
#include "CompositorObjectSystem.h"
#include "DMXObjectSystem.h"
#include "IEditorWindow.h"
#include "SceneObjectSystem.h"
#include "ScriptAssetReference.h"
#include "MikanComponent.h"
#include "MikanComponentTypes.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MikanPropertyTypes.h"
#include "MikanServer.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "OSUtils.h"
#include "PathUtils.h"
#include "ProjectManager.h"
#include "ScriptRequestHandler.h"
#include "StringUtils.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "tinyfiledialogs.h"
#include <fstream>

// -- MikanComponentConfig -----
const std::string MikanComponentDefinition::k_componentIdPropertyId= "component_id";
const std::string MikanComponentDefinition::k_componentNamePropertyId= "component_name";
const std::string MikanComponentDefinition::k_componentScriptPathPropertyId= "component_script";

MikanComponentDefinition::MikanComponentDefinition()
	: m_componentId(-1)
	, m_componentName()
	, m_componentScriptAssetRefConfig(ScriptAssetReferenceFactory().allocateAssetReferenceConfig())
{
}

MikanComponentDefinition::MikanComponentDefinition(int componentId, const std::string& componentName)
	: m_componentId(componentId)
	, m_componentName(componentName)
	, m_componentScriptAssetRefConfig(ScriptAssetReferenceFactory().allocateAssetReferenceConfig())
{
}

configuru::Config MikanComponentDefinition::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt[k_componentIdPropertyId]= m_componentId;
	pt[k_componentNamePropertyId]= m_componentName;
	if (m_componentScriptAssetRefConfig && m_componentScriptAssetRefConfig->isValid())
	{
		pt[k_componentScriptPathPropertyId]= m_componentScriptAssetRefConfig->writeToJSON();
	}

	return pt;
}

void MikanComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_componentId= pt.get_or<int>(k_componentIdPropertyId, -1);
	m_componentName= pt.get_or<std::string>(k_componentNamePropertyId, "");
	m_componentScriptAssetRefConfig= ScriptAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_componentScriptPathPropertyId))
	{
		m_componentScriptAssetRefConfig->readFromJSON(pt[k_componentScriptPathPropertyId]);
	}
}

bool MikanComponentDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
												  const Serialization::PolymorphicObjectPtr& initParams)
{
	const auto* componentValues= initParams.getTypedPointer<MikanComponentValues>();
	if (componentValues)
	{
		// Don't read the component ID since we already have an assigned component ID

		// Assign a component ID if we don't already have one
		const std::string desiredName= componentValues->component_name.getUtf8Value();
		if (!desiredName.empty())
		{
			m_componentName= componentValues->component_name.getUtf8Value();
		}

		// Create the script asset ref config if we don't already have one
		if (!m_componentScriptAssetRefConfig)
		{
			m_componentScriptAssetRefConfig= ScriptAssetReferenceFactory().allocateAssetReferenceConfig();
		}
		m_componentScriptAssetRefConfig->assetPath=
			PathUtils::utf8CStrToPathString(componentValues->component_script.getUtf8Value());
	}

	return true;
}

void MikanComponentDefinition::setComponentName(const std::string& name)
{
	if (name != m_componentName)
	{
		m_componentName= name;
		notifyPropertyChanged(
			ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentNamePropertyId));
	}
}

bool MikanComponentDefinition::hasComponentScriptPath() const
{
	return !m_componentScriptAssetRefConfig->assetPath.empty();
}

std::filesystem::path MikanComponentDefinition::getComponentScriptPath() const
{
	return m_componentScriptAssetRefConfig->assetPath;
}

void MikanComponentDefinition::setComponentScriptPath(const std::filesystem::path& scriptPath)
{
	if (scriptPath.string() != m_componentScriptAssetRefConfig->assetPath)
	{
		m_componentScriptAssetRefConfig->assetPath= scriptPath.string();
		notifyPropertyChanged(
			ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentScriptPathPropertyId));
	}
}

// -- MikanComponent -----
MikanComponent::MikanComponent(MikanObjectWeakPtr owner)
	: m_ownerObject(owner)
	, m_scriptAssetRef(ScriptAssetReferenceFactory().allocateAssetReference())
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

	m_bWasInitialized= true;

	if (objectSystemPtr->OnComponentInitialized)
	{
		objectSystemPtr->OnComponentInitialized(objectSystemPtr, shared_from_this());
	}

	if (m_definition)
	{
		// If the component definition has a script path, copy it to the script asset ref
		const std::filesystem::path scriptPath= m_definition->getComponentScriptPath();
		if (!scriptPath.empty())
		{
			// Copy the script path from the definition to the script asset ref
			m_scriptAssetRef->setAssetPath(scriptPath);

			// Initialize the script context
			initScriptContext();
		}

		// Listen for definition changes
		m_definition->OnPropertyChanged+= MakeDelegate(this, &MikanComponent::onDefinitionMarkedDirty);
	}
}

void MikanComponent::postInit() {}

void MikanComponent::dispose()
{
	if (m_bWasDisposed)
		return;

	m_bWasDisposed= true;

	MikanObjectSystemPtr objectSystemPtr= getOwnerObject()->getOwnerSystem();

	disposeScriptContext();

	if (m_definition)
	{
		m_definition->OnPropertyChanged-= MakeDelegate(this, &MikanComponent::onDefinitionMarkedDirty);
	}

	if (objectSystemPtr->OnComponentDisposed)
	{
		objectSystemPtr->OnComponentDisposed(objectSystemPtr, shared_from_this());
	}

	if (m_bWantsUpdate)
	{
		objectSystemPtr->onUpdate-= MakeDelegate(this, &MikanComponent::update);
	}

	if (onDisposed)
	{
		onDisposed(this);
	}
}

void MikanComponent::setDefinition(MikanComponentDefinitionPtr config)
{
	assert(!m_bWasInitialized);
	m_definition= config;
	m_definition->setOwnerComponent(getSelfPtr<MikanComponent>());

	// Make the component name match the config name
	m_name= config->getComponentName();
}

void MikanComponent::onDefinitionMarkedDirty(CommonConfigPtr configPtr,
											 const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentScriptPathPropertyId))
	{
		// Copy the script path, if any, from the definition to the script asset ref
		m_scriptAssetRef->setAssetPath(m_definition->getComponentScriptPath());

		// (Re)Initialize the script context
		initScriptContext();
	}
}

int MikanComponent::getComponentId() const
{
	if (m_definition)
	{
		return m_definition->getComponentId();
	}

	return INVALID_MIKAN_ID;
}

IEditorWindow* MikanComponent::getOwnerEditorWindow() const
{
	MikanObjectPtr ownerObject= m_ownerObject.lock();
	if (ownerObject)
	{
		return ownerObject->getOwnerSystem()->getOwnerProjectManager()->getOwnerWindow();
	}

	return nullptr;
}

IMkGraphicsContext* MikanComponent::getGraphicsContext() const
{
	IEditorWindow* ownerEditorWindow= getOwnerEditorWindow();
	if (ownerEditorWindow)
	{
		return ownerEditorWindow->getGraphicsContext().get();
	}

	return nullptr;
}

void MikanComponent::setName(const std::string& name)
{
	m_name= name;

	if (m_bWasInitialized)
		m_definition->setComponentName(name);
}

ProjectManagerPtr MikanComponent::getOwnerProjectManager() const
{
	return getOwnerObject()->getOwnerSystem()->getOwnerProjectManager();
}

bool MikanComponent::destroyOwnerObject()
{
	MikanObjectPtr ownerObject= getOwnerObject();
	if (ownerObject)
	{
		auto ownerSystem= ownerObject->getOwnerSystem();
		if (ownerSystem)
		{
			return ownerSystem->deleteObject(ownerObject);
		}
	}

	return false;
}

// -- Lua Binding ----
void MikanComponent::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<MikanComponent>("MikanComponent")
		.addProperty("name", &MikanComponent::getName, &MikanComponent::setName)
		.addProperty("className", &MikanComponent::getComponentClassName)
		.addFunction("getCameraSystem", [](MikanComponent* c) -> CameraObjectSystem*
					 { return c->getObjectSystemOfType<CameraObjectSystem>().get(); })
		.addFunction("getSceneSystem", [](MikanComponent* c) -> SceneObjectSystem*
					 { return c->getObjectSystemOfType<SceneObjectSystem>().get(); })
		.addFunction("getDMXSystem", [](MikanComponent* c) -> DMXObjectSystem*
					 { return c->getObjectSystemOfType<DMXObjectSystem>().get(); })
		.addFunction("getAnchorSystem", [](MikanComponent* c) -> AnchorObjectSystem*
					 { return c->getObjectSystemOfType<AnchorObjectSystem>().get(); })
		.addFunction("getCompositorSystem", [](MikanComponent* c) -> CompositorObjectSystem*
					 { return c->getObjectSystemOfType<CompositorObjectSystem>().get(); })
		.endClass();
}

bool MikanComponent::hasValidComponentScript() const
{
	return m_scriptContext != nullptr ? m_scriptContext->hasLoadedScript() : false;
}

void MikanComponent::addNewComponentScript()
{
	const std::filesystem::path scriptsDir= PathUtils::getProjectDirectory() / "scripts";
	std::filesystem::create_directories(scriptsDir);

	const std::string prefix= getComponentClassName() + std::to_string(getDefinition()->getComponentId());
	const std::filesystem::path scriptPath= PathUtils::makeTimestampedFilePath(scriptsDir, prefix, ".lua");

	// Create an empty Lua file at the new path
	std::ofstream(scriptPath).flush();

	getDefinition()->setComponentScriptPath(scriptPath);
}

void MikanComponent::editComponentScript()
{
	const auto scriptPath= getDefinition()->getComponentScriptPath();
	if (scriptPath.empty())
		return;

	const std::string editorCmd= App::getInstance()->getAppSettings()->getScriptEditorCommand();
	OSUtils::openFileWithApplication(scriptPath, editorCmd);
}

void MikanComponent::reloadComponentScript()
{
	if (m_scriptContext)
	{
		// Unbind before reloading so any HTTP trigger routes the previous script version
		// registered are dropped, then rebind afterward to pick up whatever the reloaded
		// script (re)registers.
		MikanServer::getInstance()->getScriptRequestHandler()->unbindScriptContect(m_scriptContext);
		m_scriptContext->reloadScript();
		MikanServer::getInstance()->getScriptRequestHandler()->bindScriptContect(m_scriptContext);
	}
}

void MikanComponent::removeComponentScript()
{
	disposeScriptContext();

	getDefinition()->setComponentScriptPath(std::filesystem::path());
}

void MikanComponent::selectComponentScript()
{
	ScriptAssetReferenceFactory assetRefFactory;
	const char* picked= tinyfd_openFileDialog(
		assetRefFactory.getFileDialogTitle(), assetRefFactory.getDefaultPath(), assetRefFactory.getFilterPatternCount(),
		assetRefFactory.getFilterPatterns(), assetRefFactory.getFilterDescription(), 1);

	if (picked != nullptr && picked[0] != '\0')
	{
		std::filesystem::path newAssetPath(picked);

		if (m_scriptAssetRef->getResolvedAssetPath() != newAssetPath)
		{
			// Set the new script path in the component definition
			// This will trigger the script context to be initialized via the definition dirty event
			getDefinition()->setComponentScriptPath(newAssetPath);
		}
	}
}

ComponentScriptContextPtr MikanComponent::allocateScriptContext()
{
	return std::make_shared<ComponentScriptContext>(getSelfPtr<MikanComponent>());
}

void MikanComponent::initScriptContext()
{
	disposeScriptContext();

	// Get the script path from the component definition
	std::filesystem::path scriptPath= m_scriptAssetRef->getResolvedAssetPath();
	if (scriptPath.empty())
		return;

	// Create and load the script context
	m_scriptContext= allocateScriptContext();
	if (!m_scriptContext->loadScript(scriptPath))
	{
		MIKAN_LOG_WARNING("MikanComponent::initScriptContext") << "Failed to load script " << scriptPath;

		// Failed to load script
		m_scriptContext= nullptr;
		return;
	}

	// Register the script context with the mikan server
	MikanServer::getInstance()->getScriptRequestHandler()->bindScriptContect(m_scriptContext);

	// Register update callback for script context
	getOwnerObject()->getOwnerSystem()->onUpdate+= MakeDelegate(this, &MikanComponent::updateScriptContext);
}

void MikanComponent::updateScriptContext(float deltaSeconds)
{
	if (m_scriptContext)
	{
		m_scriptContext->updateScript(deltaSeconds);
	}
}

void MikanComponent::disposeScriptContext()
{
	if (!m_scriptContext)
		return;

	// Unregister update callback for script context
	getOwnerObject()->getOwnerSystem()->onUpdate-= MakeDelegate(this, &MikanComponent::updateScriptContext);

	// Unregister the script context with the mikan server
	MikanServer::getInstance()->getScriptRequestHandler()->unbindScriptContect(m_scriptContext);

	// Clean up the script context
	m_scriptContext= nullptr;
}

// -- IEntityAccessor ----
std::string MikanComponent::makePropertyUIIdentifier(const std::string& propName) const
{
	return StringUtils::stringify(getComponentClassName(), getComponentId(), "_", propName);
}

rfk::Struct const* MikanComponent::getClientAPIValuesStructType() const
{
	return &MikanComponentValues::staticGetArchetype();
}

// -- IPropertyInterface ----
void MikanComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(MikanComponentDefinition::k_componentIdPropertyId, MikanVariantType::INT)
			->setReadOnly()
			->setDefaultValue(-1)
			->setUIHidden());
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(MikanComponentDefinition::k_componentNamePropertyId,
																  MikanVariantType::STRING));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 MikanComponentDefinition::k_componentScriptPathPropertyId, MikanVariantType::STRING)
								 ->addMetaData(std::make_shared<AssetReferenceFactoryMetaData>(
									 AssetReferenceFactory::createFactory<ScriptAssetReferenceFactory>())));
}

bool MikanComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == MikanComponentDefinition::k_componentIdPropertyId)
	{
		outValue= m_definition->getComponentId();
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		outValue= getName();
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		outValue= m_definition->getComponentScriptPath();
		return true;
	}

	return false;
}

bool MikanComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == MikanComponentDefinition::k_componentNamePropertyId)
	{
		setName(inValue.getUtf8Value());
		return true;
	}
	else if (propertyName == MikanComponentDefinition::k_componentScriptPathPropertyId)
	{
		if (inValue.value_type == MikanVariantType::STRING)
		{
			std::filesystem::path scriptPath= inValue.getUtf8Value();
			m_definition->setComponentScriptPath(scriptPath);
			return true;
		}
		else if (inValue.value_type == MikanVariantType::INVALID)
		{
			m_definition->setComponentScriptPath(std::filesystem::path());
			return true;
		}
	}

	return false;
}

// -- IFunctionInterface ----
const std::string MikanComponent::k_reloadScriptFunctionId= "reload_script";
const std::string MikanComponent::k_editScriptFunctionId= "edit_script";
const std::string MikanComponent::k_addNewScriptFunctionId= "add_new_script";
const std::string MikanComponent::k_removeScriptFunctionId= "remove_script";
const std::string MikanComponent::k_selectScriptFunctionId= "select_script";

void MikanComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_addNewScriptFunctionId, "Add New Script")->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_editScriptFunctionId, "Add New Script")->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_reloadScriptFunctionId, "Reload Script")->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_removeScriptFunctionId, "Remove Script")->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_removeScriptFunctionId, "Select Script")->setUIHidden());
}

bool MikanComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == MikanComponent::k_addNewScriptFunctionId)
	{
		addNewComponentScript();
		return true;
	}
	else if (functionName == MikanComponent::k_editScriptFunctionId)
	{
		editComponentScript();
		return true;
	}
	else if (functionName == MikanComponent::k_reloadScriptFunctionId)
	{
		reloadComponentScript();
		return true;
	}
	else if (functionName == MikanComponent::k_removeScriptFunctionId)
	{
		removeComponentScript();
		return true;
	}
	else if (functionName == MikanComponent::k_selectScriptFunctionId)
	{
		selectComponentScript();
		return true;
	}

	return false;
}