#include "TransformComponent.h"
#include "RmlModel_TransformComponent.h"
#include "AnchorObjectSystem.h"
#include "StencilObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_TransformComponent::RmlModel_TransformComponent()
	: RmlModel_MikanComponent()
	, m_transformComponentIdList(std::make_shared<RmlDataBinding_ComponentList>())
{}

bool RmlModel_TransformComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<TransformComponent>(
			rmlContext,
			"TransformComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool {

				// Build the list of all transform component IDs by collecting from anchor and stencil systems
				m_transformComponentIdList->init(
					constructor,
					CommonConfigPtr(),
					"transform_component_ids",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
						// Collect transform components from anchor system
						auto anchorObjectSystem = getAnchorObjectSystem();
						if (anchorObjectSystem)
						{
							for (const auto& it : anchorObjectSystem->getAnchorMap())
							{
								// Use the anchor ID as the identifier
								outComponentIdList.push_back((int)it.first);
							}
						}

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

bool RmlModel_TransformComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_transformComponentIdList->setOwnerConfig(getAnchorObjectSystemConfig());
		m_transformComponentIdList->rebuildComponentIdList(true);

		return true;
	}

	return false;
}

AnchorObjectSystemPtr RmlModel_TransformComponent::getAnchorObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<AnchorObjectSystem>();
	}

	return nullptr;
}

AnchorObjectSystemConfigPtr RmlModel_TransformComponent::getAnchorObjectSystemConfig() const
{
	auto anchorObjectSystem = getAnchorObjectSystem();
	if (anchorObjectSystem)
	{
		return anchorObjectSystem->getAnchorSystemConfig();
	}

	return nullptr;
}

StencilObjectSystemPtr RmlModel_TransformComponent::getStencilObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<StencilObjectSystem>();
	}

	return nullptr;
}

StencilObjectSystemConfigPtr RmlModel_TransformComponent::getStencilObjectSystemConfig() const
{
	auto stencilObjectSystem = getStencilObjectSystem();
	if (stencilObjectSystem)
	{
		return stencilObjectSystem->getStencilSystemConfig();
	}

	return nullptr;
}