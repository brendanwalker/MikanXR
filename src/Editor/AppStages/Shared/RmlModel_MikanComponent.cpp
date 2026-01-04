#include "RmlModel_MikanComponent.h"
#include "ComponentScriptContext.h"
#include "Shared/RmlDataBinding_List.h"
#include "RmlManager.h"

#include <RmlUi/Core/DataModelHandle.h>

RmlModel_MikanComponent::RmlModel_MikanComponent()
	: m_component()
	, m_entityAccessor(std::make_shared<RmlModel_EntityAccessor>())
	, m_scriptTriggerList(std::make_shared<RmlDataBinding_ScriptTriggerList>())
{
}

bool RmlModel_MikanComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	RmlManager::getInstance()->bindEnumDefinitionsToDataModel(constructor);

	// Listen for property changes
	m_entityAccessor->OnEntityPropertyChanged += MakeDelegate(
			this,
			&RmlModel_MikanComponent::onComponentPropertyChanged);

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

MikanComponentPtr RmlModel_MikanComponent::getComponent() const
{
	return m_component.lock();
}

bool RmlModel_MikanComponent::setComponent(MikanComponentPtr component)
{
	MikanComponentPtr oldComponent = m_component.lock();

	if (component != oldComponent || m_component.expired())
	{
		if (component)
		{
			m_entityAccessor->setEntityAccessor(component);
		}
		else
		{
			m_entityAccessor->setEntityAccessor(nullptr);
		}

		m_component = component;

		return true;
	}

	return false;
}

// IRmlModel
Rml::Context* RmlModel_MikanComponent::getContext()
{
	return m_entityAccessor->getContext();
}

Rml::DataModelHandle& RmlModel_MikanComponent::getModelHandle()
{
	return m_entityAccessor->getModelHandle();
}

void RmlModel_MikanComponent::dispose()
{
	m_entityAccessor->OnEntityPropertyChanged -= MakeDelegate(
		this,
		&RmlModel_MikanComponent::onComponentPropertyChanged);
	m_entityAccessor->dispose();

	m_component.reset();
}

void RmlModel_MikanComponent::update(float deltaSeconds)
{
	m_entityAccessor->update(deltaSeconds);
}

void RmlModel_MikanComponent::addModelUpdateCallback(std::function<void()> callback)
{
	m_entityAccessor->addModelUpdateCallback(callback);
}