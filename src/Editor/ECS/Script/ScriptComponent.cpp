#include "ScriptComponent.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "AssetReferencePropertyMetaData.h"
#include "ProjectScriptContext.h"
#include "MikanObject.h"
#include "MikanScriptTypes.h"
#include "OSUtils.h"
#include "PathUtils.h"
#include "ScriptAssetReference.h"
#include "ScriptObjectSystem.h"

#include "tinyfiledialogs.h"

// -- ScriptDefinition -----
const std::string ScriptDefinition::k_scriptPathPropertyId= "script_path";

ScriptDefinition::ScriptDefinition()
	: MikanComponentDefinition()
	, m_scriptAssetRefConfig(ScriptAssetReferenceFactory().allocateAssetReferenceConfig())
{
}

ScriptDefinition::ScriptDefinition(MikanScriptID scriptId)
	: MikanComponentDefinition(scriptId, "")
	, m_scriptAssetRefConfig(ScriptAssetReferenceFactory().allocateAssetReferenceConfig())
{
}

configuru::Config ScriptDefinition::writeToJSON()
{
	configuru::Config pt= MikanComponentDefinition::writeToJSON();

	if (m_scriptAssetRefConfig->isValid())
	{
		pt[k_scriptPathPropertyId]= m_scriptAssetRefConfig->writeToJSON();
	}

	return pt;
}

void ScriptDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_scriptAssetRefConfig= ScriptAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_scriptPathPropertyId))
	{
		m_scriptAssetRefConfig->readFromJSON(pt[k_scriptPathPropertyId]);
	}
}

bool ScriptDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
										  const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!MikanComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues= initParams.getTypedPointer<MikanScriptComponentValues>();
	if (componentValues)
	{
		m_scriptAssetRefConfig->assetPath= PathUtils::utf8CStrToPathString(componentValues->script_path.getUtf8Value());
	}

	return true;
}

bool ScriptDefinition::hasScriptPath() const { return !m_scriptAssetRefConfig->assetPath.empty(); }

std::filesystem::path ScriptDefinition::getScriptPath() const { return m_scriptAssetRefConfig->assetPath; }

void ScriptDefinition::setScriptPath(const std::filesystem::path& scriptPath)
{
	if (scriptPath.string() != m_scriptAssetRefConfig->assetPath)
	{
		m_scriptAssetRefConfig->assetPath= scriptPath.string();
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_scriptPathPropertyId));
	}
}

// -- ScriptComponent -----
ScriptComponent::ScriptComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, m_scriptAssetRef(ScriptAssetReferenceFactory().allocateAssetReference())
{
}

void ScriptComponent::init()
{
	MikanComponent::init();

	m_scriptAssetRef->setAssetPath(getScriptDefinition()->getScriptPath());

	// The project context reloads on the system's next update so a script
	// added mid-frame never tears down a running state
	if (ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem())
	{
		scriptSystem->requestReload();
	}
}

void ScriptComponent::dispose()
{
	if (m_bWasDisposed)
		return;

	if (ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem())
	{
		scriptSystem->requestReload();
	}

	MikanComponent::dispose();
}

ScriptObjectSystemPtr ScriptComponent::getOwnerScriptSystem() const
{
	MikanObjectPtr ownerObject= getOwnerObject();
	if (!ownerObject)
		return nullptr;

	return std::dynamic_pointer_cast<ScriptObjectSystem>(ownerObject->getOwnerSystem());
}

std::filesystem::path ScriptComponent::getResolvedScriptPath() const
{
	return m_scriptAssetRef->getResolvedAssetPath();
}

bool ScriptComponent::isScriptLoaded() const
{
	ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem();
	CommonScriptContextPtr scriptContext= scriptSystem ? scriptSystem->getScriptContext() : nullptr;

	return scriptContext && scriptContext->isScriptLoaded(getComponentId());
}

void ScriptComponent::getTriggerNames(std::vector<std::string>& outNames) const
{
	ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem();
	CommonScriptContextPtr scriptContext= scriptSystem ? scriptSystem->getScriptContext() : nullptr;

	if (scriptContext)
	{
		scriptContext->getTriggerNamesForScript(getComponentId(), outNames);
	}
}

bool ScriptComponent::invokeTrigger(const std::string& triggerName)
{
	ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem();
	CommonScriptContextPtr scriptContext= scriptSystem ? scriptSystem->getScriptContext() : nullptr;

	return scriptContext && scriptContext->invokeScriptTrigger(triggerName);
}

void ScriptComponent::editScript()
{
	if (!getScriptDefinition()->hasScriptPath())
		return;

	const std::filesystem::path scriptPath= getResolvedScriptPath();
	const std::string editorCmd= App::getInstance()->getAppSettings()->getScriptEditorCommand();

	// A script inside the project opens with the project folder ahead of it, so
	// the editor lands in the project workspace (where the generated .luarc.json
	// and launch config live) with the file focused
	const std::filesystem::path projectDir= PathUtils::getProjectDirectory();
	const std::filesystem::path relPath=
		projectDir.empty() ? std::filesystem::path() : scriptPath.lexically_relative(projectDir);
	const bool isUnderProject=
		!relPath.empty() && relPath.native().substr(0, 2) != L".." && relPath.native().front() != L'/';
	if (isUnderProject)
	{
		OSUtils::openPathsWithApplication({projectDir, scriptPath}, editorCmd);
	}
	else
	{
		OSUtils::openFileWithApplication(scriptPath, editorCmd);
	}
}

void ScriptComponent::reloadScript()
{
	if (ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem())
	{
		scriptSystem->reloadAllScripts();
	}
}

void ScriptComponent::selectScript()
{
	ScriptAssetReferenceFactory assetRefFactory;
	const char* picked= tinyfd_openFileDialog(
		assetRefFactory.getFileDialogTitle(), assetRefFactory.getDefaultPath(), assetRefFactory.getFilterPatternCount(),
		assetRefFactory.getFilterPatterns(), assetRefFactory.getFilterDescription(), 1);

	if (picked != nullptr && picked[0] != '\0')
	{
		// The definition change reloads the project context through onDefinitionMarkedDirty
		getScriptDefinition()->setScriptPath(std::filesystem::path(picked));
	}
}

void ScriptComponent::onDefinitionMarkedDirty(CommonConfigPtr configPtr,
											  const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(ScriptDefinition::k_scriptPathPropertyId))
	{
		m_scriptAssetRef->setAssetPath(getScriptDefinition()->getScriptPath());

		if (ScriptObjectSystemPtr scriptSystem= getOwnerScriptSystem())
		{
			scriptSystem->requestReload();
		}
	}
}

// -- IEntityAccessor ----
rfk::Struct const* ScriptComponent::getClientAPIValuesStructType() const
{
	return &MikanScriptComponentValues::staticGetArchetype();
}

// -- IPropertyInterface ----
void ScriptComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(ScriptDefinition::k_scriptPathPropertyId, MikanVariantType::STRING)
			->addMetaData(std::make_shared<AssetReferenceFactoryMetaData>(
				AssetReferenceFactory::createFactory<ScriptAssetReferenceFactory>())));
}

bool ScriptComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == ScriptDefinition::k_scriptPathPropertyId)
	{
		outValue= getScriptDefinition()->getScriptPath();
		return true;
	}

	return MikanComponent::getPropertyValue(propertyName, outValue);
}

bool ScriptComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == ScriptDefinition::k_scriptPathPropertyId)
	{
		if (inValue.value_type == MikanVariantType::STRING)
		{
			getScriptDefinition()->setScriptPath(std::filesystem::path(inValue.getUtf8Value()));
			return true;
		}
		else if (inValue.value_type == MikanVariantType::INVALID)
		{
			getScriptDefinition()->setScriptPath(std::filesystem::path());
			return true;
		}

		return false;
	}

	return MikanComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string ScriptComponent::k_editScriptFunctionId= "edit_script";
const std::string ScriptComponent::k_reloadScriptFunctionId= "reload_script";
const std::string ScriptComponent::k_selectScriptFunctionId= "select_script";

void ScriptComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getFunctionDescriptors(outDescriptors);

	// The script panel draws these itself, beside the path row
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_editScriptFunctionId, "Edit Script")->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_reloadScriptFunctionId, "Reload Script")->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_selectScriptFunctionId, "Select Script")->setUIHidden());
}

bool ScriptComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == k_editScriptFunctionId)
	{
		editScript();
		return true;
	}
	else if (functionName == k_reloadScriptFunctionId)
	{
		reloadScript();
		return true;
	}
	else if (functionName == k_selectScriptFunctionId)
	{
		selectScript();
		return true;
	}

	return MikanComponent::invokeFunction(functionName);
}
