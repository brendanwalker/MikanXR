#include "ProjectConfig.h"
#include "RmlModel_ProjectTracking.h"
#include "TrackingSystemsConfig.h"
#include "VRTrackingSystemDefinition.h"
#include "MarkerTrackingSystemDefinition.h"
#include "TrackingMountDefinition.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectTracking::RmlModel_ProjectTracking()
	: m_trackingSystemIdList(std::make_shared<RmlDataBinding_ComponentList>())
	, m_trackingMountIdList(std::make_shared<RmlDataBinding_ComponentList>())
	, m_selectedVRTrackingSystemModel(std::make_shared<RmlModel_PropertyInterface>())
	, m_selectedMarkerTrackingSystemModel(std::make_shared<RmlModel_PropertyInterface>())
	, m_selectedTrackingMountModel(std::make_shared<RmlModel_PropertyInterface>())
{
}

bool RmlModel_ProjectTracking::init(
	Rml::Context* rmlContext, 
	ProjectConfigPtr projectConfig)
{
	TrackingSystemsConfigPtr trackingSystemConfig= projectConfig->trackingSystemsConfig;

	m_projectConfig = projectConfig;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "tracking_systems");
	if (!constructor)
		return false;

	// Register component lists
	m_trackingSystemIdList->init(
		constructor, 
		trackingSystemConfig,
		"tracker_system_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			TrackingSystemsConfigPtr trackingConfig = m_projectConfig.lock()->trackingSystemsConfig;

			for (const auto& vrSystemPtr : trackingConfig->getVRTrackingSystemList())
			{
				if (vrSystemPtr)
				{
					outComponentIdList.push_back((int)vrSystemPtr->getTrackingSystemId());
				}
			}
			for (const auto& markerSystemPtr : trackingConfig->getMarkerTrackingSystemList())
			{
				if (markerSystemPtr)
				{
					outComponentIdList.push_back((int)markerSystemPtr->getTrackingSystemId());
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
				auto vrTrackingSystemDefinition= std::static_pointer_cast<VRTrackingSystemDefinition>(ownerConfig);
				
				for (const auto& trackingMountPtr : vrTrackingSystemDefinition->getTrackingMounts())
				{
					if (trackingMountPtr)
					{
						outComponentIdList.push_back((int)trackingMountPtr->getTrackingMountId());
					}
				}
			}
		});

	// Register Data Model Fields
	constructor.Bind("selected_tracking_system_id", &m_selectedTrackingSystemId);
	constructor.Bind("selected_tracking_mount_id", &m_selectedTrackingMountId);

	// Register Selected Object Models
	// TODO
	//m_selectedVRTrackingSystemModel.init<VRTrackingSystemDefinition>(
	//	rmlContext,
	//	"vr_tracking_system_definition");
	//m_selectedMarkerTrackingSystemModel.init<MarkerTrackingSystemDefinition>(
	//	rmlContext,
	//	"marker_tracking_system_definition");
	//m_selectedTrackingMountModel.init<TrackingMountDefinition>(
	//	rmlContext,
	//	"tracking_mount_definition");

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_steamvr_tracking_system", &RmlModel_ProjectTracking::addNewSteamVRTrackingSystem, this);
	constructor.BindEventCallback("add_new_marker_tracking_system", &RmlModel_ProjectTracking::addNewMarkerTrackingSystem, this);
	constructor.BindEventCallback("remove_tracking_system", &RmlModel_ProjectTracking::removeTrackingSystem, this);
	constructor.BindEventCallback("add_new_tracking_mount", &RmlModel_ProjectTracking::addNewTrackingMount, this);
	constructor.BindEventCallback("remove_tracking_mount", &RmlModel_ProjectTracking::removeTrackingMount, this);
	constructor.BindEventCallback("select_tracking_system_entry", &RmlModel_ProjectTracking::selectTrackingSystemEntry, this);
	constructor.BindEventCallback("select_tracking_mount_entry", &RmlModel_ProjectTracking::selectTrackingMountEntry, this);

	// Listen for tracking system config changes
	m_trackingSystemIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectTracking::trackingSystemIdListChanged);
	m_trackingMountIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectTracking::trackingMountIdListChanged);

	return true;
}

void RmlModel_ProjectTracking::dispose()
{
	m_trackingSystemIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectTracking::trackingSystemIdListChanged);
	m_trackingMountIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectTracking::trackingMountIdListChanged);

	RmlModel::dispose();
}

void RmlModel_ProjectTracking::trackingSystemIdListChanged(bool bOwnerChanged)
{

}

void RmlModel_ProjectTracking::trackingMountIdListChanged(bool bOwnerChanged)
{
}

TrackingSystemsConfigPtr RmlModel_ProjectTracking::getTrackingSystemsConfig()
{
	return m_projectConfig.lock()->trackingSystemsConfig;
}

MarkerTrackingSystemDefinitionPtr RmlModel_ProjectTracking::getSelectedMarkerTrackingSystem()
{
	return getTrackingSystemsConfig()->getMarkerTrackingSystemConfig((MikanTrackingSystemID)m_selectedTrackingSystemId);
}

VRTrackingSystemDefinitionPtr RmlModel_ProjectTracking::getSelectedVRTrackingSystem()
{
	return getTrackingSystemsConfig()->getVRTrackingSystemConfig((MikanTrackingSystemID)m_selectedTrackingSystemId);
}

void RmlModel_ProjectTracking::addNewSteamVRTrackingSystem(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getTrackingSystemsConfig()->addVRTrackingSystem(eTrackingRuntime::SteamVR);
}

void RmlModel_ProjectTracking::addNewMarkerTrackingSystem(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getTrackingSystemsConfig()->addMarkerTrakingSystem();
}

void RmlModel_ProjectTracking::removeTrackingSystem(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int systemId = parameters[0].Get<int>();
	
	getTrackingSystemsConfig()->removeTrackingSystem((MikanTrackingSystemID)systemId);
}

void RmlModel_ProjectTracking::addNewTrackingMount(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	VRTrackingSystemDefinitionPtr vrSystemPtr = getSelectedVRTrackingSystem();
	if (vrSystemPtr)
	{
		vrSystemPtr->addTrackingMount();
	}
}

void RmlModel_ProjectTracking::removeTrackingMount(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.size() < 1)
		return;

	const int mountId = parameters[0].Get<int>();

	VRTrackingSystemDefinitionPtr vrSystemPtr = getSelectedVRTrackingSystem();
	if (vrSystemPtr)
	{
		vrSystemPtr->removeTrackingMount((MikanTrackingMountID)mountId);
	}
}

void RmlModel_ProjectTracking::selectTrackingSystemEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const MikanTrackingSystemID selectedSystemId = (MikanTrackingSystemID)parameters[0].Get<int>();
	setSelectedTrackingSystemId(selectedSystemId);
}

void RmlModel_ProjectTracking::selectTrackingMountEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const MikanTrackingMountID selectedMountId = (MikanTrackingMountID)parameters[0].Get<int>();
	setSelectedTrackingMountId(selectedMountId);
}

void RmlModel_ProjectTracking::setSelectedTrackingSystemId(MikanTrackingSystemID trackingSystemId)
{
	m_selectedTrackingSystemId = (int)trackingSystemId;
	m_modelHandle.DirtyVariable("selected_tracking_system_index");
}

void RmlModel_ProjectTracking::setSelectedTrackingMountId(MikanTrackingMountID trackingMountId)
{
	m_selectedTrackingMountId = (int)trackingMountId;
	m_modelHandle.DirtyVariable("selected_tracking_mount_index");
}