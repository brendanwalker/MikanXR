#include "Shared/RmlModel_StencilComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "AnchorObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_StencilComponent::RmlModel_StencilComponent()
	: RmlModel_TypedMikanComponent<StencilComponent>()
	, m_anchorComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_StencilComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_TypedMikanComponent<StencilComponent>::onConstruct(constructor))
		return false;

	// Build the list of all stencil component IDs by collecting from stencil systems
	m_anchorComponentIdList->init(
		constructor,
		CommonConfigPtr(),
		"anchor_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) 
		{
			// Collect transform components from stencil system
			auto anchorObjectSystem = getAnchorObjectSystem();
			if (anchorObjectSystem)
			{
				// Add "none" option first
				outComponentIdList.push_back(INVALID_MIKAN_ID); 

				// Add anchor IDs
				for (const auto& it : anchorObjectSystem->getAnchorMap())
				{
					outComponentIdList.push_back((int)it.first);
				}
			}
		});

	constructor.BindEventCallback(
		"select_cull_mode_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto cullMode = (eStencilCullMode)ev.GetParameter<int>("value", 0);

			getStencilComponent()->getStencilComponentDefinition()->setCullMode(cullMode);
		});

	constructor.BindEventCallback(
		"select_anchor_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int selectedAnchorId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

			getStencilComponent()->attachTransformComponentToAnchor(selectedAnchorId);
		});

	return true;
}

bool RmlModel_StencilComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_TypedMikanComponent<StencilComponent>::setComponent(component))
	{
		m_anchorComponentIdList->setOwnerConfig(getAnchorObjectSystemConfig());
		m_anchorComponentIdList->rebuildList(true);

		return true;
	}

	return false;
}

StencilComponentPtr RmlModel_StencilComponent::getStencilComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<StencilComponent>(component);
	}
	return nullptr;
}

AnchorObjectSystemPtr RmlModel_StencilComponent::getAnchorObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<AnchorObjectSystem>();
	}

	return nullptr;
}

AnchorObjectSystemConfigPtr RmlModel_StencilComponent::getAnchorObjectSystemConfig() const
{
	auto anchorObjectSystem = getAnchorObjectSystem();
	if (anchorObjectSystem)
	{
		return anchorObjectSystem->getAnchorSystemConfig();
	}

	return nullptr;
}