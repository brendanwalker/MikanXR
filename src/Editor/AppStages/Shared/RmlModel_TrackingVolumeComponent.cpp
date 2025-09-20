#include "TrackingVolumeComponent.h"
#include "RmlModel_TrackingVolumeComponent.h"
#include "RmlModel_PropertyInterface.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "VRTrackingVolumeComponent.h"
#include "Shared/RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_TrackingVolumeComponent::RmlModel_TrackingVolumeComponent()
	: RmlModel_MikanComponent()
	, m_markerComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_TrackingVolumeComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<TrackingVolumeComponent>(
			rmlContext,
			"TrackingVolumeComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool {

				// Build the list of all marker component IDs from the MarkerObjectSystem
				m_markerComponentIdList->init(
					constructor,
					CommonConfigPtr(),
					"marker_component_ids",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
						auto markerObjectSystem = getMarkerObjectSystem();
						if (markerObjectSystem)
						{
							for (const auto& it : markerObjectSystem->getMarkerMap())
							{
								outComponentIdList.push_back((int)it.first);
							}
						}
					});

				// Build the list of all tracking mount IDs from the TrackingMountObjectSystem
				m_trackingMountIdList->init(
					constructor,
					CommonConfigPtr(),
					"tracking_mount_ids",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
						auto vrTrackingVolumeDefinition = 
							dynamic_pointer_cast<VRTrackingVolumeDefinition>(ownerConfig);
						if (vrTrackingVolumeDefinition)
						{
							outComponentIdList= vrTrackingVolumeDefinition->getTrackingMountIDs();
						}
					});

				return true;
			});

	return true;
}

bool RmlModel_TrackingVolumeComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_markerComponentIdList->setOwnerConfig(getMarkerObjectSystemConfig());
		m_markerComponentIdList->rebuildList(true);

		m_trackingMountIdList->setOwnerConfig(component->getDefinition());
		m_trackingMountIdList->rebuildList(true);

		return true;
	}

	return false;
}

MarkerObjectSystemPtr RmlModel_TrackingVolumeComponent::getMarkerObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<MarkerObjectSystem>();
	}

	return nullptr;
}

MarkerObjectSystemConfigPtr RmlModel_TrackingVolumeComponent::getMarkerObjectSystemConfig() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getMarkerSystemConfig();
	}

	return nullptr;
}

TrackingMountObjectSystemPtr RmlModel_TrackingVolumeComponent::getTrackingMountObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<TrackingMountObjectSystem>();
	}

	return nullptr;
}

TrackingMountObjectSystemConfigPtr RmlModel_TrackingVolumeComponent::getTrackingMountObjectSystemConfig() const
{
	auto trackingMountObjectSystem = getTrackingMountObjectSystem();
	if (trackingMountObjectSystem)
	{
		return trackingMountObjectSystem->getTrackingMountSystemConfig();
	}

	return nullptr;
}