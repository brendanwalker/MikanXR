#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "CommonConfigFwd.h"
#include "ObjectFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemManager.h"
#include "RmlFunctionInterface.h"
#include "RmlPropertyInterface.h"

#include <filesystem>
#include <memory>
#include <typeinfo>

class MikanComponentDefinition : public CommonConfig
{
public:
	MikanComponentDefinition();
	MikanComponentDefinition(int componentId, const std::string& componentName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_componentIdPropertyId;
	int getComponentId() const { return m_componentId; }

	static const std::string k_componentNamePropertyId;
	const std::string& getComponentName() const { return m_componentName; }
	void setComponentName(const std::string& stencilName);

	static const std::string k_componentScriptPathPropertyId;
	bool hasComponentScriptPath() const;
	const std::filesystem::path getComponentScriptPath() const;
	void setComponentScriptPath(const std::filesystem::path& scriptPath);

protected:
	int m_componentId;
	std::string m_componentName;
	AssetReferenceConfigPtr m_componentScriptAssetRefConfig;
};

class MikanComponent : 
	public std::enable_shared_from_this<MikanComponent>,
	public IRmlPropertyInterface,
	public IRmlFunctionInterface
{
public:
	MikanComponent(MikanObjectWeakPtr owner);

	class IMkWindow* getOwnerWindow() const;

	inline bool getWasInitialized() const { return m_bWasInitialized; }
	inline bool getWasDisposed() const { return m_bWasDisposed; }
	
	virtual void setDefinition(MikanComponentDefinitionPtr config);
	virtual MikanComponentDefinitionPtr getDefinition() const { return m_definition; }

	void setName(const std::string& name);
	const std::string& getName() const { return m_name; }

	MikanObjectPtr getOwnerObject() const { return m_ownerObject.lock(); }
	ObjectSystemManager* getOwnerObjectSystemManager() const;

	template <class t_object_system_type>
	std::shared_ptr<t_object_system_type> getObjectSystemOfType() const
	{
		return getOwnerObjectSystemManager()->getSystemOfType<t_object_system_type>();
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

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

protected:
	virtual void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	bool m_bWasInitialized= false;
	bool m_bWasDisposed= false;
	bool m_bWantsUpdate= false;
	bool m_bWantsCustomRender= false;
	std::string m_name;
	MikanObjectWeakPtr m_ownerObject;
	MikanComponentDefinitionPtr m_definition;
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
