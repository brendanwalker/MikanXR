#pragma once

#include "CommonConfig.h"
#include "CommonConfigFwd.h"
#include "IEntityIDAllocator.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <configuru.hpp>

// Template class for managing a serializable pool of component definitions
// Used by MikanObjectSystemDefinition subclasses to manage component configs
template <class t_component_definition, typename t_id_type>
class MikanTypedComponentPoolDefinition : public CommonConfig
{
public:
	using ComponentDefinitionPtr= std::shared_ptr<t_component_definition>;
	using DefinitionConstPtr= std::shared_ptr<const t_component_definition>;
	using DefinitionList= std::vector<ComponentDefinitionPtr>;
	using DefinitionListConstIter= typename DefinitionList::const_iterator;

	MikanTypedComponentPoolDefinition(IEntityIDAllocatorPtr idAllocator)
		: CommonConfig("ComponentPool")
		, m_idAllocator(idAllocator)
	{
	}

	// Serialization
	virtual configuru::Config writeToJSON() override
	{
		configuru::Config pt= CommonConfig::writeToJSON();

		std::vector<configuru::Config> definitionConfigs;
		for (auto definitionPtr : m_definitions)
		{
			definitionConfigs.push_back(definitionPtr->writeToJSON());
		}
		pt.insert_or_assign("component_definitions", definitionConfigs);

		return pt;
	}

	virtual void readFromJSON(const configuru::Config& pt) override
	{
		CommonConfig::readFromJSON(pt);

		// Read in the definitions
		m_definitions.clear();
		if (pt.has_key("component_definitions"))
		{
			for (const configuru::Config& definitionConfig : pt["component_definitions"].as_array())
			{
				// Read on the the component definition
				auto definitionPtr= std::make_shared<t_component_definition>();
				definitionPtr->readFromJSON(definitionConfig);
				m_definitions.push_back(definitionPtr);
				addChildConfig(definitionPtr);

				// Make sure the next ID is greater than the loaded definition's ID
				// to avoid collisions with future allocations
				m_idAllocator.lock()->ensureNextIdGreaterThan(definitionPtr->getComponentId());
			}
		}
	}

	// Config accessors
	ComponentDefinitionPtr getById(int32_t id) const
	{
		auto it= findDefinitionIteratorById(id);

		if (it != m_definitions.end())
		{
			return *it;
		}

		return ComponentDefinitionPtr();
	}

	ComponentDefinitionPtr getByName(const std::string& name) const
	{
		auto it= std::find_if(
			m_definitions.begin(), m_definitions.end(),
			[this, &name](DefinitionConstPtr definitionPtr)
			{
				return definitionPtr->getConfigName() == name;
			});

		if (it != m_definitions.end())
		{
			return *it;
		}

		return ComponentDefinitionPtr();
	}

	using PredFunction= std::function<bool(DefinitionConstPtr)>;
	ComponentDefinitionPtr findByPredicate(PredFunction pred) const
	{
		auto it= std::find_if(
			m_definitions.begin(), m_definitions.end(),
			[this, &pred](DefinitionConstPtr definitionPtr)
			{
				return pred(definitionPtr);
			});
		if (it != m_definitions.end())
		{
			return *it;
		}
		return ComponentDefinitionPtr();
	}

	const DefinitionList& getAll() const { return m_definitions; }

	// Config mutations
	ComponentDefinitionPtr allocateDefinition()
	{
		t_id_type nextId= m_idAllocator.lock()->allocateNextId();

		return std::make_shared<t_component_definition>(nextId);
	}

	bool addDefinition(ComponentDefinitionPtr definition)
	{
		t_id_type id= definition->getComponentId();

		auto it= findDefinitionIteratorById(id);

		if (it == m_definitions.end())
		{
			m_definitions.push_back(definition);
			addChildConfig(definition);

			return true;
		}

		return false;
	}

	bool removeDefinition(t_id_type id)
	{
		auto it= findDefinitionIteratorById(id);

		if (it != m_definitions.end())
		{
			removeChildConfig(*it);
			m_definitions.erase(it);

			return true;
		}

		return false;
	}

protected:
	DefinitionListConstIter findDefinitionIteratorById(t_id_type id) const
	{
		return std::find_if(
			m_definitions.begin(), m_definitions.end(),
			[this, id](DefinitionConstPtr definitionPtr)
			{
				return definitionPtr->getComponentId() == id;
			});
	}

private:
	IEntityIDAllocatorWeakPtr m_idAllocator;
	DefinitionList m_definitions;
};
