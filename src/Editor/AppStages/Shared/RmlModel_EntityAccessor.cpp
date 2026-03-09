#include "RmlModel_EntityAccessor.h"
#include "RmlUtility.h"
#include "RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include <assert.h>

RmlModel_EntityAccessor::~RmlModel_EntityAccessor()
{
	// This should have already been cleaned up in dispose()
	assert(m_entityAccessor.lock() == nullptr);
}

// Template helper function to bind vector component accessors
template<int ComponentCount>
void bindVectorComponents(
	RmlModel_EntityAccessor* ownerRmlModel,
	Rml::DataModelConstructor& constructor,
	const std::string& propertyName,
	PropertyDescriptorConstPtr propertyDescriptor)
{
	static const char* componentNames[] = { "x", "y", "z", "w" };
	static_assert(ComponentCount >= 2 && ComponentCount <= 4, "Component count must be 2, 3, or 4");

	for (size_t componentIndex = 0; componentIndex < ComponentCount; ++componentIndex)
	{
		const std::string componentName = propertyName + "_" + componentNames[componentIndex];

		if (propertyDescriptor->isReadOnly())
		{
			const std::string propName = propertyDescriptor->getName();
			constructor.BindFunc(
				componentName,
				[ownerRmlModel, propName, componentIndex](Rml::Variant& outVariant) {
					IPropertyInterfacePtr propInterface = ownerRmlModel->getEntityAccessor();
					if (propInterface)
					{
						MikanVariant vectorVariant;
						if (propInterface->getPropertyValue(propName, vectorVariant))
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

			const std::string propName = propertyDescriptor->getName();
			constructor.BindFunc(
				componentName,
				[ownerRmlModel, propName, componentIndex](Rml::Variant& outVariant) {
					IPropertyInterfacePtr propInterface = ownerRmlModel->getEntityAccessor();
					if (propInterface)
					{
						MikanVariant vectorVariant;
						if (propInterface->getPropertyValue(propName, vectorVariant))
						{
							outVariant = vectorVariant.getVectorComponentValue(componentIndex);
						}
					}
				},
				[ownerRmlModel, propName, componentIndex](const Rml::Variant& variant) {
					IPropertyInterfacePtr propInterface = ownerRmlModel->getEntityAccessor();
					if (propInterface)
					{
						MikanVariant vectorVariant;
						if (propInterface->getPropertyValue(propName, vectorVariant))
						{
							const float newComponentValue= variant.Get<float>();

							vectorVariant.setVectorComponentValue(componentIndex, newComponentValue);
							propInterface->setPropertyValue(propName, vectorVariant);
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
		const MikanVariantType variantType = propertyDescriptor->getDataType();

		// Keep track of property descriptors for change notifications
		m_propertyDescriptors.insert({ propertyName, propertyDescriptor });

		// Vector types need special handling to bind individual components
		if (variantType == MikanVariantType::VECTOR2F)
		{
			bindVectorComponents<2>(this, constructor, propertyName, propertyDescriptor);
		}
		else if (variantType == MikanVariantType::VECTOR3F)
		{
			bindVectorComponents<3>(this, constructor, propertyName, propertyDescriptor);
		}
		else if (variantType == MikanVariantType::VECTOR4F)
		{
			bindVectorComponents<4>(this, constructor, propertyName, propertyDescriptor);
		}
		// Array types need special handling
		else if (variantType == MikanVariantType::INT_ARRAY)
		{
			assert(propertyDescriptor->isReadOnly());

			// Create and register int list binding
			const std::string propName = propertyDescriptor->getName();
			RmlDataBinding_IntListPtr intListBinding = std::make_shared<RmlDataBinding_IntList>();
			intListBinding->init(
				constructor,
				CommonConfigPtr(),
				propName,
				[this, propName](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
					IPropertyInterfacePtr propInterface = getEntityAccessor();
					if (propInterface)
					{
						MikanVariant listPropertyValue;
						if (propInterface->getPropertyValue(propName, listPropertyValue))
						{
							outComponentIdList = listPropertyValue.getIntArrayValue();
						}
					}
				});

			m_intListBindings.push_back(intListBinding);
		}
		// All other types can be bound directly
		else
		{
			if (propertyDescriptor->isReadOnly())
			{
				constructor.BindFunc(
					propertyName,
					[this, propertyName, propertyDescriptor](Rml::Variant& outRmlVariant) {
						IPropertyInterfacePtr propertyInterface = m_entityAccessor.lock();

						MikanVariant mikanVariant;
						if (propertyInterface)
						{
							propertyInterface->getPropertyValue(propertyName, mikanVariant);
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
				const MikanVariantType dataType = propertyDescriptor->getDataType();
				const MikanVariant defaultValue = propertyDescriptor->getDefaultValue();
				constructor.BindFunc(
					propertyName,
					[this, propertyName, defaultValue](Rml::Variant& outRmlVariant) {
						IPropertyInterfacePtr propertyInterface = m_entityAccessor.lock();

						MikanVariant mikanVariant;
						if (propertyInterface)
						{
							propertyInterface->getPropertyValue(propertyName, mikanVariant);
						}
						else
						{
							mikanVariant = defaultValue;
						}

						Rml::Utilities::MikanVariantToRmlVariant(mikanVariant, outRmlVariant);
					},
					[this, propertyName, dataType](const Rml::Variant& inRmlVariant) {
						IPropertyInterfacePtr propertyInterface = m_entityAccessor.lock();
						if (propertyInterface)
						{
							MikanVariant mikanVariant=
								Rml::Utilities::RmlVariantToMikanVariant(
									inRmlVariant, dataType);
							propertyInterface->setPropertyValue(propertyName, mikanVariant);
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
					functionInterface->invokeFunction(functionDescriptor->getFunctionName());
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

void RmlModel_EntityAccessor::dispose()
{
	// Clear property interface to remove any bound callbacks on m_propertyChangeEventSource
	clearEntityAccessor();
	RmlModel::dispose();
}

void RmlModel_EntityAccessor::clearEntityAccessor()
{
	IEntityAccessorPtr oldEntityAccessor = m_entityAccessor.lock();

	if (oldEntityAccessor)
	{
		CommonConfigPtr oldEntityConfig = oldEntityAccessor->getEntityConfig();
		assert(m_bWasAccessorSet);

		oldEntityAccessor->onDisposed -=
			MakeDelegate(this, &RmlModel_EntityAccessor::onEntityDisposed);
		oldEntityConfig->OnPropertyChanged -=
			MakeDelegate(this, &RmlModel_EntityAccessor::onEntityConfigChanged);

		// Tell the int list bindings to clear their owner config
		for (RmlDataBinding_IntListPtr& intListBinding : m_intListBindings)
		{
			intListBinding->setOwnerConfig(CommonConfigPtr());
		}

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
		CommonConfigPtr newEntityConfig;

		clearEntityAccessor();

		if (newEntityAccessor)
		{
			newEntityConfig = newEntityAccessor->getEntityConfig();

			newEntityAccessor->onDisposed +=
				MakeDelegate(this, &RmlModel_EntityAccessor::onEntityDisposed);
			newEntityConfig->OnPropertyChanged +=
				MakeDelegate(this, &RmlModel_EntityAccessor::onEntityConfigChanged);

			// For debugging purposes we track whether an accessor was ever set to a valid accessor
			// This helps catch cases where the accessor was destroyed without notification
			m_bWasAccessorSet = true;
		}

		m_entityAccessor = newEntityAccessor;

		// Tell the int list bindings to listen to the new entity config changed
		for (RmlDataBinding_IntListPtr& intListBinding : m_intListBindings)
		{
			intListBinding->setOwnerConfig(newEntityConfig);
		}

		m_modelHandle.DirtyAllVariables();
	}
}

void RmlModel_EntityAccessor::onEntityDisposed(const IEntityAccessor* selfPtr)
{
	clearEntityAccessor();
}

void RmlModel_EntityAccessor::onEntityConfigChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	IEntityAccessorPtr entityAccessor = m_entityAccessor.lock();
	assert(entityAccessor);

	// Dirty changed properties in the data model for this entity
	if (entityAccessor->getEntityConfig() == configPtr)
	{
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
				else if (variantType == MikanVariantType::INT_ARRAY)
				{
					// Int array properties are handled by RmlDataBinding_IntList
				}
				else
				{
					m_modelHandle.DirtyVariable(propertyName);
				}
			}
		}
	}

	// Forward the change notification (which could be from a child config)
	if (OnEntityPropertyChanged)
	{
		OnEntityPropertyChanged(entityAccessor, changedPropertySet);
	}
}
