#include "RmlModel_PropertyInterface.h"
#include "PropertyInterface.h"
#include "FunctionInterface.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>


bool RmlModel_PropertyInterface::init(
	Rml::Context* rmlContext,
	const std::string& modelName,
	const std::vector<std::string>& propertyNames,
	const std::vector<std::string>& functionNames)
{
	m_propertyInterface = nullptr;
	m_functionInterface = nullptr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, modelName);
	if (!constructor)
		return false;

	// Bind Properties from Property Interface
	for (const std::string& propertyName : propertyNames)
	{
		constructor.BindFunc(
			propertyName,
			[this, propertyName](Rml::Variant& variant) {
				if (m_propertyInterface)
				{
					m_propertyInterface->getPropertyValue(propertyName, variant);
				}
			},
			[this, propertyName](const Rml::Variant& variant) {
				if (m_propertyInterface)
				{
					m_propertyInterface->setPropertyValue(propertyName, variant);
				}
			});
	}

	// Binding Functions from Function Interface
	for (const std::string& functionName : functionNames)
	{
		constructor.BindEventCallback(
			functionName,
			[this, functionName](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
				if (m_functionInterface)
				{
					m_functionInterface->invokeFunction(functionName);
				}
			});
	}

	return true;
}

void RmlModel_PropertyInterface::setPropertyInterface(IPropertyInterface* propertyInterface)
{
	m_propertyInterface = propertyInterface;
}

void RmlModel_PropertyInterface::setFunctionInterface(IFunctionInterface* functionInterface)
{
	m_functionInterface = functionInterface;
}