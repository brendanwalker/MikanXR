#include "MarkerTrackingVolumeComponent.h"
#include "RmlModel_MarkerTrackingVolumeComponent.h"
#include "RmlModel_PropertyInterface.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_MarkerTrackingVolumeComponent::RmlModel_MarkerTrackingVolumeComponent()
	: RmlModel_TypedMikanComponent<MarkerTrackingVolumeComponent>()
	, m_markerComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_MarkerTrackingVolumeComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
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

	constructor.BindEventCallback(
		"select_origin_marker_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int markerId = ev.GetParameter<int>("value", 0);
			MarkerTrackingVolumeComponentPtr volumeComponent = getMarkerTrackingVolumeComponent();
			if (volumeComponent)
			{
				volumeComponent->getMarkerTrackingVolumeDefinition()->setOriginMarkerId(markerId);
			}
		});

	return true;
}

bool RmlModel_MarkerTrackingVolumeComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_TypedMikanComponent<MarkerTrackingVolumeComponent>::setComponent(component))
	{
		m_markerComponentIdList->setOwnerConfig(getMarkerObjectSystemConfig());
		m_markerComponentIdList->rebuildList(true);

		return true;
	}

	return false;
}

MarkerObjectSystemPtr RmlModel_MarkerTrackingVolumeComponent::getMarkerObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<MarkerObjectSystem>();
	}

	return nullptr;
}

MarkerObjectSystemConfigPtr RmlModel_MarkerTrackingVolumeComponent::getMarkerObjectSystemConfig() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getMarkerSystemConfig();
	}

	return nullptr;
}

TrackingMountObjectSystemPtr RmlModel_MarkerTrackingVolumeComponent::getTrackingMountObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<TrackingMountObjectSystem>();
	}

	return nullptr;
}

TrackingMountObjectSystemConfigPtr RmlModel_MarkerTrackingVolumeComponent::getTrackingMountObjectSystemConfig() const
{
	auto trackingMountObjectSystem = getTrackingMountObjectSystem();
	if (trackingMountObjectSystem)
	{
		return trackingMountObjectSystem->getTrackingMountSystemConfig();
	}

	return nullptr;
}

MarkerTrackingVolumeComponentPtr RmlModel_MarkerTrackingVolumeComponent::getMarkerTrackingVolumeComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(component);
	}
	return nullptr;
}