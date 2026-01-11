#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "CommonConfigFwd.h"
#include "IEntityAccessor.h"
#include "ObjectFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectManager.h"
#include "ScriptingFwd.h"

#include <filesystem>
#include <memory>
#include <typeinfo>

class MikanComponentDefinition : public CommonConfig
{
public:
	MikanComponentDefinition();
	MikanComponentDefinition(int componentId, const std::string& componentName);

	MikanComponentPtr getOwnerComponent() const { return m_ownerComponent.lock(); }
	void setOwnerComponent(MikanComponentPtr ownerComponent) { m_ownerComponent = ownerComponent; }

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_componentIdPropertyId;
	int getComponentId() const { return m_componentId; }

	static const std::string k_componentNamePropertyId;
	const std::string& getComponentName() const { return m_componentName; }
	void setComponentName(const std::string& stencilName);

	static const std::string k_componentScriptPathPropertyId;
	bool hasComponentScriptPath() const;
	std::filesystem::path getComponentScriptPath() const;
	void setComponentScriptPath(const std::filesystem::path& scriptPath);

protected:
	MikanComponentWeakPtr m_ownerComponent;
	int m_componentId;
	std::string m_componentName;
	AssetReferenceConfigPtr m_componentScriptAssetRefConfig;
};

class MikanComponent : 
	public std::enable_shared_from_this<MikanComponent>,
	public IEntityAccessor
{
public:
	MikanComponent(MikanObjectWeakPtr owner);

	class IMkWindow* getOwnerWindow() const;
	class IEditorWindow* getOwnerEditorWindow() const;

	inline bool getWasInitialized() const { return m_bWasInitialized; }
	inline bool getWasDisposed() const { return m_bWasDisposed; }
	
	virtual void setDefinition(MikanComponentDefinitionPtr config);
	virtual MikanComponentDefinitionPtr getDefinition() const { return m_definition; }

	inline static const std::string k_componentClassName = "MikanComponent";
	virtual std::string getComponentClassName() const { return k_componentClassName; }

	void setName(const std::string& name);
	const std::string& getName() const { return m_name; }

	MikanObjectPtr getOwnerObject() const { return m_ownerObject.lock(); }
	ProjectManagerPtr getOwnerProjectManager() const;

	template <class t_object_system_type>
	std::shared_ptr<t_object_system_type> getObjectSystemOfType() const
	{
		return getOwnerProjectManager()->getSystemOfType<t_object_system_type>();
	}

	template <class t_derived_type>
	std::shared_ptr<t_derived_type> getSelfPtr()
	{
		return std::dynamic_pointer_cast<t_derived_type>(shared_from_this());
	}

	template <class t_derived_type>
	std::shared_ptr<t_derived_type> getSelfPtr() const
	{
		return std::dynamic_pointer_cast<t_derived_type>(shared_from_this());
	}

	template <class t_derived_type>
	std::weak_ptr<t_derived_type> getSelfWeakPtr()
	{
		return getSelfPtr<t_derived_type>();
	}

	template <class t_derived_type>
	std::weak_ptr<t_derived_type> getSelfWeakPtr() const
	{
		return getSelfPtr<t_derived_type>();
	}

	virtual void init();
	virtual void dispose();
	
	// set m_bWantsUpdate to true in constructor to make this function be called
	virtual void update(float deltaSeconds) {}

	// set m_bWantsCustomRender to true in constructor to make this function be called
	virtual void customRender() {}

	// -- Scripting ----
	inline ComponentScriptContextPtr getScriptContext() { return m_scriptContext; }
	void reloadComponentScript();
	void addNewComponentScript();
	void removeComponentScript();

	// -- IEntityAccessor ----
	virtual CommonConfigPtr getEntityConfig() override { return m_definition; }

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_reloadScriptFunctionId;
	static const std::string k_addNewScriptFunctionId;
	static const std::string k_removeScriptFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

protected:
	virtual ComponentScriptContextPtr allocateScriptContext();
	void initScriptContext();
	void disposeScriptContext();
	void updateScriptContext(float deltaSeconds);
	virtual void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	bool m_bWasInitialized= false;
	bool m_bWasDisposed= false;
	bool m_bWantsUpdate= false;
	bool m_bWantsCustomRender= false;
	std::string m_name;
	MikanObjectWeakPtr m_ownerObject;
	MikanComponentDefinitionPtr m_definition;
	AssetReferencePtr m_scriptAssetRef;
	ComponentScriptContextPtr m_scriptContext;
};

template<class t_derived_type>
std::shared_ptr<t_derived_type> ComponentCast(MikanComponentPtr component)
{
	return std::dynamic_pointer_cast<t_derived_type>(component);
}

template<class t_component_type>
const char* ComponentTypeName(MikanComponentPtr component)
{
	return typeid(*component.get()).name();
}

template<class t_component_type>
const char* ComponentTypeName()
{
	return typeid(t_component_type).name();
}
