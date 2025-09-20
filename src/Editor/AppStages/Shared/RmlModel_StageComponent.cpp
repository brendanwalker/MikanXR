#include "StageComponent.h"
#include "RmlModel_StageComponent.h"
#include "TrackingVolumeObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_StageComponent::RmlModel_StageComponent()
	: RmlModel_MikanComponent()
	, m_trackingVolumeIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_StageComponent::init(Rml::Context* rmlContext)
{
	bool bSuccess=
		m_propertyInterface->init<StageComponent>(
			rmlContext,
			"StageComponent",
			[this](Rml::DataModelConstructor& constructor) -> bool {

				// Build the list of all tracking volume IDs from the TrackingVolumeObjectSystem
				m_trackingVolumeIdList->init(
					constructor,
					CommonConfigPtr(),
					"tracking_volume_ids",
					[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
						auto trackingVolumeObjectSystem= getTrackingVolumeObjectSystem();
						if (trackingVolumeObjectSystem)
						{
							outComponentIdList = trackingVolumeObjectSystem->getTrackingVolumeIdList();
						}
					});

				return true;
			});

	return true;
}

bool RmlModel_StageComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_trackingVolumeIdList->setOwnerConfig(getTrackingVolumeObjectSystemConfig());
		m_trackingVolumeIdList->rebuildList(true);

		return true;
	}

	return false;
}

TrackingVolumeObjectSystemPtr RmlModel_StageComponent::getTrackingVolumeObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<TrackingVolumeObjectSystem>();
	}

	return nullptr;
}

TrackingVolumeObjectSystemConfigPtr RmlModel_StageComponent::getTrackingVolumeObjectSystemConfig() const
{
	auto trackingVolumeObjectSystem = getTrackingVolumeObjectSystem();
	if (trackingVolumeObjectSystem)
	{
		return trackingVolumeObjectSystem->getTrackingVolumeSystemConfig();
	}

	return nullptr;
}