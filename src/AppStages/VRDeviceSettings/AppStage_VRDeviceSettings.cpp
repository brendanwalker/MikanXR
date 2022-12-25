//-- inludes -----
#include "VRDeviceSettings/AppStage_VRDeviceSettings.h"
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "SpatialAnchors/AppStage_SpatialAnchors.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "MathUtility.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "VRDeviceView.h"
#include "VRDeviceManager.h"

#include <glm/gtc/matrix_transform.hpp>
//#include <imgui.h>

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

struct VRDeviceSettingsDataModel
{
	Rml::DataModelHandle model_handle;

	Rml::Vector<Rml::String> tracker_devices;
	int selected_camera_tracker_device = 0;
	int selected_mat_tracker_device = 0;
	Rml::String camera_vr_device_path;
	Rml::String mat_vr_device_path;
	Rml::Vector<Rml::String> spatial_anchors;
	int selected_camera_spatial_anchor = 0;
	float camera_scale= 1.f;
};

//-- statics ----__
const char* AppStage_VRDeviceSettings::APP_STAGE_NAME = "VRDeviceSettings";

//-- public methods -----
AppStage_VRDeviceSettings::AppStage_VRDeviceSettings(App* app)
	: AppStage(app, AppStage_VRDeviceSettings::APP_STAGE_NAME)
	, m_dataModel(new VRDeviceSettingsDataModel)
	, m_selectedCameraVRTrackerIndex(-1)
	, m_selectedMatVRTrackerIndex(-1)
{ }

AppStage_VRDeviceSettings::~AppStage_VRDeviceSettings()
{
	delete m_dataModel;
}

void AppStage_VRDeviceSettings::enter()
{
	AppStage::enter();
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	rebuildVRTrackerList();
	VRDeviceManager::getInstance()->OnDeviceListChanged +=
		MakeDelegate(this, &AppStage_VRDeviceSettings::rebuildVRTrackerList);

	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("vr_device_settings");
	if (!constructor)
		return;

	constructor.Bind("tracker_devices", &m_dataModel->tracker_devices);
	constructor.Bind("selected_camera_tracker_device", &m_dataModel->selected_camera_tracker_device);
	constructor.Bind("selected_mat_tracker_device", &m_dataModel->selected_mat_tracker_device);
	constructor.Bind("camera_vr_device_path", &m_dataModel->camera_vr_device_path);
	constructor.Bind("mat_vr_device_path", &m_dataModel->mat_vr_device_path);
	constructor.Bind("spatial_anchors", &m_dataModel->spatial_anchors);
	constructor.Bind("selected_camera_spatial_anchor", &m_dataModel->selected_camera_spatial_anchor);
	constructor.Bind("camera_scale", &m_dataModel->camera_scale);
	m_dataModel->model_handle = constructor.GetModelHandle();

	m_dataModel->tracker_devices.push_back("Select VR Tracker");
	for (VRDeviceViewPtr deviceView : m_vrTrackers)
	{
		const Rml::String friendlyName = deviceView->getTrackerRole() + " - " + deviceView->getSerialNumber();

		m_dataModel->tracker_devices.push_back(friendlyName);
	}

	m_dataModel->selected_camera_tracker_device= m_selectedCameraVRTrackerIndex + 1;
	m_dataModel->selected_mat_tracker_device= m_selectedMatVRTrackerIndex + 1;
	m_dataModel->camera_vr_device_path = profileConfig->cameraVRDevicePath;
	m_dataModel->mat_vr_device_path = profileConfig->matVRDevicePath;

	int anchorListIndex= 0;
	m_dataModel->spatial_anchors.push_back("NONE");
	for (const MikanSpatialAnchorInfo& anchorInfo : profileConfig->spatialAnchorList)
	{
		m_dataModel->spatial_anchors.push_back(anchorInfo.anchor_name);
		if (anchorInfo.anchor_id == profileConfig->cameraParentAnchorId)
		{
			m_dataModel->selected_camera_spatial_anchor= anchorListIndex + 1;
		}

		anchorListIndex++;
	}
	m_dataModel->selected_camera_spatial_anchor = anchorListIndex;
	

	addRmlDocument("rml\\vr_device_settings.rml");
}

void AppStage_VRDeviceSettings::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("vr_device_settings");

	VRDeviceManager::getInstance()->OnDeviceListChanged -=
		MakeDelegate(this, &AppStage_VRDeviceSettings::rebuildVRTrackerList);

	AppStage::exit();
}

void AppStage_VRDeviceSettings::update()
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	bool bCameraSettingsChanged= false;
	bool bMatSettingsChanged= false;

	if (m_dataModel->model_handle.IsVariableDirty("selected_camera_tracker_device"))
	{
		setSelectedCameraVRTrackerIndex(m_dataModel->selected_camera_tracker_device - 1);
		VRDeviceViewPtr deviceView= getSelectedCameraVRTracker();
		profileConfig->cameraVRDevicePath = deviceView ? getSelectedCameraVRTracker()->getDevicePath() : "";
		bCameraSettingsChanged= true;
	}

	if (m_dataModel->model_handle.IsVariableDirty("selected_camera_spatial_anchor"))
	{
		int anchorListIndex= m_dataModel->selected_camera_spatial_anchor - 1;

		if (anchorListIndex >= 0)
		{
			profileConfig->cameraParentAnchorId = profileConfig->spatialAnchorList[anchorListIndex].anchor_id;
		}
		else
		{
			profileConfig->cameraParentAnchorId = INVALID_MIKAN_ID;
		}
		bCameraSettingsChanged= true;
	}

	if (m_dataModel->model_handle.IsVariableDirty("camera_scale"))
	{
		profileConfig->cameraScale= m_dataModel->camera_scale;
		bCameraSettingsChanged= true;
	}

	if (m_dataModel->model_handle.IsVariableDirty("selected_mat_tracker_device"))
	{
		setSelectedMatVRTrackerIndex(m_dataModel->selected_mat_tracker_device - 1);
		VRDeviceViewPtr deviceView= getSelectedMatVRTracker();
		profileConfig->matVRDevicePath = deviceView ? deviceView->getDevicePath() : "";
		bMatSettingsChanged= true;
	}

	if (bCameraSettingsChanged || bMatSettingsChanged)
	{
		// Save out config changes
		profileConfig->save();

		if (bCameraSettingsChanged)
		{
			// Let any connected clients know that the video source attachment settings changed
			MikanServer::getInstance()->publishVideoSourceAttachmentChangedEvent();
		}
	}
}

void AppStage_VRDeviceSettings::onRmlClickEvent(const std::string& value)
{
	if (value == "calibrate_vr_camera_alignment")
	{
		m_app->pushAppStage<AppStage_AlignmentCalibration>();
	}
	else if (value == "test_vr_camera_alignment")
	{
		m_app->pushAppStage<AppStage_AlignmentCalibration>()->setBypassCalibrationFlag(true);
	}
	else if (value == "goto_spatial_anchor_setup")
	{
		m_app->pushAppStage<AppStage_SpatialAnchors>();
	}
	else if (value == "goto_main_menu")
	{
		m_app->popAppState();
	}
}

void AppStage_VRDeviceSettings::setSelectedCameraVRTrackerIndex(int index)
{
	m_selectedCameraVRTrackerIndex =
		(index > -1 && index < m_vrTrackers.size())
		? index
		: m_selectedCameraVRTrackerIndex;
}

void AppStage_VRDeviceSettings::setSelectedMatVRTrackerIndex(int index)
{
	m_selectedMatVRTrackerIndex =
		(index > -1 && index < m_vrTrackers.size())
		? index
		: m_selectedMatVRTrackerIndex;
}

VRDeviceViewPtr AppStage_VRDeviceSettings::getSelectedCameraVRTracker() const
{
	return
		(m_selectedCameraVRTrackerIndex != -1)
		? m_vrTrackers[m_selectedCameraVRTrackerIndex]
		: nullptr;
}

VRDeviceViewPtr AppStage_VRDeviceSettings::getSelectedMatVRTracker() const
{
	return
		(m_selectedMatVRTrackerIndex != -1)
		? m_vrTrackers[m_selectedMatVRTrackerIndex]
		: nullptr;
}

int AppStage_VRDeviceSettings::getVRTrackerCount() const
{
	return (int)m_vrTrackers.size();
}

void AppStage_VRDeviceSettings::rebuildVRTrackerList()
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();
	bool bIsConfigDirty= false;

	m_vrTrackers = m_app->getVRDeviceManager()->getFilteredVRDeviceList(eDeviceType::VRTracker);

	// Find the index of the currently selected mat vr device
	{
		std::string matDevicePath = profileConfig->matVRDevicePath;
		auto it = std::find_if(m_vrTrackers.begin(), m_vrTrackers.end(),
			[matDevicePath](const VRDeviceViewPtr& view)
			{
				return view->getDevicePath() == matDevicePath;
			});

		m_selectedMatVRTrackerIndex =
			(it != m_vrTrackers.end())
			? (int)std::distance(m_vrTrackers.begin(), it)
			: -1;
	}

	// Find the index of the currently selected camera vr device
	{
		std::string cameraDevicePath = profileConfig->cameraVRDevicePath;
		auto it = std::find_if(m_vrTrackers.begin(), m_vrTrackers.end(),
			[cameraDevicePath](const VRDeviceViewPtr& view)
			{
				return view->getDevicePath() == cameraDevicePath;
			});

		m_selectedCameraVRTrackerIndex =
			(it != m_vrTrackers.end())
			? (int)std::distance(m_vrTrackers.begin(), it)
			: -1;
	}

	if (m_selectedCameraVRTrackerIndex == -1 && m_vrTrackers.size() > 0)
	{
		m_selectedCameraVRTrackerIndex = 0;

		if (profileConfig->cameraVRDevicePath == "")
		{
			profileConfig->cameraVRDevicePath = m_vrTrackers[0]->getDevicePath();
			bIsConfigDirty= true;
		}
	}

	if (m_selectedMatVRTrackerIndex == -1 && m_vrTrackers.size() > 0)
	{
		m_selectedMatVRTrackerIndex = 0;

		if (profileConfig->matVRDevicePath == "")
		{
			profileConfig->matVRDevicePath = m_vrTrackers[0]->getDevicePath();
			bIsConfigDirty = true;
		}
	}

	if (bIsConfigDirty)
	{
		profileConfig->save();
	}
}