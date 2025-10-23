#include "Shared/RmlModel_StencilComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "StencilObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_StencilComponent::RmlModel_StencilComponent()
	: RmlModel_TypedMikanComponent<StencilComponent>()
	, m_stencilComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_StencilComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_TypedMikanComponent<StencilComponent>::onConstruct(constructor))
		return false;

	// Build the list of all stencil component IDs by collecting from stencil systems
	m_stencilComponentIdList->init(
		constructor,
		CommonConfigPtr(),
		"stencil_component_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) 
		{
			// Collect transform components from stencil system
			auto stencilObjectSystem = getStencilObjectSystem();
			if (stencilObjectSystem)
			{
				// Add quad stencil IDs
				for (const auto& it : stencilObjectSystem->getQuadStencilMap())
				{
					outComponentIdList.push_back((int)it.first);
				}

				// Add box stencil IDs
				for (const auto& it : stencilObjectSystem->getBoxStencilMap())
				{
					outComponentIdList.push_back((int)it.first);
				}

				// Add model stencil IDs
				for (const auto& it : stencilObjectSystem->getModelStencilMap())
				{
					outComponentIdList.push_back((int)it.first);
				}
			}
		});

	return true;
}

bool RmlModel_StencilComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_TypedMikanComponent<StencilComponent>::setComponent(component))
	{
		m_stencilComponentIdList->setOwnerConfig(getStencilObjectSystemConfig());
		m_stencilComponentIdList->rebuildList(true);

		return true;
	}

	return false;
}

StencilObjectSystemPtr RmlModel_StencilComponent::getStencilObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<StencilObjectSystem>();
	}

	return nullptr;
}

StencilObjectSystemConfigPtr RmlModel_StencilComponent::getStencilObjectSystemConfig() const
{
	auto stencilObjectSystem = getStencilObjectSystem();
	if (stencilObjectSystem)
	{
		return stencilObjectSystem->getStencilSystemConfig();
	}

	return nullptr;
}