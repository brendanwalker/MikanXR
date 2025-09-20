#include "MarkerComponent.h"
#include "Shared/RmlModel_MarkerComponent.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "Shared/RmlDataBinding_List.h"
#include "MarkerObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_MarkerComponent::RmlModel_MarkerComponent()
	: RmlModel_MikanComponent()
	, m_arucoIdList(std::make_shared<RmlDataBinding_ArucoIdList>())
{}

bool RmlModel_MarkerComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<MarkerComponent>(
			rmlContext,
			"MarkerComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool 
			{
				// Build the list of all aruco IDs by collecting from the marker system config
				m_arucoIdList->init(
					constructor,
					CommonConfigPtr(),
					MarkerObjectSystemConfig::k_arucoIdListPropertyId,
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outArucoIdList) 
					{
						auto markerObjectSystemConfig = getMarkerObjectSystemConfig();
						if (markerObjectSystemConfig)
						{
							markerObjectSystemConfig->getArucoIdList(outArucoIdList);
						}
					});

				return true;
			});

	return true;
}

bool RmlModel_MarkerComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_arucoIdList->setOwnerConfig(getMarkerObjectSystemConfig());
		m_arucoIdList->rebuildList(true);

		return true;
	}

	return false;
}

MarkerObjectSystemPtr RmlModel_MarkerComponent::getMarkerObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<MarkerObjectSystem>();
	}

	return nullptr;
}

MarkerObjectSystemConfigPtr RmlModel_MarkerComponent::getMarkerObjectSystemConfig() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getMarkerSystemConfig();
	}

	return nullptr;
}