#pragma once

#include "MikanObjectSystem.h"
#include "MikanTypedComponentPool.h"
#include "MikanTypedObjectSystemDefinition.h"
#include "MikanPropertyDatabase.h"
#include "MikanFunctionDatabase.h"
#include "TransformComponent.h"

#include <memory>
#include <string>

// Template base class for object systems that manage a single pool of components
// TComponent: The component type (e.g., SceneComponent)
// TDefinition: The component definition type (e.g., SceneComponentDefinition)
// TID: The ID type used to identify components (e.g., MikanSceneID)
// TSystem: The system type (e.g., SceneObjectSystem)
// TSystemDefinition: The system definition type (e.g., SceneObjectSystemDefinition)
template<class TComponent, class TDefinition, typename TID, class TSystem, class TSystemDefinition>
class MikanTypedObjectSystem : public MikanObjectSystem
{
public:
	using ComponentPtr = std::shared_ptr<TComponent>;
	using ComponentConstPtr = std::shared_ptr<const TComponent>;
	using ComponentDefinitionPtr = std::shared_ptr<TDefinition>;
	using SystemDefinitionPtr = std::shared_ptr<TSystemDefinition>;
	using SystemDefinitionConstPtr = std::shared_ptr<const TSystemDefinition>;
	using Pool = MikanTypedComponentPool<TComponent, TDefinition, TID>;
	using DefinitionInitFunction = std::function<bool(ComponentDefinitionPtr)>;

	MikanTypedObjectSystem(
		ProjectManagerPtr ownerObjectSystem)
		: MikanObjectSystem(ownerObjectSystem)
		, m_componentPool(
			this,
			[this](auto def) { return this->objectFactory(def); })
	{
	}

	// -- MikanObjectSystem ----
	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override
	{
		MikanObjectSystem::init(definitionPtr);

		// Create pool objects from definitions
		SystemDefinitionConstPtr systemDefinition = getTypedDefinitionConst();
		m_componentPool.initializeFromDefinitions(systemDefinition->getAllDefinitions());

		return true;
	}

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override
	{
		propertyDatabase->template registerPropertiesForSystem<TSystem>();
		propertyDatabase->template registerPropertiesForComponent<TSystem, TComponent>();
	}

	virtual void registerFunctionDescriptors(MikanFunctionDatabasePtr functionDatabase) override
	{
		functionDatabase->template registerFunctionsForSystem<TSystem>();
		functionDatabase->template registerFunctionsForComponent<TSystem, TComponent>();
	}

	virtual bool deleteObject(MikanObjectPtr objectPtr) override
	{
		auto primaryComponent= objectPtr->getComponentOfType<TComponent>();
		if (primaryComponent)
		{
			return removeObjectByPrimaryComponentId(primaryComponent->getComponentId());
		}

		return false;
	}

	// System Definition Accessors
	std::shared_ptr<const TSystemDefinition> getTypedDefinitionConst() const
	{
		return std::static_pointer_cast<const TSystemDefinition>(getDefinitionConst());
	}

	std::shared_ptr<TSystemDefinition> getTypedDefinition()
	{
		return std::static_pointer_cast<TSystemDefinition>(getDefinition());
	}

	// Component Pool Accessors
	virtual MikanComponentPtr getComponentById(int componentId) const override
	{
		return std::static_pointer_cast<MikanComponent>(m_componentPool.getById(static_cast<TID>(componentId)));
	}

	virtual bool getComponentIdList(const std::string& componentClassName, std::vector<int>& outComponentIdList) const
	{
		if (componentClassName == TComponent::k_componentClassName)
		{
			const typename Pool::ComponentMap& componentMap = m_componentPool.getAll();
			for (const auto& kvpair : componentMap)
			{
				outComponentIdList.push_back(static_cast<int>(kvpair.first));
			}
			return true;
		}
		return false;
	}

	ComponentPtr getTypedComponentById(TID id) const
	{
		return m_componentPool.getById(id);
	}

	ComponentPtr getTypedComponentByName(const std::string& name) const
	{
		return m_componentPool.getByName(name);
	}

	TID getFirstComponentId() const
	{
		return m_componentPool.getFirstComponentId();
	}

	using PredFunction = std::function<bool(ComponentConstPtr)>;
	ComponentPtr getTypedComponentByPredicate(PredFunction predicate) const
	{
		return m_componentPool.findByPredicate(predicate);
	}

	const typename Pool::ComponentMap& getComponentMap() const
	{
		return m_componentPool.getAll();
	}

	using VisitFunction = std::function<void(ComponentPtr)>;
	void visitComponents(VisitFunction visitFunc) const
	{
		for (const auto& kvpair : getComponentMap())
		{
			ComponentPtr componentPtr = kvpair.second.lock();

			if (componentPtr)
			{
				visitFunc(componentPtr);
			}
		}
	}

	// Component Pool Mutators
	virtual MikanComponentPtr addNewObjectByUntypedDefinition(
		const std::string& primaryComponentClass,
		Serialization::PolymorphicObjectPtr initParams) override
	{
		// Only support creating objects from definitions that match the primary component type of this system
		if (primaryComponentClass == TComponent::k_componentClassName)
		{
			MikanObjectSystem* ownerObjectSystem = this;

			return addNewObjectByTypedDefinition(
				[ownerObjectSystem, &initParams](ComponentDefinitionPtr definition) {
					// Initialize the definition from the polymorphic object init params
					return definition->readFromInitParams(ownerObjectSystem, initParams);
				});
		}
		return MikanComponentPtr();
	}

	ComponentPtr addNewObjectByTypedDefinition(DefinitionInitFunction definitionInit = {})
	{
		SystemDefinitionPtr systemDefinition = getTypedDefinition();

		// Allocate new component definition (doesn't add to pool yet)
		ComponentDefinitionPtr componentDefinition = systemDefinition->allocateNewDefinition();
		componentDefinition->setComponentName(
			TComponent::k_componentClassName
			+ std::to_string(componentDefinition->getComponentId()));

		// Allow caller to initialize definition before creating object
		if (definitionInit)
		{
			if (!definitionInit(componentDefinition))
			{
				// If definition initialization fails, deallocate the definition and return null
				systemDefinition->removeDefinition(componentDefinition->getComponentId());

				return ComponentPtr();
			}
		}

		// Create the component object using the pool (will add definition to pool after object is built)
		return m_componentPool.create(componentDefinition);
	}

	bool removeObjectByPrimaryComponentId(TID componentId)
	{
		ComponentPtr component = getTypedComponentById(componentId);
		if (component)
		{
			SystemDefinitionPtr systemDefinition = getTypedDefinition();
			assert(systemDefinition);
			MikanObjectPtr objectPtr = component->getOwnerObject();
			assert(objectPtr);

			return 
				// 1. Remove the object from the system first (fires object/component disposed events)
				MikanObjectSystem::deleteObject(objectPtr) &&
				// 2. Remove the component reference from the typed component pool
				m_componentPool.dispose(componentId) &&
				// 3. Remove the component definition from the system definition 
				// (fires system property change event, which is now safe to do now that the object is cleaned up)
				systemDefinition->removeDefinition(componentId);
		}

		return false;
	}

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
	{
		MikanObjectSystem::getPropertyDescriptors(outDescriptors);

		outDescriptors.push_back(
			std::make_shared<PropertyDescriptor>(TSystemDefinition::k_componentIdListPropertyId, MikanVariantType::INT_ARRAY)
			->setReadOnly());
	}
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override
	{
		if (propertyName == TSystemDefinition::k_componentIdListPropertyId)
		{
			std::vector<int> componentIdList;
			getTypedDefinitionConst()->getAllComponentIds(componentIdList);
			outValue = componentIdList;
			return true;
		}

		return MikanObjectSystem::getPropertyValue(propertyName, outValue);
	}

protected:
	ComponentPtr createObjectFromDefinition(ComponentDefinitionPtr componentDefinition)
	{
		// Create the scene object using the pool (will add definition to pool after object is built)
		return m_componentPool.create(componentDefinition);
	}

	virtual void additionalComponentFactory(
		MikanObjectPtr ownerComponentObject,
		ComponentDefinitionPtr componentDefinition)
	{
		// override in derived classes to add additional components to the object
	}

private:
	ComponentPtr objectFactory(ComponentDefinitionPtr componentDefinition)
	{
		SystemDefinitionPtr systemDefinition = getTypedDefinition();

		MikanObjectPtr mikanObject = newEmptyObject();
		mikanObject->setName(componentDefinition->getComponentName());

		// Add the primary component to the object
		ComponentPtr componentPtr = mikanObject->template addComponent<TComponent>();
		componentPtr->setDefinition(componentDefinition);

		// If this is a TransformComponent, set it as the root component
		auto rootComponent = std::dynamic_pointer_cast<TransformComponent>(componentPtr);
		if (rootComponent)
		{
			mikanObject->setRootComponent(rootComponent);
		}

		// Allow derived systems to add additional components
		additionalComponentFactory(mikanObject, componentDefinition);

		// Init the object once all components are added
		mikanObject->init();

		// Then run post-init after all the component are initialized
		mikanObject->postInit();

		// Add definition to pool (fires property change event now that object is fully built)
		systemDefinition->addDefinition(componentDefinition);

		return componentPtr;
	}

private:
	Pool m_componentPool;
};