#pragma once

#include "MikanObjectSystem.h"
#include "MikanTypedComponentPoolDefinition.h"

#include <memory>
#include <string>
#include <vector>

#include <configuru.hpp>

// Template base class for object system definitions that manage a single pool of components
// TComponent: The component type (e.g., SceneComponent)
// TDefinition: The component definition type (e.g., SceneComponentDefinition)
// TID: The ID type used to identify components (e.g., MikanSceneID)
template<class TComponent, typename TDefinition, typename TID>
class MikanTypedObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	using IDAllocatorPtr = std::shared_ptr<class IEntityIDAllocator>;
	using PoolDefinition = MikanTypedComponentPoolDefinition<TDefinition, TID>;
	using PoolDefinitionPtr = std::shared_ptr<PoolDefinition>;
	using ComponentDefinitionPtr = std::shared_ptr<TDefinition>;
	using DefinitionConstPtr = std::shared_ptr<const TDefinition>;

	static const std::string k_componentPoolPropertyId;
	static const std::string k_componentIdListPropertyId;

	MikanTypedObjectSystemDefinition(
		const std::string& configName, 
		IDAllocatorPtr idAllocator)
		: MikanObjectSystemDefinition(configName, idAllocator)
	{
		m_poolDefinition = std::make_shared<PoolDefinition>(idAllocator);
		addChildConfig(m_poolDefinition);
	}

	virtual configuru::Config writeToJSON() override
	{
		configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();
		pt[k_componentPoolPropertyId] = m_poolDefinition->writeToJSON();
		return pt;
	}

	virtual void readFromJSON(const configuru::Config& pt) override
	{
		MikanObjectSystemDefinition::readFromJSON(pt);

		if (pt.has_key(k_componentPoolPropertyId))
		{
			m_poolDefinition->readFromJSON(pt[k_componentPoolPropertyId]);
		}
	}

	// Pool accessors
	inline PoolDefinitionPtr getPoolDefinition() const { return m_poolDefinition; }

	inline bool hasDefinitions() const
	{
		return !m_poolDefinition->getAll().empty();
	}

	inline ComponentDefinitionPtr getDefinitionById(TID id) const
	{
		return m_poolDefinition->getById(id);
	}

	inline ComponentDefinitionPtr getDefinitionByName(const std::string& name) const
	{
		return m_poolDefinition->getByName(name);
	}

	inline const std::vector<ComponentDefinitionPtr>& getAllDefinitions() const
	{
		return m_poolDefinition->getAll();
	}

	void getAllComponentIds(std::vector<int>& outComponentIdList) const
	{
		for (ComponentDefinitionPtr it : m_poolDefinition->getAll())
		{			
			outComponentIdList.push_back(it->getComponentId());
		}
	}

	// Pool mutations
	inline ComponentDefinitionPtr allocateNewDefinition()
	{
		return m_poolDefinition->allocateDefinition();
	}

	bool addDefinition(ComponentDefinitionPtr definition)
	{
		if (m_poolDefinition->addDefinition(definition))
		{
			notifyDefinitionsChanged();
			return true;
		}

		return false;
	}

	bool removeDefinition(TID id)
	{
		if (m_poolDefinition->removeDefinition(id))
		{
			notifyDefinitionsChanged();
			return true;
		}

		return false;
	}

protected:
	void notifyDefinitionsChanged()
	{
		notifyPropertyChanged(
			ConfigPropertyChangeSet()
			.addPropertyName(k_componentPoolPropertyId)
			.addPropertyName(k_componentIdListPropertyId));
	}

private:
	PoolDefinitionPtr m_poolDefinition;
};

// Static member definitions
template<class TComponent, class TDefinition, typename TID>
const std::string MikanTypedObjectSystemDefinition<TComponent, TDefinition, TID>
	::k_componentPoolPropertyId = TComponent::k_componentClassName + "Pool";

template<class TComponent, class TDefinition, typename TID>
const std::string MikanTypedObjectSystemDefinition<TComponent,TDefinition,TID>
	::k_componentIdListPropertyId = TComponent::k_componentClassName + "IdList";