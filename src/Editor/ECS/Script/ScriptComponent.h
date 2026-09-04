#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ScriptVariableTable.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// One Lua script file registered to the project. Every script runs in the
// project's single script context, in pool order. The definition also owns
// the values of the variables the script registers.
class ScriptDefinition : public MikanComponentDefinition, public IScriptVariableStore
{
public:
	ScriptDefinition();
	ScriptDefinition(MikanScriptID scriptId);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;
	virtual bool readFromInitParams(MikanObjectSystem* ownerObjectSystem,
									const Serialization::PolymorphicObjectPtr& initParams) override;

	inline MikanScriptID getScriptId() const { return getComponentId(); }

	static const std::string k_scriptPathPropertyId;
	bool hasScriptPath() const;
	std::filesystem::path getScriptPath() const;
	void setScriptPath(const std::filesystem::path& scriptPath);

	// Every variable edit notifies this one name, so the whole table is the
	// unit the transaction recorder captures and re-applies
	static const std::string k_scriptVariablesPropertyId;
	inline const ScriptVariableTable& getScriptVariables() const { return m_scriptVariables; }
	bool getScriptVariable(const std::string& name, MikanVariant& outValue) const;
	// Replace the whole table with one notification
	void setScriptVariables(const ScriptVariableTable& table);

	// -- IScriptVariableStore ----
	virtual bool getScriptVariableOfType(const std::string& name, MikanVariantType type,
										 MikanVariant& outValue) const override;
	virtual void setScriptVariable(const std::string& name, const MikanVariant& value) override;

private:
	AssetReferenceConfigPtr m_scriptAssetRefConfig;
	ScriptVariableTable m_scriptVariables;
};

class ScriptComponent : public MikanComponent
{
public:
	ScriptComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;

	inline static const std::string k_componentClassName= "ScriptComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	ScriptObjectSystemPtr getOwnerScriptSystem() const;
	inline ScriptDefinitionPtr getScriptDefinition() const
	{
		return std::static_pointer_cast<ScriptDefinition>(m_definition);
	}

	// The definition's path resolved against the project directory
	std::filesystem::path getResolvedScriptPath() const;
	// True once this script's chunk has run in the project context
	bool isScriptLoaded() const;
	void getTriggerNames(std::vector<std::string>& outNames) const;
	bool invokeTrigger(const std::string& triggerName);

	// The variables this script's chunk registered, in registration order;
	// empty while the script is not loaded
	void getScriptVariableNames(std::vector<std::string>& outNames) const;
	bool getScriptVariable(const std::string& name, MikanVariant& outValue) const;
	// Writes the definition; the Lua global follows through onDefinitionMarkedDirty
	bool setScriptVariable(const std::string& name, const MikanVariant& value);

	void editScript();
	void reloadScript();
	void selectScript();

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_editScriptFunctionId;
	static const std::string k_reloadScriptFunctionId;
	static const std::string k_selectScriptFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(const std::string& functionName) override;

protected:
	virtual void onDefinitionMarkedDirty(CommonConfigPtr configPtr,
										 const ConfigPropertyChangeSet& changedPropertySet) override;

private:
	// Write every stored value this script registered into its Lua global
	void pushScriptVariablesToContext();

	AssetReferencePtr m_scriptAssetRef;
};
