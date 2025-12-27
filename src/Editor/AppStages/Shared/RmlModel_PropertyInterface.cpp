#include "RmlModel_PropertyInterface.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

// Template helper function to bind vector component accessors
template<typename VectorType, int ComponentCount>
void bindVectorComponents(
	Rml::DataModelConstructor& constructor,
	const std::string& propertyName,
	RmlPropertyDescriptorConstPtr propertyDescriptor,
	IRmlPropertyInterfaceWeakPtr propertyInterface)
{
	static const char* componentNames[] = { "x", "y", "z", "w" };
	static_assert(ComponentCount >= 2 && ComponentCount <= 4, "Component count must be 2, 3, or 4");

	for (int i = 0; i < ComponentCount; ++i)
	{
		const std::string componentName = propertyName + "_" + componentNames[i];
		const int componentIndex = i;

		if (propertyDescriptor->isReadOnly())
		{
			constructor.BindFunc(
				componentName,
				[propertyInterface, propertyDescriptor, componentIndex](Rml::Variant& variant) {
					IRmlPropertyInterfacePtr propInterface = propertyInterface.lock();
					if (propInterface)
					{
						Rml::Variant vectorVariant;
						if (propInterface->getPropertyValueFromRml(propertyDescriptor, vectorVariant))
						{
							variant = vectorVariant.Get<VectorType>()[componentIndex];
						}
					}
				});
		}
		else
		{
			constructor.BindFunc(
				componentName,
				[propertyInterface, propertyDescriptor, componentIndex](Rml::Variant& variant) {
					IRmlPropertyInterfacePtr propInterface = propertyInterface.lock();
					if (propInterface)
					{
						Rml::Variant vectorVariant;
						if (propInterface->getPropertyValueFromRml(propertyDescriptor, vectorVariant))
						{
							variant = vectorVariant.Get<VectorType>()[componentIndex];
						}
					}
				},
				[propertyInterface, propertyDescriptor, componentIndex](const Rml::Variant& variant) {
					IRmlPropertyInterfacePtr propInterface = propertyInterface.lock();
					if (propInterface)
					{
						Rml::Variant vectorVariant;
						if (propInterface->getPropertyValueFromRml(propertyDescriptor, vectorVariant))
						{
							VectorType vec = vectorVariant.Get<VectorType>();
							vec[componentIndex] = variant.Get<float>();
							propInterface->setPropertyValueFromRml(propertyDescriptor, Rml::Variant(vec));
						}
					}
				});
		}
	}
}

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
		const Rml::Variant& defaultValue = propertyDescriptor->getDefaultValue();
		const Rml::Variant::Type variantType = defaultValue.GetType();

		// Keep track of property descriptors for change notifications
		m_propertyDescriptors.insert({ propertyName, propertyDescriptor });

		// Vector types need special handling to bind individual components
		if (variantType == Rml::Variant::VECTOR2)
		{
			bindVectorComponents<Rml::Vector2f, 2>(constructor, propertyName, propertyDescriptor, m_propertyInterface);
		}
		else if (variantType == Rml::Variant::VECTOR3)
		{
			bindVectorComponents<Rml::Vector3f, 3>(constructor, propertyName, propertyDescriptor, m_propertyInterface);
		}
		else if (variantType == Rml::Variant::VECTOR4)
		{
			bindVectorComponents<Rml::Vector4f, 4>(constructor, propertyName, propertyDescriptor, m_propertyInterface);
		}
		// All other types can be bound directly
		else
		{
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
							variant = propertyDescriptor->getDefaultValue();
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
		}
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

	if (newPropertyInterface != oldPropertyInterface || m_propertyInterface.expired())
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
		auto it = m_propertyDescriptors.find(propertyName);
		if (it != m_propertyDescriptors.end())
		{
			// Also dirty component variables for vector types
			const RmlPropertyDescriptorConstPtr& propertyDescriptor = it->second;
			const Rml::Variant& defaultValue = propertyDescriptor->getDefaultValue();
			const Rml::Variant::Type variantType = defaultValue.GetType();

			if (variantType == Rml::Variant::VECTOR2)
			{
				m_modelHandle.DirtyVariable(propertyName + "_x");
				m_modelHandle.DirtyVariable(propertyName + "_y");
			}
			else if (variantType == Rml::Variant::VECTOR3)
			{
				m_modelHandle.DirtyVariable(propertyName + "_x");
				m_modelHandle.DirtyVariable(propertyName + "_y");
				m_modelHandle.DirtyVariable(propertyName + "_z");
			}
			else if (variantType == Rml::Variant::VECTOR4)
			{
				m_modelHandle.DirtyVariable(propertyName + "_x");
				m_modelHandle.DirtyVariable(propertyName + "_y");
				m_modelHandle.DirtyVariable(propertyName + "_z");
				m_modelHandle.DirtyVariable(propertyName + "_w");
			}
			else
			{
				m_modelHandle.DirtyVariable(propertyName);
			}
		}
	}
}
