#pragma once

#include "MikanObjectSystem.h"
#include "MikanTypedObjectPool.h"
#include "MikanTypedObjectSystemDefinition.h"
#include "MikanPropertyDatabase.h"

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
	using Pool = MikanTypedObjectPool<TComponent, TDefinition, TID>;
	using DefinitionInitFunction = std::function<void(ComponentDefinitionPtr)>;

	MikanTypedObjectSystem(
		ProjectManagerPtr ownerObjectSystem)
		: MikanObjectSystem(ownerObjectSystem)
		, m_pool(
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
		m_pool.initializeFromDefinitions(systemDefinition->getAllDefinitions());

		return true;
	}

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override
	{
		propertyDatabase->template registerPropertiesForSystem<TSystem>();
		propertyDatabase->template registerPropertiesForComponent<TSystem, TComponent>();
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
		return m_pool.getById(static_cast<TID>(componentId));
	}

	ComponentPtr getTypedComponentById(TID id) const
	{
		return m_pool.getById(id);
	}

	ComponentPtr getTypedComponentByName(const std::string& name) const
	{
		return m_pool.getByName(name);
	}

	const typename Pool::ComponentMap& getComponentMap() const
	{
		return m_pool.getAll();
	}

	// Component Pool Mutators
	ComponentPtr addNewObject(DefinitionInitFunction definitionInit = {})
	{
		SystemDefinitionPtr systemDefinition = getTypedDefinition();

		// Allocate new scene definition (doesn't add to pool yet)
		ComponentDefinitionPtr componentDefinition = systemDefinition->allocateNewDefinition();
		componentDefinition->setComponentName(
			TComponent::k_componentClassName
			+ std::to_string(componentDefinition->getComponentId()));

		// Allow caller to initialize definition before creating object
		definitionInit(componentDefinition);

		// Create the scene object using the pool (will add definition to pool after object is built)
		return m_pool.create(componentDefinition);
	}

	bool removeObject(TID componentId)
	{
		SystemDefinitionPtr systemDefinition = getTypedDefinition();

		return
			m_pool.dispose(componentId) &&
			systemDefinition->removeDefinition(componentId);
	}

	// -- IPropertyInterface ----
	static const std::string k_componentIdListPropertyId;
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
	{
		MikanObjectSystem::getPropertyDescriptors(outDescriptors);

		outDescriptors.push_back(
			std::make_shared<PropertyDescriptor>(k_componentIdListPropertyId, MikanVariantType::INT_ARRAY)
			->setReadOnly());
	}
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override
	{
		if (propertyDesc->getName() == k_componentIdListPropertyId)
		{
			std::vector<int> componentIdList;
			m_pool.getAllComponentIds(componentIdList);
			outValue = componentIdList;
			return true;
		}

		return MikanObjectSystem::getPropertyValue(propertyDesc, outValue);
	}

protected:
	ComponentPtr createObjectFromDefinition(ComponentDefinitionPtr componentDefinition)
	{
		// Create the scene object using the pool (will add definition to pool after object is built)
		return m_pool.create(componentDefinition);
	}

	virtual void additionalComponentFactory(ComponentDefinitionPtr componentDefinition)
	{
		// override in derived classes to add additional components to the object
	}

private:
	ComponentPtr objectFactory(ComponentDefinitionPtr componentDefinition)
	{
		SystemDefinitionPtr systemDefinition = getTypedDefinition();

		MikanObjectPtr mikanObject = newObject();
		mikanObject->setName(componentDefinition->getComponentName());

		// Add the primary component to the object
		ComponentPtr componentPtr = mikanObject->template addComponent<TComponent>();
		mikanObject->setRootComponent(componentPtr);
		componentPtr->setDefinition(componentDefinition);

		// Allow derived systems to add additional components
		additionalComponentFactory(componentDefinition);

		// Init the object once all components are added
		mikanObject->init();

		// Add definition to pool (fires property change event now that object is fully built)
		systemDefinition->addDefinition(componentDefinition);

		return componentPtr;
	}

private:
	Pool m_pool;
};

// Static member definitions
template<class TComponent, class TDefinition, typename TID, class TSystem, class TSystemDefinition>
const std::string MikanTypedObjectSystem<
	TComponent,
	TDefinition,
	TID,
	TSystem,
	TSystemDefinition>
::k_componentIdListPropertyId = TComponent::k_componentClassName + "IdList";