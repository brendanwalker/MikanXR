#include "TransformComponent.h"
#include "Shared/RmlModel_AnchorComponent.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "Shared/RmlDataBinding_List.h"
#include "AnchorObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_AnchorComponent::RmlModel_AnchorComponent()
	: RmlModel_MikanComponent()
	, m_stencilComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_AnchorComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<AnchorComponent>(rmlContext);
}

bool RmlModel_AnchorComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all anchor component IDs by collecting from the anchor systems
	m_stencilComponentIdList->init(
		constructor,
		CommonConfigPtr(),
		"anchor_component_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) 
		{
			// Collect anchor components from anchor system
			auto anchorObjectSystem = getAnchorObjectSystem();
			if (anchorObjectSystem)
			{
				for (const auto& it : anchorObjectSystem->getAnchorMap())
				{
					// Use the anchor ID as the identifier
					outComponentIdList.push_back((int)it.first);
				}
			}
		});

	return true;
}

bool RmlModel_AnchorComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_stencilComponentIdList->setOwnerConfig(getAnchorObjectSystemConfig());
		m_stencilComponentIdList->rebuildList(true);

		return true;
	}

	return false;
}

AnchorObjectSystemPtr RmlModel_AnchorComponent::getAnchorObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<AnchorObjectSystem>();
	}

	return nullptr;
}

AnchorObjectSystemDefinitionPtr RmlModel_AnchorComponent::getAnchorObjectSystemConfig() const
{
	auto anchorObjectSystem = getAnchorObjectSystem();
	if (anchorObjectSystem)
	{
		return anchorObjectSystem->getAnchorSystemConfig();
	}

	return nullptr;
}