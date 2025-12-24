#include "RmlModel_VRTrackingVolumeComponent.h"
#include "RmlModel_PropertyInterface.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "Shared/RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_VRTrackingVolumeComponent::RmlModel_VRTrackingVolumeComponent()
	: RmlModel_MikanComponent()
	, m_markerComponentIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_VRTrackingVolumeComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<VRTrackingVolumeComponent>(rmlContext);
}

bool RmlModel_VRTrackingVolumeComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

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

bool RmlModel_VRTrackingVolumeComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_markerComponentIdList->setOwnerConfig(getMarkerObjectSystemConfig());
		m_markerComponentIdList->rebuildList(true);

		m_trackingMountIdList->setOwnerConfig(component ? component->getDefinition() : CommonConfigPtr());
		m_trackingMountIdList->rebuildList(true);

		return true;
	}

	return false;
}

MarkerObjectSystemPtr RmlModel_VRTrackingVolumeComponent::getMarkerObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<MarkerObjectSystem>();
	}

	return nullptr;
}

MarkerObjectSystemConfigPtr RmlModel_VRTrackingVolumeComponent::getMarkerObjectSystemConfig() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getMarkerSystemConfig();
	}

	return nullptr;
}

TrackingMountObjectSystemPtr RmlModel_VRTrackingVolumeComponent::getTrackingMountObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<TrackingMountObjectSystem>();
	}

	return nullptr;
}

TrackingMountObjectSystemConfigPtr RmlModel_VRTrackingVolumeComponent::getTrackingMountObjectSystemConfig() const
{
	auto trackingMountObjectSystem = getTrackingMountObjectSystem();
	if (trackingMountObjectSystem)
	{
		return trackingMountObjectSystem->getTrackingMountSystemConfig();
	}

	return nullptr;
}

VRTrackingVolumeComponentPtr RmlModel_VRTrackingVolumeComponent::getVRTrackingVolumeComponent() const
{
	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(m_component.lock());
}