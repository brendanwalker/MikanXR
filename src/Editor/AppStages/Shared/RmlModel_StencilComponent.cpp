#include "TransformComponent.h"
#include "RmlModel_StencilComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "StencilObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_StencilComponent::RmlModel_StencilComponent()
	: RmlModel_MikanComponent()
	, m_stencilComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_StencilComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<TransformComponent>(
			rmlContext,
			"StencilComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool 
			{
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
			});

	return true;
}

bool RmlModel_StencilComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
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