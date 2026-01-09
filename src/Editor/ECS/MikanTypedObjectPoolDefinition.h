#pragma once

#include "CommonConfig.h"
#include "CommonConfigFwd.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <configuru.hpp>

// Template class for managing a serializable pool of component definitions
// Used by MikanObjectSystemDefinition subclasses to manage component configs
template<class t_component_definition, typename t_id_type>
class MikanTypedObjectPoolDefinition : public CommonConfig
{
public:
	using ComponentDefinitionPtr = std::shared_ptr<t_component_definition>;
	using DefinitionConstPtr = std::shared_ptr<const t_component_definition>;
	using DefinitionList = std::vector<ComponentDefinitionPtr>;
	using DefinitionListConstIter = typename DefinitionList::const_iterator;

	MikanTypedObjectPoolDefinition(
		const std::string& poolPropertyId,
		int32_t initialNextId = 0)
		: CommonConfig("ComponentPool")
		, m_nextId(initialNextId)
	{}

	// Serialization
	virtual configuru::Config writeToJSON() override
	{
		configuru::Config pt = CommonConfig::writeToJSON();

		pt["next_id"] = m_nextId;

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

		m_nextId = pt.get_or<int32_t>("next_id", m_nextId);

		// Read in the definitions
		m_definitions.clear();
		if (pt.has_key("component_definitions"))
		{
			for (const configuru::Config& definitionConfig : pt["component_definitions"].as_array())
			{
				auto definitionPtr = std::make_shared<t_component_definition>();
				definitionPtr->readFromJSON(definitionConfig);
				m_definitions.push_back(definitionPtr);
				addChildConfig(definitionPtr);
			}
		}
	}

	// Config accessors
	const std::string& getListPropertyId() const 
	{ 
		return m_poolPropertyId; 
	}

	ComponentDefinitionPtr getById(int32_t id) const
	{
		auto it = findDefinitionIteratorById(id);

		if (it != m_definitions.end())
		{
			return *it;
		}

		return ComponentDefinitionPtr();
	}

	ComponentDefinitionPtr getByName(const std::string& name) const
	{
		auto it = std::find_if(
			m_definitions.begin(), m_definitions.end(),
			[this, &name](DefinitionConstPtr definitionPtr) {
				return definitionPtr->getConfigName() == name;
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
		t_id_type nextId = m_nextId;
		m_nextId++;

		return std::make_shared<t_component_definition>(nextId);
	}

	bool addDefinition(ComponentDefinitionPtr definition)
	{
		t_id_type id = definition->getComponentId();

		auto it = findDefinitionIteratorById(id);

		if (it == m_definitions.end())
		{
			m_definitions.push_back(definition);
			addChildConfig(definition);

			// Update next ID if the added definition's ID is >= current next ID
			if (id >= m_nextId)
			{
				m_nextId = id + 1;
			}

			notifyDefinitionsChanged();

			return true;
		}

		return false;
	}

	bool removeDefinition(t_id_type id)
	{
		auto it = findDefinitionIteratorById(id);

		if (it != m_definitions.end())
		{
			removeChildConfig(*it);
			m_definitions.erase(it);

			notifyDefinitionsChanged();

			return true;
		}

		return false;
	}

	void notifyDefinitionsChanged()
	{
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(m_poolPropertyId));
	}

protected:
	DefinitionListConstIter findDefinitionIteratorById(t_id_type id) const
	{
		return std::find_if(
			m_definitions.begin(), m_definitions.end(),
			[this, id](DefinitionConstPtr definitionPtr) {
				return definitionPtr->getComponentId() == id;
			});
	}

private:
	t_id_type m_nextId;
	DefinitionList m_definitions;
	std::string m_poolPropertyId;     // Property ID for change notifications
};
