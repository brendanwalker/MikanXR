#pragma once

#include "MikanObjectSystem.h"
#include "MikanTypedObjectPoolDefinition.h"

#include <memory>
#include <string>
#include <vector>

#include <configuru.hpp>

// Template base class for object system definitions that manage a single pool of components
// TDefinition: The component definition type (e.g., SceneComponentDefinition)
// TID: The ID type used to identify components (e.g., MikanSceneID)
template<typename TDefinition, typename TID>
class MikanTypedObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	using PoolDefinition = MikanTypedObjectPoolDefinition<TDefinition, TID>;
	using PoolDefinitionPtr = std::shared_ptr<PoolDefinition>;
	using ComponentDefinitionPtr = std::shared_ptr<TDefinition>;
	using DefinitionConstPtr = std::shared_ptr<const TDefinition>;

	MikanTypedObjectSystemDefinition(
		const std::string& configName,
		const std::string& poolPropertyId)
		: MikanObjectSystemDefinition(configName)
		, m_poolPropertyId(poolPropertyId)
	{
		m_poolDefinition = std::make_shared<PoolDefinition>(poolPropertyId);
		addChildConfig(m_poolDefinition);
	}

	virtual configuru::Config writeToJSON() override
	{
		configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();
		pt[m_poolPropertyId] = m_poolDefinition->writeToJSON();
		return pt;
	}

	virtual void readFromJSON(const configuru::Config& pt) override
	{
		MikanObjectSystemDefinition::readFromJSON(pt);

		if (pt.has_key(m_poolPropertyId))
		{
			m_poolDefinition->readFromJSON(pt[m_poolPropertyId]);
		}
	}

	// Pool accessors
	PoolDefinitionPtr getPoolDefinition() const { return m_poolDefinition; }

	ComponentDefinitionPtr getDefinitionById(TID id) const
	{
		return m_poolDefinition->getById(id);
	}

	ComponentDefinitionPtr getDefinitionByName(const std::string& name) const
	{
		return m_poolDefinition->getByName(name);
	}

	const std::vector<ComponentDefinitionPtr>& getAllDefinitions() const
	{
		return m_poolDefinition->getAll();
	}

	// Pool mutations
	ComponentDefinitionPtr allocateNewDefinition()
	{
		return m_poolDefinition->allocateDefinition();
	}

	bool addDefinition(ComponentDefinitionPtr definition)
	{
		m_poolDefinition->addDefinition(definition);
		return true;
	}

	bool removeDefinition(TID id)
	{
		return m_poolDefinition->removeDefinition(id);
	}

protected:
	PoolDefinitionPtr m_poolDefinition;
	std::string m_poolPropertyId;
};
