#include "MarkerTrackingVolumeComponent.h"
#include "MarkerTrackingVolumeSystem.h"
#include "MikanCoreTypes.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "RmlModel_ProjectTracking.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "Shared/RmlModel_MarkerTrackingVolumeComponent.h"
#include "Shared/RmlModel_VRTrackingVolumeComponent.h"
#include "Shared/RmlModel_TrackingMountComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "StringUtils.h"
#include "TrackingMountComponent.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingVolumeQueries.h"
#include "VRTrackingVolumeComponent.h"
#include "VRTrackingVolumeSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectTracking::RmlModel_ProjectTracking()
	: m_trackingVolumeIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{
}

bool RmlModel_ProjectTracking::init(ProjectRmlModelContext* context)
{
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_trackingMountSystem = ownerAppStage->getObjectSystemOfType<TrackingMountObjectSystem>();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "TrackingSystems");
	if (!constructor)
		return false;

	// Get project manager for tracking volume queries
	ProjectManagerPtr projectManager = ownerAppStage->getProjectManager();
	auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
	auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();

	// Register component lists
	m_trackingVolumeIdList->init(
		constructor,
		vrTrackingVolumeSystem->getVRTrackingVolumeSystemConfig(),
		"tracker_volume_ids", // virtual property name since this is a composite list
		[projectManager](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			outComponentIdList = TrackingVolumeQueries::getTrackingVolumeIdList(projectManager);
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) {
			return
				changedPropertySet.hasPropertyName(VRTrackingVolumeSystemDefinition::k_vrTrackingVolumeListPropertyId) ||
				changedPropertySet.hasPropertyName(MarkerTrackingVolumeSystemDefinition::k_markerTrackingVolumeListPropertyId);
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

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_steamvr_tracking_volume", &RmlModel_ProjectTracking::addNewSteamVRTrackingVolume, this);
	constructor.BindEventCallback("add_new_marker_tracking_volume", &RmlModel_ProjectTracking::addNewMarkerTrackingVolume, this);
	constructor.BindEventCallback("remove_tracking_volume", &RmlModel_ProjectTracking::removeTrackingVolume, this);
	constructor.BindEventCallback("add_new_tracking_mount", &RmlModel_ProjectTracking::addNewTrackingMount, this);
	constructor.BindEventCallback("remove_tracking_mount", &RmlModel_ProjectTracking::removeTrackingMountID, this);
	constructor.BindEventCallback("select_tracking_volume_entry", &RmlModel_ProjectTracking::selectTrackingVolumeEntry, this);
	constructor.BindEventCallback("select_tracking_mount_entry", &RmlModel_ProjectTracking::selectTrackingMountEntry, this);

	// Listen for tracking system config changes
	TrackingMountObjectSystemConfigPtr trackingMountConfig = 
		m_trackingMountSystem.lock()->getTrackingMountSystemConfig();
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

TrackingMountObjectSystemPtr RmlModel_ProjectTracking::getTrackingMountSystem()
{
	return m_trackingMountSystem.lock();
}

MarkerTrackingVolumeComponentPtr RmlModel_ProjectTracking::getSelectedMarkerTrackingVolume()
{
	ProjectManagerPtr projectManager = m_projectRmlModelContext->getOwnerAppStage()->getProjectManager();
	auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();
	return markerTrackingVolumeSystem->getMarkerTrackingVolumeById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
}

VRTrackingVolumeComponentPtr RmlModel_ProjectTracking::getSelectedVRTrackingVolume()
{
	ProjectManagerPtr projectManager = m_projectRmlModelContext->getOwnerAppStage()->getProjectManager();
	auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
	return vrTrackingVolumeSystem->getVRTrackingVolumeById((MikanTrackingVolumeID)m_selectedTrackingVolumeId);
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
	ProjectManagerPtr projectManager = m_projectRmlModelContext->getOwnerAppStage()->getProjectManager();
	auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
	VRTrackingVolumeComponentPtr trackingVolume=
		vrTrackingVolumeSystem->addNewVRTrackingVolume(eTrackingRuntime::SteamVR);
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
	ProjectManagerPtr projectManager = m_projectRmlModelContext->getOwnerAppStage()->getProjectManager();
	auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();
	MarkerTrackingVolumeComponentPtr trackingVolume= markerTrackingVolumeSystem->addNewMarkerTrackingVolume();
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
	ProjectManagerPtr projectManager = m_projectRmlModelContext->getOwnerAppStage()->getProjectManager();
	TrackingVolumeQueries::removeTrackingVolume(projectManager, m_selectedTrackingVolumeId);
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
			m_projectRmlModelContext->getVRTrackingVolumeModel()->setComponent(vrTrackingComponent);
			m_projectRmlModelContext->getMarkerTrackingVolumeModel()->setComponent(nullptr);

			m_trackingMountIdList->setOwnerConfig(vrTrackingComponent->getDefinition());
		}
		else if (MarkerTrackingVolumeComponentPtr markerComponent = getSelectedMarkerTrackingVolume())
		{
			m_isVRTrackingVolume = false;
			m_projectRmlModelContext->getVRTrackingVolumeModel()->setComponent(nullptr);
			m_projectRmlModelContext->getMarkerTrackingVolumeModel()->setComponent(markerComponent);

			m_trackingMountIdList->setOwnerConfig(CommonConfigPtr());
		}
		else
		{
			m_isVRTrackingVolume = false;
			m_projectRmlModelContext->getVRTrackingVolumeModel()->setComponent(nullptr);
			m_projectRmlModelContext->getMarkerTrackingVolumeModel()->setComponent(nullptr);

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
			m_projectRmlModelContext->getTrackingMountModel()->setComponent(trackingMount);
		}
		else
		{
			m_projectRmlModelContext->getTrackingMountModel()->setComponent(nullptr);
		}
	}
}