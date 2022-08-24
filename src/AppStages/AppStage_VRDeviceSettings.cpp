//-- inludes -----
#include "AppStage_VRDeviceSettings.h"
#include "AppStage_AlignmentCalibration.h"
#include "AppStage_SpatialAnchors.h"
#include "AppStage_MainMenu.h"
#include "App.h"
#include "MathUtility.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "VRDeviceView.h"
#include "VRDeviceManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

//-- statics ----__
const char* AppStage_VRDeviceSettings::APP_STAGE_NAME = "CameraSettings";

//-- public methods -----
AppStage_VRDeviceSettings::AppStage_VRDeviceSettings(App* app)
	: AppStage(app, AppStage_VRDeviceSettings::APP_STAGE_NAME)
	, m_selectedCameraVRTrackerIndex(-1)
	, m_selectedMatVRTrackerIndex(-1)
{ }

void AppStage_VRDeviceSettings::enter()
{
	rebuildVRTrackerList();
	VRDeviceManager::getInstance()->OnDeviceListChanged +=
		MakeDelegate(this, &AppStage_VRDeviceSettings::rebuildVRTrackerList);
}

void AppStage_VRDeviceSettings::exit()
{
	VRDeviceManager::getInstance()->OnDeviceListChanged -=
		MakeDelegate(this, &AppStage_VRDeviceSettings::rebuildVRTrackerList);
}

void AppStage_VRDeviceSettings::update()
{
}

void AppStage_VRDeviceSettings::render()
{
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

void AppStage_VRDeviceSettings::renderUI()
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	const char* k_window_title = "VR Device Settings";
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowSize(ImVec2(350, 400));
	ImGui::Begin(k_window_title, nullptr, window_flags);

	ImGui::Text("Camera VR Device");
	if (m_selectedCameraVRTrackerIndex != -1)
	{
		int oldSelectedIndex= m_selectedCameraVRTrackerIndex;

		if (ImGui::Button("<##VRCameraDevice"))
		{
			m_selectedCameraVRTrackerIndex= int_max(m_selectedCameraVRTrackerIndex - 1, 0);
		}
		ImGui::SameLine();
		{
			VRDeviceViewPtr deviceView = getSelectedCameraVRTracker();
			ImGui::Text("%s - %s (%d/%d)",
				deviceView->getTrackerRole().c_str(), 
				deviceView->getSerialNumber().c_str(),
				m_selectedCameraVRTrackerIndex + 1, 
				(int)m_vrTrackers.size());
		}
		ImGui::SameLine();
		if (ImGui::Button(">##VRCameraDevice"))
		{
			m_selectedCameraVRTrackerIndex = 
				int_min(m_selectedCameraVRTrackerIndex + 1, (int)m_vrTrackers.size() - 1);
		}

		if (oldSelectedIndex != m_selectedCameraVRTrackerIndex)
		{
			profileConfig->cameraVRDevicePath = getSelectedCameraVRTracker()->getDevicePath();
			profileConfig->save();
		}
	}
	else
	{
		ImGui::Text("%s", profileConfig->cameraVRDevicePath.c_str());
	}

	ImGui::Separator();

	ImGui::Text("Mat VR Device");
	if (m_selectedMatVRTrackerIndex != -1)
	{
		int oldSelectedIndex = m_selectedMatVRTrackerIndex;

		if (ImGui::Button("<##VRMatDevice"))
		{
			m_selectedMatVRTrackerIndex = int_max(m_selectedMatVRTrackerIndex - 1, 0);
		}
		ImGui::SameLine();
		{
			VRDeviceViewPtr deviceView = getSelectedMatVRTracker();
			ImGui::Text("%s - %s (%d/%d)",
				deviceView->getTrackerRole().c_str(),
				deviceView->getSerialNumber().c_str(),
				m_selectedMatVRTrackerIndex + 1,
				(int)m_vrTrackers.size());
		}
		ImGui::SameLine();
		if (ImGui::Button(">##VRMatDevice"))
		{
			m_selectedMatVRTrackerIndex =
				int_min(m_selectedMatVRTrackerIndex + 1, (int)m_vrTrackers.size() - 1);
		}

		if (oldSelectedIndex != m_selectedMatVRTrackerIndex)
		{
			profileConfig->matVRDevicePath = getSelectedMatVRTracker()->getDevicePath();
			profileConfig->save();
		}
	}
	else
	{
		ImGui::Text("%s", profileConfig->matVRDevicePath.c_str());
	}
	
	ImGui::Separator();

	if (m_vrTrackers.size() >= 2 && 
		m_selectedCameraVRTrackerIndex != m_selectedMatVRTrackerIndex)
	{
		if (ImGui::Button("Calibrate VR/Camera Alignment"))
		{
			m_app->pushAppStage<AppStage_AlignmentCalibration>();
		}
		if (ImGui::Button("Test VR/Camera Alignment"))
		{
			m_app->pushAppStage<AppStage_AlignmentCalibration>()->setBypassCalibrationFlag(true);
		}
	}

	if (ImGui::Button(locTextUTF8("", "spatial_anchor_setup")))
	{
		m_app->pushAppStage<AppStage_SpatialAnchors>();
	}

	ImGui::Separator();

	if (ImGui::Button("Return to Main Menu"))
	{
		m_app->popAppState();
	}

	ImGui::End();
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