#include "RmlModel_PropertyInterface.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>


bool RmlModel_PropertyInterface::init(
	Rml::Context* rmlContext,
	const std::string& modelName,
	const std::vector<RmlPropertyDescriptorConstPtr>& propertyDescriptors,
	const std::vector<RmlFunctionDescriptorConstPtr>& functionDescriptors,
	OnConstruct onContructCallback)
{
	m_propertyChangeEventSource.reset();
	m_propertyInterface.reset();
	m_functionInterface.reset();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, modelName);
	if (!constructor)
		return false;

	// Bind Properties from Property Interface
	for (const RmlPropertyDescriptorConstPtr& propertyDescriptor : propertyDescriptors)
	{
		const std::string& propertyName = propertyDescriptor->getName();

		if (propertyDescriptor->isReadOnly())
		{
			constructor.BindFunc(
				propertyName,
				[this, propertyDescriptor](Rml::Variant& variant) {
					IRmlPropertyInterfacePtr propertyInterface = m_propertyInterface.lock();
					if (propertyInterface)
					{
						propertyInterface->getPropertyValueFromRml(propertyDescriptor, variant);
					}
					else
					{
						variant= propertyDescriptor->getDefaultValue();
					}
				});
		}
		else
		{
			constructor.BindFunc(
				propertyName,
				[this, propertyDescriptor](Rml::Variant& variant) {
					IRmlPropertyInterfacePtr propertyInterface = m_propertyInterface.lock();
					if (propertyInterface)
					{
						propertyInterface->getPropertyValueFromRml(propertyDescriptor, variant);
					}
					else
					{
						variant = propertyDescriptor->getDefaultValue();
					}
				},
				[this, propertyDescriptor](const Rml::Variant& variant) {
					IRmlPropertyInterfacePtr propertyInterface = m_propertyInterface.lock();
					if (propertyInterface)
					{
						propertyInterface->setPropertyValueFromRml(propertyDescriptor, variant);
					}
				});
		}

		m_propertyDescriptors.insert({ propertyName, propertyDescriptor });
	}

	// Binding Functions from Function Interface
	for (const RmlFunctionDescriptorConstPtr& functionDescriptor : functionDescriptors)
	{
		const std::string& functionName = functionDescriptor->getFunctionName();

		constructor.BindEventCallback(
			functionName,
			[this, functionDescriptor](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
				IRmlFunctionInterfacePtr functionInterface = m_functionInterface.lock();
				if (functionInterface)
				{
					functionInterface->invokeFunctionFromRml(functionDescriptor);
				}
			});
	}

	// Handle any custom construction steps
	if (onContructCallback && !onContructCallback(constructor))
	{
		return false;
	}

	return true;
}

void RmlModel_PropertyInterface::setPropertyInterface(
	IRmlPropertyInterfacePtr newPropertyInterface,
	CommonConfigPtr newPropertyChangeEventSource)
{
	IRmlPropertyInterfacePtr oldPropertyInterface = m_propertyInterface.lock();
	CommonConfigPtr oldPropertyChangeEventSource = m_propertyChangeEventSource.lock();

	if (newPropertyInterface != oldPropertyInterface)
	{
		if (oldPropertyChangeEventSource)
		{
			oldPropertyChangeEventSource->OnMarkedDirty -=
				MakeDelegate(this, &RmlModel_PropertyInterface::onPropertiesChanged);
		}

		if (newPropertyChangeEventSource)
		{
			newPropertyChangeEventSource->OnMarkedDirty +=
				MakeDelegate(this, &RmlModel_PropertyInterface::onPropertiesChanged);
		}

		m_propertyInterface = newPropertyInterface;
		m_modelHandle.DirtyAllVariables();
	}
}

void RmlModel_PropertyInterface::setFunctionInterface(IRmlFunctionInterfacePtr functionInterface)
{
	m_functionInterface = functionInterface;
}

void RmlModel_PropertyInterface::onPropertiesChanged(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	for (const std::string& propertyName : changedPropertySet.getSet())
	{
		if (m_propertyDescriptors.find(propertyName) != m_propertyDescriptors.end())
		{
			m_modelHandle.DirtyVariable(propertyName);
		}
	}
}
