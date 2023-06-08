#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "RmlModel_VRDeviceSettings.h"
#include "MathMikan.h"
#include "ProfileConfig.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_VRDeviceSettings::init(
	Rml::Context* rmlContext,
	const ProfileConfigConstPtr profile,
	const VRDeviceManager* vrDeviceManager)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "vr_device_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("tracker_devices", &m_vrDeviceList);
	constructor.Bind("spatial_anchors", &m_spatialAnchors);
	constructor.Bind("camera_vr_device_path", &m_cameraVRDevicePath);
	constructor.Bind("camera_parent_anchor_id", &m_cameraParentAnchorId);	
	constructor.Bind("camera_scale", &m_cameraScale);
	constructor.Bind("mat_vr_device_path", &m_matVRDevicePath);
	constructor.Bind("origin_vr_device_path", &m_originVRDevicePath);
	constructor.Bind("origin_vertical_align_flag", &m_originVerticalAlignFlag);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"update_camera_tracker_device",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string vrDevicePath = ev.GetParameter<Rml::String>("value", "");
			if (OnUpdateCameraVRDevicePath) OnUpdateCameraVRDevicePath(vrDevicePath);
		});
	constructor.BindEventCallback(
		"update_camera_parent_anchor_id",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int anchorId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);
			if (OnUpdateCameraParentAnchorId) OnUpdateCameraParentAnchorId(anchorId);
		});
	constructor.BindEventCallback(
		"update_camera_scale",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const float newScale = ev.GetParameter<float>("value", 1.f);
			if (OnUpdateCameraScale) OnUpdateCameraScale(newScale);
		});
	constructor.BindEventCallback(
		"update_mat_tracker_device",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string vrDevicePath = ev.GetParameter<Rml::String>("value", "");
			if (OnUpdateMatVRDevicePath) OnUpdateMatVRDevicePath(vrDevicePath);
		});
	constructor.BindEventCallback(
		"update_origin_tracker_device",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string vrDevicePath = ev.GetParameter<Rml::String>("value", "");
			if (OnUpdateOriginVRDevicePath) OnUpdateOriginVRDevicePath(vrDevicePath);
		});
	constructor.BindEventCallback(
		"update_vertical_align_flag",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnUpdateOriginVerticalAlignFlag)
			{
				const std::string value = ev.GetParameter<Rml::String>("value", "");
				const bool bIsChecked = !value.empty();

				OnUpdateOriginVerticalAlignFlag(bIsChecked);
			}
		});

	// Fill in the data model
	rebuildVRDeviceList(vrDeviceManager);
	rebuildAnchorList(profile);
	m_cameraVRDevicePath = profile->cameraVRDevicePath;
	m_cameraParentAnchorId = profile->cameraParentAnchorId;
	m_cameraScale = profile->cameraScale;
	m_matVRDevicePath = profile->matVRDevicePath;
	m_originVRDevicePath = profile->originVRDevicePath;
	m_originVerticalAlignFlag = profile->originVerticalAlignFlag;

	return true;
}

void RmlModel_VRDeviceSettings::dispose()
{
	OnUpdateCameraVRDevicePath.Clear();
	OnUpdateCameraParentAnchorId.Clear();
	OnUpdateMatVRDevicePath.Clear();
	OnUpdateOriginVRDevicePath.Clear();
	OnUpdateCameraScale.Clear();

	RmlModel::dispose();
}

void RmlModel_VRDeviceSettings::rebuildVRDeviceList(const VRDeviceManager* vrDeviceManager)
{
	VRDeviceList vrTrackers = vrDeviceManager->getFilteredVRDeviceList(eDeviceType::VRTracker);

	m_vrDeviceList.clear();
	for (VRDeviceViewPtr vrTrackerPtr : vrTrackers)
	{
		m_vrDeviceList.push_back(vrTrackerPtr->getDevicePath());
	}
	m_modelHandle.DirtyVariable("tracker_devices");
}

void RmlModel_VRDeviceSettings::rebuildAnchorList(ProfileConfigConstPtr profile)
{
	m_spatialAnchors.clear();
	for (AnchorDefinitionPtr anchorInfo : profile->anchorConfig->spatialAnchorList)
	{
		m_spatialAnchors.push_back(anchorInfo->getAnchorId());
	}
	m_modelHandle.DirtyVariable("spatial_anchors");
}