//-- inludes -----
#include "VRDeviceSettings/AppStage_VRDeviceSettings.h"
#include "VRDeviceSettings/RmlModel_VRDeviceSettings.h"
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "SpatialAnchors/AppStage_SpatialAnchors.h"
#include "VRTrackingRecenter/AppStage_VRTrackingRecenter.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "VRDeviceManager.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----
const char* AppStage_VRDeviceSettings::APP_STAGE_NAME = "VRDeviceSettings";

//-- public methods -----
AppStage_VRDeviceSettings::AppStage_VRDeviceSettings(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_VRDeviceSettings::APP_STAGE_NAME)
	, m_vrDeviceSettingsModel(new RmlModel_VRDeviceSettings)
{ }

AppStage_VRDeviceSettings::~AppStage_VRDeviceSettings()
{
	delete m_vrDeviceSettingsModel;
}

void AppStage_VRDeviceSettings::enter()
{
	AppStage::enter();

	// Start listening for tracker device changes
	VRDeviceManager* vrDeviceManager= VRDeviceManager::getInstance();
	vrDeviceManager->OnDeviceListChanged += MakeDelegate(this, &AppStage_VRDeviceSettings::rebuildVRTrackerList);

	// Init the UI
	{
		ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();
		Rml::Context* context = getRmlContext();

		// Init the vr device settings model
		m_vrDeviceSettingsModel->init(context, profileConfig, vrDeviceManager);
		m_vrDeviceSettingsModel->OnUpdateCameraVRDevicePath= MakeDelegate(this, &AppStage_VRDeviceSettings::onUpdateCameraVRDevicePath);
		m_vrDeviceSettingsModel->OnUpdateMatVRDevicePath= MakeDelegate(this, &AppStage_VRDeviceSettings::onUpdateMatVRDevicePath);

		// Init vr device settings view now that the dependent model has been created
		m_vrDeviceSettingsView = addRmlDocument("vr_device_settings.rml");
	}
}

void AppStage_VRDeviceSettings::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("vr_device_settings");

	// Stop listening for tracker device changes
	VRDeviceManager::getInstance()->OnDeviceListChanged -=
		MakeDelegate(this, &AppStage_VRDeviceSettings::rebuildVRTrackerList);

	AppStage::exit();
}

// VR Device Setting Model UI Events
void AppStage_VRDeviceSettings::onUpdateCameraVRDevicePath(const std::string& devicePath)
{
	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	profileConfig->cameraVRDevicePath= devicePath;
	profileConfig->markDirty(
		ConfigPropertyChangeSet()
		.addPropertyName(ProfileConfig::k_cameraVRDevicePathPropertyId));

	// Let any connected clients know that the video source attachment settings changed
	MikanServer::getInstance()->publishVideoSourceAttachmentChangedEvent();
}

void AppStage_VRDeviceSettings::onUpdateMatVRDevicePath(const std::string& devicePath)
{
	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	profileConfig->matVRDevicePath = devicePath;
	profileConfig->markDirty(
		ConfigPropertyChangeSet()
		.addPropertyName(ProfileConfig::k_matVRDevicePathPropertyId));

	// Let any connected clients know that the video source attachment settings changed
	MikanServer::getInstance()->publishVideoSourceAttachmentChangedEvent();
}

void AppStage_VRDeviceSettings::onRmlClickEvent(const std::string& value)
{
	if (value == "goto_vr_tracking_recenter")
	{
		m_ownerWindow->pushAppStage<AppStage_VRTrackingRecenter>();
	}
	else if (value == "test_vr_camera_alignment")
	{
		m_ownerWindow->pushAppStage<AppStage_AlignmentCalibration>()->setBypassCalibrationFlag(true);
	}
	else if (value == "calibrate_vr_camera_alignment")
	{
		m_ownerWindow->pushAppStage<AppStage_AlignmentCalibration>();
	}
	else if (value == "goto_spatial_anchor_setup")
	{
		m_ownerWindow->pushAppStage<AppStage_SpatialAnchors>();
	}
	else if (value == "goto_main_menu")
	{
		m_ownerWindow->popAppState();
	}
}

void AppStage_VRDeviceSettings::rebuildVRTrackerList()
{
	m_vrDeviceSettingsModel->rebuildVRDeviceList(VRDeviceManager::getInstance());
}
