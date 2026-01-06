#pragma once

#include "MikanTypedObjectPoolDefinition.h"
#include "MikanObjectSystem.h"
#include "MikanObject.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

// Template class for managing a runtime pool of components
// Used by MikanObjectSystem subclasses to manage component instances
template<class t_component_class, class t_definition_class, typename t_id_type>
class MikanTypedObjectPool
{
public:
	using ComponentPtr = std::shared_ptr<t_component_class>;
	using ComponentWeakPtr = std::weak_ptr<t_component_class>;
	using ComponentMap = std::map<t_id_type, ComponentWeakPtr>;
	using DefinitionPtr = std::shared_ptr<t_definition_class>;
	using DefinitionConstPtr = std::shared_ptr<const t_definition_class>;
	using PoolDefinitionPtr = std::shared_ptr<MikanTypedObjectPoolDefinition<t_definition_class, t_id_type>>;
	using FactoryFunction = std::function<ComponentPtr(DefinitionPtr)>;

	MikanTypedObjectPool(
		MikanObjectSystem* ownerSystem,
		FactoryFunction factory)
		: m_ownerSystem(ownerSystem)
		, m_factory(factory)
	{
	}

	// Lookup operations
	ComponentPtr getById(t_id_type id) const
	{
		auto iter = m_components.find(id);
		if (iter != m_components.end())
		{
			return iter->second.lock();
		}

		return ComponentPtr();
	}

	ComponentPtr getByName(const std::string& name) const
	{
		for (auto it = m_components.begin(); it != m_components.end(); it++)
		{
			ComponentPtr componentPtr = it->second.lock();

			if (componentPtr && componentPtr->getName() == name)
			{
				return componentPtr;
			}
		}

		return ComponentPtr();
	}

	const ComponentMap& getAll() const { return m_components; }

	// Lifecycle operations
	ComponentPtr create(DefinitionPtr definition)
	{
		ComponentPtr component = m_factory(definition);
		if (component)
		{
			const t_id_type id = definition->getComponentId();

			m_components.insert({ id, component});
		}

		return component;
	}

	bool dispose(t_id_type id)
	{
		auto it = m_components.find(id);
		if (it != m_components.end())
		{
			// Default disposal: get component and delete its owner object
			ComponentPtr component = it->second.lock();
			if (component)
			{
				m_ownerSystem->deleteObject(component->getOwnerObject());
			}

			// Remove from component map
			m_components.erase(it);

			return true;
		}

		return false;
	}

	void disposeAll()
	{
		// Copy the map keys to avoid iterator invalidation
		std::vector<t_id_type> ids;
		for (const auto& pair : m_components)
		{
			ids.push_back(pair.first);
		}

		// Dispose all components
		for (t_id_type id : ids)
		{
			dispose(id);
		}

		m_components.clear();
	}

	// Initialization from definition pool
	void initializeFromDefinitions(PoolDefinitionPtr poolDefinition)
	{
		const auto& definitions = poolDefinition->getAll();
		for (auto definitionPtr : definitions)
		{
			t_id_type id = definitionPtr->getComponentId();
			ComponentPtr component = m_factory(definitionPtr);
			if (component)
			{
				m_components.insert({id, component});
			}
		}
	}

protected:
	MikanObjectSystem* m_ownerSystem;
	ComponentMap m_components;
	FactoryFunction m_factory;
};
