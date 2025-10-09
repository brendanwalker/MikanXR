#include "MarkerTrackingVolumeComponent.h"
#include "MikanCoreTypes.h"
#include "ProjectConfig.h"
#include "RmlModel_ProjectTracking.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "Shared/RmlModel_MarkerTrackingVolumeComponent.h"
#include "Shared/RmlModel_VRTrackingVolumeComponent.h"
#include "Shared/RmlModel_TrackingMountComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "StringUtils.h"
#include "TrackingMountComponent.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingVolumeObjectSystem.h"
#include "VRTrackingVolumeComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectTracking::RmlModel_ProjectTracking()
	: m_trackingVolumeIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_selectedVRTrackingVolumeModel(std::make_shared<RmlModel_VRTrackingVolumeComponent>())
	, m_selectedMarkerTrackingVolumeModel(std::make_shared<RmlModel_MarkerTrackingVolumeComponent>())
	, m_selectedTrackingMountModel(std::make_shared<RmlModel_TrackingMountComponent>())
{
}

bool RmlModel_ProjectTracking::init(
	Rml::Context* rmlContext, 
	ProjectConfigPtr projectConfig,
	TrackingVolumeObjectSystemPtr trackingVolumeSystem,
	TrackingMountObjectSystemPtr trackingMountSystem)
{
	TrackingVolumeObjectSystemConfigPtr trackingVolumeConfig= projectConfig->trackingVolumeSystemConfig;

	m_projectConfig = projectConfig;
	m_trackingVolumeSystem = trackingVolumeSystem;
	m_trackingMountSystem = trackingMountSystem;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "TrackingSystems");
	if (!constructor)
		return false;

	// Register component lists
	m_trackingVolumeIdList->init(
		constructor, 
		trackingVolumeConfig,
		"tracker_volume_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			TrackingVolumeObjectSystemConfigPtr trackingConfig = m_projectConfig.lock()->trackingVolumeSystemConfig;

			for (const auto& vrTrackingVolumePtr : trackingConfig->getVRTrackingVolumeList())
			{
				if (vrTrackingVolumePtr)
				{
					outComponentIdList.push_back((int)vrTrackingVolumePtr->getTrackingVolumeId());
				}
			}
			for (const auto& markerSystemPtr : trackingConfig->getMarkerTrackingVolumeList())
			{
				if (markerSystemPtr)
				{
					outComponentIdList.push_back((int)markerSystemPtr->getTrackingVolumeId());
				}
			}
		});
	m_trackingMountIdList->init(
		constructor,
		CommonConfigPtr(),
		"tracking_mount_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			if (ownerConfig)
			{
				auto vrTrackingVolumeDefinition= std::static_pointer_cast<VRTrackingVolumeDefinition>(ownerConfig);
				
				for (const MikanTrackingMountID& trackingMountId : vrTrackingVolumeDefinition->getTrackingMountIDs())
				{
					outComponentIdList.push_back(trackingMountId);
				}
			}
		});

	// Register Data Model Fields
	constructor.Bind("selected_tracking_volume_id", &m_selectedTrackingVolumeId);
	constructor.Bind("selected_tracking_mount_id", &m_selectedTrackingMountId);

	// Register Selected Object Models
	m_selectedVRTrackingVolumeModel->init(rmlContext);
	m_selectedMarkerTrackingVolumeModel->init(rmlContext);
	m_selectedTrackingMountModel->init(rmlContext);

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_steamvr_tracking_volume", &RmlModel_ProjectTracking::addNewSteamVRTrackingVolume, this);
	constructor.BindEventCallback("add_new_marker_tracking_volume", &RmlModel_ProjectTracking::addNewMarkerTrackingVolume, this);
	constructor.BindEventCallback("remove_tracking_volume", &RmlModel_ProjectTracking::removeTrackingVolume, this);
	constructor.BindEventCallback("add_new_tracking_mount", &RmlModel_ProjectTracking::addNewTrackingMount, this);
	constructor.BindEventCallback("remove_tracking_mount", &RmlModel_ProjectTracking::removeTrackingMountID, this);
	constructor.BindEventCallback("select_tracking_volume_entry", &RmlModel_ProjectTracking::selectTrackingVolumeEntry, this);
	constructor.BindEventCallback("select_tracking_mount_entry", &RmlModel_ProjectTracking::selectTrackingMountEntry, this);

	// Listen for tracking system config changes
	m_trackingVolumeIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectTracking::trackingVolumeIdListChanged);
	m_trackingMountIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectTracking::trackingMountIdListChanged);

	return true;
}

void RmlModel_ProjectTracking::dispose()
{
	m_trackingVolumeIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectTracking::trackingVolumeIdListChanged);
	m_trackingMountIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectTracking::trackingMountIdListChanged);

	m_selectedVRTrackingVolumeModel->dispose();
	m_selectedMarkerTrackingVolumeModel->dispose();
	m_selectedTrackingMountModel->dispose();

	RmlModel::dispose();
}

void RmlModel_ProjectTracking::trackingVolumeIdListChanged(bool bOwnerChanged)
{
	MikanTrackingVolumeID selectedTrackingVolumeId = INVALID_MIKAN_ID;
	if (!m_trackingVolumeIdList->isEmpty() &&
		!m_trackingVolumeIdList->contains(m_selectedTrackingVolumeId))
	{
		selectedTrackingVolumeId = m_trackingVolumeIdList->getFirstValue();
	}

	setSelectedTrackingVolumeId(selectedTrackingVolumeId);
}

void RmlModel_ProjectTracking::trackingMountIdListChanged(bool bOwnerChanged)
{
	MikanTrackingMountID selectedTrackingMountId = INVALID_MIKAN_ID;
	if (!m_trackingMountIdList->isEmpty() &&
		!m_trackingMountIdList->contains(m_selectedTrackingMountId))
	{
		selectedTrackingMountId = m_trackingMountIdList->getFirstValue();
	}
	setSelectedTrackingMountId(selectedTrackingMountId);
}

TrackingVolumeObjectSystemPtr RmlModel_ProjectTracking::getTrackingVolumeSystem()
{
	return m_trackingVolumeSystem.lock();
}

TrackingMountObjectSystemPtr RmlModel_ProjectTracking::getTrackingMountSystem()
{
	return m_trackingMountSystem.lock();
}

TrackingVolumeComponentPtr RmlModel_ProjectTracking::getSelectedTrackingVolume()
{
	return getTrackingVolumeSystem()->getTrackingVolumeById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
}

MarkerTrackingVolumeComponentPtr RmlModel_ProjectTracking::getSelectedMarkerTrackingVolume()
{
	return getTrackingVolumeSystem()->getMarkerTrackingVolumeById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
}

VRTrackingVolumeComponentPtr RmlModel_ProjectTracking::getSelectedVRTrackingVolume()
{
	return getTrackingVolumeSystem()->getVRTrackingVolumeById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
}

TrackingMountComponentPtr RmlModel_ProjectTracking::getSelectedTrackingMount()
{
	return getTrackingMountSystem()->getTrackingMountById((MikanTrackingMountID)m_selectedTrackingMountId);
}

void RmlModel_ProjectTracking::addNewSteamVRTrackingVolume(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getTrackingVolumeSystem()->addNewVRTrackingVolume(eTrackingRuntime::SteamVR);
}

void RmlModel_ProjectTracking::addNewMarkerTrackingVolume(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getTrackingVolumeSystem()->addNewMarkerTrackingVolume();
}

void RmlModel_ProjectTracking::removeTrackingVolume(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{	
	getTrackingVolumeSystem()->removeTrackingVolume(m_selectedTrackingVolumeId);
}

void RmlModel_ProjectTracking::addNewTrackingMount(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	VRTrackingVolumeComponentPtr vrTrackingVolumePtr = getSelectedVRTrackingVolume();
	if (vrTrackingVolumePtr)
	{
		TrackingMountComponentPtr trackingMount= getTrackingMountSystem()->addNewTrackingMount();

		vrTrackingVolumePtr->getVRTrackingVolumeDefinition()->addTrackingMountID(
			trackingMount->getTrackingMountDefinition()->getTrackingMountId());
	}
}

void RmlModel_ProjectTracking::removeTrackingMountID(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.size() < 1)
		return;

	const int mountId = parameters[0].Get<int>();

	VRTrackingVolumeComponentPtr vrTrackingVolumePtr = getSelectedVRTrackingVolume();
	if (vrTrackingVolumePtr)
	{
		vrTrackingVolumePtr->getVRTrackingVolumeDefinition()->removeTrackingMountID((MikanTrackingMountID)mountId);
		getTrackingMountSystem()->removeTrackingMountID((MikanTrackingMountID)mountId);
	}
}

void RmlModel_ProjectTracking::selectTrackingVolumeEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int newVolumeId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedTrackingVolumeId(newVolumeId);
}

void RmlModel_ProjectTracking::selectTrackingMountEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int newMountId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedTrackingMountId(newMountId);
}

void RmlModel_ProjectTracking::setSelectedTrackingVolumeId(MikanTrackingVolumeID trackingVolumeId)
{
	if (trackingVolumeId != m_selectedTrackingVolumeId)
	{
		m_selectedTrackingVolumeId = (int)trackingVolumeId;
		m_modelHandle.DirtyVariable("selected_tracking_volume_id");

		if (VRTrackingVolumeComponentPtr vrTrackingComponent = getSelectedVRTrackingVolume())
		{
			m_selectedVRTrackingVolumeModel->setComponent(vrTrackingComponent);
			m_selectedMarkerTrackingVolumeModel->setComponent(nullptr);

			m_trackingMountIdList->setOwnerConfig(vrTrackingComponent->getDefinition());
		}
		else if (MarkerTrackingVolumeComponentPtr markerComponent = getSelectedMarkerTrackingVolume())
		{
			m_selectedVRTrackingVolumeModel->setComponent(nullptr);
			m_selectedMarkerTrackingVolumeModel->setComponent(markerComponent);

			m_trackingMountIdList->setOwnerConfig(CommonConfigPtr());
		}
		else
		{
			m_selectedVRTrackingVolumeModel->setComponent(nullptr);
			m_selectedMarkerTrackingVolumeModel->setComponent(nullptr);

			m_trackingMountIdList->setOwnerConfig(CommonConfigPtr());
		}
	}
}

void RmlModel_ProjectTracking::setSelectedTrackingMountId(MikanTrackingMountID trackingMountId)
{
	if (trackingMountId != m_selectedTrackingMountId)
	{
		m_selectedTrackingMountId = (int)trackingMountId;
		m_modelHandle.DirtyVariable("selected_tracking_mount_id");

		if (TrackingMountComponentPtr trackingMount = getSelectedTrackingMount())
		{
			m_selectedTrackingMountModel->setComponent(trackingMount);
		}
		else
		{
			m_selectedTrackingMountModel->setComponent(nullptr);
		}
	}
}