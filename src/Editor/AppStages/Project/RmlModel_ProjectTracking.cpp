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
		trackingVolumeSystem->getTrackingVolumeSystemConfig(),
		"tracker_volume_ids", // virtual property name since this is a composite list
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
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) {
			return 
				changedPropertySet.hasPropertyName(TrackingVolumeObjectSystemConfig::k_vrTrackingVolumeListPropertyId) ||
				changedPropertySet.hasPropertyName(TrackingVolumeObjectSystemConfig::k_markerTrackingVolumeListPropertyId);
		});
	m_trackingMountIdList->init(
		constructor,
		CommonConfigPtr(),
		VRTrackingVolumeDefinition::k_trackingMountIdsPropertyId,
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
	constructor.Bind("is_vr_tracking_volume", &m_isVRTrackingVolume);
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
	TrackingMountObjectSystemConfigPtr trackingMountConfig = trackingMountSystem->getTrackingMountSystemConfig();
	m_trackingVolumeIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectTracking::trackingVolumeIdListChanged);
	m_trackingMountIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectTracking::trackingMountIdListChanged);

	// Auto-select first items if available
	if (!m_trackingVolumeIdList->isEmpty())
	{
		setSelectedTrackingVolumeId(m_trackingVolumeIdList->getFirstValue());
	}

	return true;
}

void RmlModel_ProjectTracking::dispose()
{
	TrackingMountObjectSystemConfigPtr trackingMountConfig = 
		m_trackingMountSystem.lock()->getTrackingMountSystemConfig();

	m_trackingVolumeIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectTracking::trackingVolumeIdListChanged);
	m_trackingMountIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectTracking::trackingMountIdListChanged);

	m_selectedVRTrackingVolumeModel->dispose();
	m_selectedMarkerTrackingVolumeModel->dispose();
	m_selectedTrackingMountModel->dispose();

	RmlModel::dispose();
}

void RmlModel_ProjectTracking::trackingVolumeIdListChanged(bool bOwnerChanged)
{
	MikanTrackingVolumeID newSelectedTrackingVolumeId= m_selectedTrackingVolumeId;
	if (m_trackingVolumeIdList->isEmpty())
	{
		newSelectedTrackingVolumeId = INVALID_MIKAN_ID;
	}
	else if (!m_trackingVolumeIdList->contains(m_selectedTrackingVolumeId))
	{
		newSelectedTrackingVolumeId = m_trackingVolumeIdList->getFirstValue();
	}

	// Defer the selection update to post view update after element list refreshes
	if (newSelectedTrackingVolumeId != m_selectedTrackingVolumeId)
	{
		addModelUpdateCallback([this, newSelectedTrackingVolumeId]() {
			setSelectedTrackingVolumeId(newSelectedTrackingVolumeId);
		});
	}
}

void RmlModel_ProjectTracking::trackingMountIdListChanged(bool bOwnerChanged)
{
	MikanTrackingMountID newSelectedTrackingMountId = m_selectedTrackingMountId;
	if (m_trackingMountIdList->isEmpty())
	{
		newSelectedTrackingMountId = INVALID_MIKAN_ID;
	}
	else if (!m_trackingMountIdList->contains(m_selectedTrackingMountId))
	{
		newSelectedTrackingMountId = m_trackingMountIdList->getFirstValue();
	}

	if (newSelectedTrackingMountId != m_selectedTrackingMountId)
	{
		// Defer the selection update to post view update after element list refreshes
		addModelUpdateCallback([this, newSelectedTrackingMountId]() {
			setSelectedTrackingMountId(newSelectedTrackingMountId);
			});
	}
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
	VRTrackingVolumeComponentPtr trackingVolume= 
		getTrackingVolumeSystem()->addNewVRTrackingVolume(eTrackingRuntime::SteamVR);
	MikanTrackingVolumeID volumeId = trackingVolume->getVRTrackingVolumeDefinition()->getTrackingVolumeId();

	addModelUpdateCallback([this, volumeId]() {
		setSelectedTrackingVolumeId(volumeId);
	});
}

void RmlModel_ProjectTracking::addNewMarkerTrackingVolume(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	MarkerTrackingVolumeComponentPtr trackingVolume= getTrackingVolumeSystem()->addNewMarkerTrackingVolume();
	MikanTrackingVolumeID volumeId= trackingVolume->getMarkerTrackingVolumeDefinition()->getTrackingVolumeId();

	addModelUpdateCallback([this, volumeId]() {
		setSelectedTrackingVolumeId(volumeId);
	});
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
		MikanTrackingMountID mountId= trackingMount->getTrackingMountDefinition()->getTrackingMountId();

		vrTrackingVolumePtr->getVRTrackingVolumeDefinition()->addTrackingMountID(mountId);

		addModelUpdateCallback([this, mountId]() {
			setSelectedTrackingMountId(mountId);
		});
	}
}

void RmlModel_ProjectTracking::removeTrackingMountID(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (m_selectedTrackingMountId != INVALID_MIKAN_ID)
	{
		VRTrackingVolumeComponentPtr vrTrackingVolumePtr = getSelectedVRTrackingVolume();
		if (vrTrackingVolumePtr)
		{
			const auto definition= vrTrackingVolumePtr->getVRTrackingVolumeDefinition();

			definition->removeTrackingMountID((MikanTrackingMountID)m_selectedTrackingMountId);
			getTrackingMountSystem()->removeTrackingMountID((MikanTrackingMountID)m_selectedTrackingMountId);
		}
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
			m_isVRTrackingVolume = true;
			m_selectedVRTrackingVolumeModel->setComponent(vrTrackingComponent);
			m_selectedMarkerTrackingVolumeModel->setComponent(nullptr);

			m_trackingMountIdList->setOwnerConfig(vrTrackingComponent->getDefinition());
		}
		else if (MarkerTrackingVolumeComponentPtr markerComponent = getSelectedMarkerTrackingVolume())
		{
			m_isVRTrackingVolume = false;
			m_selectedVRTrackingVolumeModel->setComponent(nullptr);
			m_selectedMarkerTrackingVolumeModel->setComponent(markerComponent);

			m_trackingMountIdList->setOwnerConfig(CommonConfigPtr());
		}
		else
		{
			m_isVRTrackingVolume = false;
			m_selectedVRTrackingVolumeModel->setComponent(nullptr);
			m_selectedMarkerTrackingVolumeModel->setComponent(nullptr);

			m_trackingMountIdList->setOwnerConfig(CommonConfigPtr());
		}

		m_modelHandle.DirtyVariable("is_vr_tracking_volume");
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