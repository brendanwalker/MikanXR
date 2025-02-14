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
	constructor.Bind("camera_vr_device_path", &m_cameraVRDevicePath);
	constructor.Bind("mat_vr_device_path", &m_matVRDevicePath);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"update_camera_tracker_device",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string vrDevicePath = ev.GetParameter<Rml::String>("value", "");
			if (OnUpdateCameraVRDevicePath) OnUpdateCameraVRDevicePath(vrDevicePath);
		});
	constructor.BindEventCallback(
		"update_mat_tracker_device",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string vrDevicePath = ev.GetParameter<Rml::String>("value", "");
			if (OnUpdateMatVRDevicePath) OnUpdateMatVRDevicePath(vrDevicePath);
		});

	// Fill in the data model
	rebuildVRDeviceList(vrDeviceManager);
	m_cameraVRDevicePath = profile->cameraVRDevicePath;
	m_matVRDevicePath = profile->matVRDevicePath;

	return true;
}

void RmlModel_VRDeviceSettings::dispose()
{
	OnUpdateCameraVRDevicePath.Clear();
	OnUpdateMatVRDevicePath.Clear();

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