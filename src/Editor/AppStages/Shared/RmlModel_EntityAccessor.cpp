#include "RmlModel_EntityAccessor.h"
#include "RmlUtility.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_EntityAccessor::~RmlModel_EntityAccessor()
{
	// Clear property interface to remove any bound callbacks on m_propertyChangeEventSource
	clearEntityAccessor();
}

// Template helper function to bind vector component accessors
template<int ComponentCount>
void bindVectorComponents(
	Rml::DataModelConstructor& constructor,
	const std::string& propertyName,
	PropertyDescriptorConstPtr propertyDescriptor,
	IPropertyInterfaceWeakPtr propertyInterface)
{
	static const char* componentNames[] = { "x", "y", "z", "w" };
	static_assert(ComponentCount >= 2 && ComponentCount <= 4, "Component count must be 2, 3, or 4");

	for (size_t componentIndex = 0; componentIndex < ComponentCount; ++componentIndex)
	{
		const std::string componentName = propertyName + "_" + componentNames[componentIndex];

		if (propertyDescriptor->isReadOnly())
		{
			constructor.BindFunc(
				componentName,
				[propertyInterface, propertyDescriptor, componentIndex](Rml::Variant& outVariant) {
					IPropertyInterfacePtr propInterface = propertyInterface.lock();
					if (propInterface)
					{
						MikanVariant vectorVariant;
						if (propInterface->getPropertyValue(propertyDescriptor, vectorVariant))
						{
							const float componentValue = vectorVariant.getVectorComponentValue(componentIndex);
							outVariant = componentValue;
						}
					}
				});
		}
		else
		{
			assert(propertyDescriptor->isReadable() && propertyDescriptor->isWritable());

			constructor.BindFunc(
				componentName,
				[propertyInterface, propertyDescriptor, componentIndex](Rml::Variant& outVariant) {
					IPropertyInterfacePtr propInterface = propertyInterface.lock();
					if (propInterface)
					{
						MikanVariant vectorVariant;
						if (propInterface->getPropertyValue(propertyDescriptor, vectorVariant))
						{
							outVariant = vectorVariant.getVectorComponentValue(componentIndex);
						}
					}
				},
				[propertyInterface, propertyDescriptor, componentIndex](const Rml::Variant& variant) {
					IPropertyInterfacePtr propInterface = propertyInterface.lock();
					if (propInterface)
					{
						MikanVariant vectorVariant;
						if (propInterface->getPropertyValue(propertyDescriptor, vectorVariant))
						{							
							const float newComponentValue= variant.Get<float>();

							vectorVariant.setVectorComponentValue(componentIndex, newComponentValue);
							propInterface->setPropertyValue(propertyDescriptor, vectorVariant);
						}
					}
				});
		}
	}
}

bool RmlModel_EntityAccessor::init(
	Rml::Context* rmlContext,
	const std::string& modelName,
	const std::vector<PropertyDescriptorConstPtr>& propertyDescriptors,
	const std::vector<FunctionDescriptorConstPtr>& functionDescriptors,
	OnConstruct onContructCallback)
{
	clearEntityAccessor();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, modelName);
	if (!constructor)
		return false;

	// Bind Properties from Property Interface
	for (const PropertyDescriptorConstPtr& propertyDescriptor : propertyDescriptors)
	{
		const std::string& propertyName = propertyDescriptor->getName();
		const MikanVariant& defaultValue = propertyDescriptor->getDefaultValue();
		const MikanVariantType variantType = defaultValue.value_type;

		// Keep track of property descriptors for change notifications
		m_propertyDescriptors.insert({ propertyName, propertyDescriptor });

		// Vector types need special handling to bind individual components
		if (variantType == MikanVariantType::VECTOR2F)
		{
			bindVectorComponents<2>(constructor, propertyName, propertyDescriptor, m_entityAccessor);
		}
		else if (variantType == MikanVariantType::VECTOR3F)
		{
			bindVectorComponents<3>(constructor, propertyName, propertyDescriptor, m_entityAccessor);
		}
		else if (variantType == MikanVariantType::VECTOR4F)
		{
			bindVectorComponents<4>(constructor, propertyName, propertyDescriptor, m_entityAccessor);
		}
		// All other types can be bound directly
		else
		{
			if (propertyDescriptor->isReadOnly())
			{
				constructor.BindFunc(
					propertyName,
					[this, propertyDescriptor](Rml::Variant& outRmlVariant) {
						IPropertyInterfacePtr propertyInterface = m_entityAccessor.lock();

						MikanVariant mikanVariant;
						if (propertyInterface)
						{
							propertyInterface->getPropertyValue(propertyDescriptor, mikanVariant);
						}
						else
						{
							mikanVariant = propertyDescriptor->getDefaultValue();
						}

						Rml::Utilities::MikanVariantToRmlVariant(mikanVariant, outRmlVariant);
					});
			}
			else if (propertyDescriptor->isReadable() && propertyDescriptor->isWritable())
			{
				constructor.BindFunc(
					propertyName,
					[this, propertyDescriptor](Rml::Variant& outRmlVariant) {
						IPropertyInterfacePtr propertyInterface = m_entityAccessor.lock();

						MikanVariant mikanVariant;
						if (propertyInterface)
						{
							propertyInterface->getPropertyValue(propertyDescriptor, mikanVariant);
						}
						else
						{
							mikanVariant = propertyDescriptor->getDefaultValue();
						}

						Rml::Utilities::MikanVariantToRmlVariant(mikanVariant, outRmlVariant);
					},
					[this, propertyDescriptor](const Rml::Variant& inRmlVariant) {
						IPropertyInterfacePtr propertyInterface = m_entityAccessor.lock();
						if (propertyInterface)
						{
							MikanVariant mikanVariant= 
								Rml::Utilities::RmlVariantToMikanVariant(
									inRmlVariant, propertyDescriptor->getDataType());
							propertyInterface->setPropertyValue(propertyDescriptor, mikanVariant);
						}
					});
			}
		}
	}

	// Binding Functions from Function Interface
	for (const FunctionDescriptorConstPtr& functionDescriptor : functionDescriptors)
	{
		const std::string& functionName = functionDescriptor->getFunctionName();

		constructor.BindEventCallback(
			functionName,
			[this, functionDescriptor](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
				IFunctionInterfacePtr functionInterface = m_entityAccessor.lock();
				if (functionInterface)
				{
					functionInterface->invokeFunction(functionDescriptor);
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

void RmlModel_EntityAccessor::clearEntityAccessor()
{
	IEntityAccessorPtr oldEntityAccessor = m_entityAccessor.lock();

	if (oldEntityAccessor)
	{
		CommonConfigPtr oldEntityConfig = oldEntityAccessor->getEntityConfig();
		assert(m_bWasAccessorSet);

		oldEntityConfig->OnDestroyed -=
			MakeDelegate(this, &RmlModel_EntityAccessor::onEntityConfigDestroyed);
		oldEntityConfig->OnPropertyChanged -=
			MakeDelegate(this, &RmlModel_EntityAccessor::onEntityConfigChanged);

		m_entityAccessor.reset();
		m_bWasAccessorSet = false;
	}
	else
	{
		// If this fires the entity accessor was destroyed without us being notified
		assert(!m_bWasAccessorSet);
	}
}

void RmlModel_EntityAccessor::setEntityAccessor(
	IEntityAccessorPtr newEntityAccessor)
{
	IEntityAccessorPtr oldEntityAccessor = m_entityAccessor.lock();

	if (newEntityAccessor != oldEntityAccessor)
	{
		clearEntityAccessor();

		if (newEntityAccessor)
		{
			CommonConfigPtr newEntityConfig = newEntityAccessor->getEntityConfig();

			newEntityConfig->OnDestroyed +=
				MakeDelegate(this, &RmlModel_EntityAccessor::onEntityConfigDestroyed);
			newEntityConfig->OnPropertyChanged +=
				MakeDelegate(this, &RmlModel_EntityAccessor::onEntityConfigChanged);

			// For debugging purposes we track whether an accessor was ever set to a valid accessor
			// This helps catch cases where the accessor was destroyed without notification
			m_bWasAccessorSet = true;
		}

		m_entityAccessor = newEntityAccessor;

		m_modelHandle.DirtyAllVariables();
	}
}

void RmlModel_EntityAccessor::onEntityConfigDestroyed(const CommonConfig* selfPtr)
{
	clearEntityAccessor();
}

void RmlModel_EntityAccessor::onEntityConfigChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	IEntityAccessorPtr entityAccessor = m_entityAccessor.lock();
	assert(entityAccessor);
	assert(entityAccessor->getEntityConfig() == configPtr);
	if (OnEntityPropertyChanged)
	{
		OnEntityPropertyChanged(entityAccessor, changedPropertySet);
	}

	for (const std::string& propertyName : changedPropertySet.getSet())
	{
		auto it = m_propertyDescriptors.find(propertyName);
		if (it != m_propertyDescriptors.end())
		{
			// Also dirty component variables for vector types
			const PropertyDescriptorConstPtr& propertyDescriptor = it->second;
			const MikanVariant& defaultValue = propertyDescriptor->getDefaultValue();
			const MikanVariantType variantType = propertyDescriptor->getDataType();

			if (variantType == MikanVariantType::VECTOR2F)
			{
				m_modelHandle.DirtyVariable(propertyName + "_x");
				m_modelHandle.DirtyVariable(propertyName + "_y");
			}
			else if (variantType == MikanVariantType::VECTOR3F)
			{
				m_modelHandle.DirtyVariable(propertyName + "_x");
				m_modelHandle.DirtyVariable(propertyName + "_y");
				m_modelHandle.DirtyVariable(propertyName + "_z");
			}
			else if (variantType == MikanVariantType::VECTOR4F)
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
