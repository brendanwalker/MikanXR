#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "CommonConfigFwd.h"
#include "ObjectFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "FunctionInterface.h"
#include "PropertyInterface.h"

#include <memory>
#include <typeinfo>

class MikanComponentDefinition : public CommonConfig
{
public:
	MikanComponentDefinition();
	MikanComponentDefinition(const std::string& componentName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_componentNamePropertyId;
	const std::string getComponentName() const { return m_componentName; }
	void setComponentName(const std::string& stencilName);

protected:
	std::string m_componentName;
};

class MikanComponent : 
	public std::enable_shared_from_this<MikanComponent>,
	public IPropertyInterface,
	public IFunctionInterface
{
public:
	MikanComponent(MikanObjectWeakPtr owner);

	inline bool getWasInitialized() const { return m_bWasInitialized; }
	inline bool getWasDisposed() const { return m_bWasDisposed; }
	
	virtual void setDefinition(MikanComponentDefinitionPtr config);
	virtual MikanComponentDefinitionPtr getDefinition() const { return m_definition; }

	void setName(const std::string& name);
	const std::string& getName() const { return m_name; }

	MikanObjectPtr getOwnerObject() const { return m_ownerObject.lock(); }

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
	virtual void update() {}

	// set m_bWantsCustomRender to true in constructor to make this function be called
	virtual void customRender() {}

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& propertyName) override;

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
