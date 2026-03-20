#include "AppStage.h"
#include "Shared/RmlModel_StencilComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "AnchorObjectSystem.h"
#include "QuadStencilComponent.h"
#include "BoxStencilComponent.h"
#include "ModelStencilComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

// -- RmlModel_StencilComponent -----
RmlModel_StencilComponent::RmlModel_StencilComponent()
	: RmlModel_MikanComponent()
	, m_anchorComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_StencilComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
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
				for (const auto& it : anchorObjectSystem->getComponentMap())
				{
					outComponentIdList.push_back((int)it.first);
				}
			}
		});

	constructor.BindEventCallback(
		"select_cull_mode_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto cullMode = (eStencilCullMode)ev.GetParameter<int>("value", 0);
			auto stencilComponent = getStencilComponent();
			if (stencilComponent)
				stencilComponent->getStencilComponentDefinition()->setCullMode(cullMode);
		});

	return true;
}

bool RmlModel_StencilComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
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

AnchorObjectSystemDefinitionPtr RmlModel_StencilComponent::getAnchorObjectSystemConfig() const
{
	auto anchorObjectSystem = getAnchorObjectSystem();
	if (anchorObjectSystem)
	{
		return anchorObjectSystem->getTypedDefinition();
	}

	return nullptr;
}

//-- RmlModel_QuadStencilComponent -----
bool RmlModel_QuadStencilComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<QuadStencilComponent>(ownerAppStage->getRmlContext());
}

//-- RmlModel_BoxStencilComponent -----
bool RmlModel_BoxStencilComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<BoxStencilComponent>(ownerAppStage->getRmlContext());
}

//-- RmlModel_ModelStencilComponent -----
bool RmlModel_ModelStencilComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<ModelStencilComponent>(ownerAppStage->getRmlContext());
}
