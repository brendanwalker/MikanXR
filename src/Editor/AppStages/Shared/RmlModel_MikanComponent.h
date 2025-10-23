#pragma once

#include "ComponentFwd.h"
#include "ComponentScriptContext.h"
#include "MikanComponent.h"
#include "RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlModel_PropertyInterface.h"

#include <memory>

template <class t_component_type>
class RmlModel_TypedMikanComponent
{
public:
	RmlModel_TypedMikanComponent()
		: m_component()
		, m_propertyInterface(std::make_shared<RmlModel_PropertyInterface>())
		, m_scriptTriggerList(std::make_shared<RmlDataBinding_ScriptTriggerList>())
	{
	}

	bool init(Rml::Context* rmlContext)
	{
		return
			m_propertyInterface->init<t_component_type>(
				rmlContext,
				t_component_type::k_componentClassName,
				[this](Rml::DataModelConstructor& constructor) -> bool
				{
					return onConstruct(constructor);
				});
	}

	virtual bool onConstruct(Rml::DataModelConstructor& constructor)
	{
		m_scriptTriggerList->init(
			constructor,
			CommonConfigPtr(),
			"script_triggers",
			[this](CommonConfigPtr ownerConfig, Rml::Vector<std::string>& outScriptTriggerList) {				
				ComponentScriptContextPtr scriptContext = getComponent()->getScriptContext();
				if (scriptContext)
				{
					outScriptTriggerList= scriptContext->getScriptTriggers();
				}
			});

		constructor.BindEventCallback(
			"reload_script",
			[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
				getComponent()->reloadComponentScript();
			});
		constructor.BindEventCallback(
			"add_new_script",
			[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
				getComponent()->addNewComponentScript();
			});
		constructor.BindEventCallback(
			"invoke_script_trigger",
			[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
				ComponentScriptContextPtr scriptContext = getComponent()->getScriptContext();
				if (arguments.size() == 1 && scriptContext)
				{
					scriptContext->invokeScriptTrigger(arguments[0].Get<Rml::String>());
				}
			});

		return true; 
	}

	MikanComponentPtr getComponent() const
	{
		return m_component.lock();
	}

	virtual bool setComponent(MikanComponentPtr component)
	{
		MikanComponentPtr oldComponent = m_component.lock();

		if (component != oldComponent)
		{
			m_component = component;
			m_propertyInterface->setPropertyInterface(component, component->getDefinition());
			m_propertyInterface->setFunctionInterface(component);

			return true;
		}

		return false;
	}

	void dispose()
	{
		m_propertyInterface->dispose();
		m_component.reset();
	}

protected:
	MikanComponentWeakPtr m_component;
	RmlModel_PropertyInterfacePtr m_propertyInterface;
	RmlDataBinding_ScriptTriggerListPtr m_scriptTriggerList;
};
