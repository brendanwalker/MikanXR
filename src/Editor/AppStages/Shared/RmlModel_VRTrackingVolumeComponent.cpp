#include "AppStage.h"
#include "RmlModel_VRTrackingVolumeComponent.h"
#include "RmlModel_EntityAccessor.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_VRTrackingVolumeComponent::RmlModel_VRTrackingVolumeComponent()
	: RmlModel_MikanComponent()
	, m_markerComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_VRTrackingVolumeComponent::init(AppStage* ownerAppStage)
{
	m_markerObjectSystem = ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>();
	m_trackingMountObjectSystem = ownerAppStage->getObjectSystemOfType<TrackingMountObjectSystem>();

	return initTypedPropertyInterface<VRTrackingVolumeComponent>(ownerAppStage->getRmlContext());
}

bool RmlModel_VRTrackingVolumeComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all marker component IDs from the MarkerObjectSystem
	m_markerComponentIdList->init(
		constructor,
		getMarkerObjectSystem(),
		MarkerObjectSystemDefinition::k_componentIdListPropertyId,
		true);

	constructor.BindEventCallback(
		"select_charuco_mount_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int newMountId = ev.GetParameter<int>("value", 0);
			VRTrackingVolumeComponentPtr volumeComponent = getVRTrackingVolumeComponent();
			if (volumeComponent)
			{
				volumeComponent->getVRTrackingVolumeDefinition()->setCharucoTrackingMountId(newMountId);
			}
		});

	constructor.BindEventCallback(
		"select_origin_marker_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int markerId = ev.GetParameter<int>("value", 0);
			VRTrackingVolumeComponentPtr volumeComponent = getVRTrackingVolumeComponent();
			if (volumeComponent)
			{
				volumeComponent->getVRTrackingVolumeDefinition()->setOriginMarkerId(markerId);
			}
		});

	constructor.BindEventCallback(
		"select_utility_marker_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int markerId = ev.GetParameter<int>("value", 0);
			VRTrackingVolumeComponentPtr volumeComponent = getVRTrackingVolumeComponent();
			if (volumeComponent)
			{
				volumeComponent->getVRTrackingVolumeDefinition()->setUtilityMarkerId(markerId);
			}
		});

	return true;
}

MarkerObjectSystemPtr RmlModel_VRTrackingVolumeComponent::getMarkerObjectSystem() const
{
	return m_markerObjectSystem.lock();
}

MarkerObjectSystemDefinitionPtr RmlModel_VRTrackingVolumeComponent::getMarkerObjectSystemDefinition() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getTypedDefinition();
	}

	return nullptr;
}

TrackingMountObjectSystemPtr RmlModel_VRTrackingVolumeComponent::getTrackingMountObjectSystem() const
{
	return m_trackingMountObjectSystem.lock();
}

TrackingMountObjectSystemDefinitionPtr RmlModel_VRTrackingVolumeComponent::getTrackingMountObjectSystemConfig() const
{
	auto trackingMountObjectSystem = getTrackingMountObjectSystem();
	if (trackingMountObjectSystem)
	{
		return trackingMountObjectSystem->getTypedDefinition();
	}

	return nullptr;
}

VRTrackingVolumeComponentPtr RmlModel_VRTrackingVolumeComponent::getVRTrackingVolumeComponent() const
{
	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(m_component.lock());
}