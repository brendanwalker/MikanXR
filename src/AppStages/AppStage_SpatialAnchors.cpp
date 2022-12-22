//-- inludes -----
#include "AppStage_SpatialAnchors.h"
#include "AppStage_MainMenu.h"
#include "App.h"
#include "Colors.h"
#include "GlCamera.h"
#include "GlScene.h"
#include "Renderer.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "StringUtils.h"
#include "VRDeviceView.h"
#include "VRDeviceManager.h"

#include <glm/gtc/matrix_transform.hpp>

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

struct SpatialAnchorSetupDataModel
{
	Rml::DataModelHandle model_handle;

	Rml::Vector<Rml::String> tracker_devices;
	int selected_tracker_device = 0;
	Rml::String anchor_vr_device_path;
	Rml::Vector<Rml::String> spatial_anchors;
	int selected_spatial_anchor = 0;
	int max_spatial_anchors = MAX_MIKAN_SPATIAL_ANCHORS;
};

//-- statics ----__
const char* AppStage_SpatialAnchors::APP_STAGE_NAME = "Spatial Anchor Setup";

//-- public methods -----
AppStage_SpatialAnchors::AppStage_SpatialAnchors(App* app)
	: AppStage(app, AppStage_SpatialAnchors::APP_STAGE_NAME)
	, m_dataModel(new SpatialAnchorSetupDataModel)
	, m_scene(new GlScene)
	, m_camera(nullptr)
	, m_profile(nullptr)
{ 
}

AppStage_SpatialAnchors::~AppStage_SpatialAnchors()
{
	delete m_dataModel;
	delete m_scene;
}

void AppStage_SpatialAnchors::enter()
{
	AppStage::enter();

	App* app= App::getInstance();
	m_profile = app->getProfileConfig();

	buildVRTrackerList();

	for (auto it : m_vrTrackers)
	{
		it->getVRDeviceInterface()->bindToScene(m_scene);
	}

	m_camera = app->getRenderer()->pushCamera();
	m_camera->setCameraOrbitPitch(20.0f);
	m_camera->setCameraOrbitYaw(45.0f);
	m_camera->setCameraOrbitRadius(3.5f);
	m_camera->bindInput();

	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("spatial_anchor_settings");
	if (!constructor)
		return;

	constructor.Bind("tracker_devices", &m_dataModel->tracker_devices);
	constructor.Bind("selected_tracker_device", &m_dataModel->selected_tracker_device);
	constructor.Bind("anchor_vr_device_path", &m_dataModel->anchor_vr_device_path);
	constructor.Bind("spatial_anchors", &m_dataModel->spatial_anchors);
	constructor.Bind("selected_spatial_anchor", &m_dataModel->selected_spatial_anchor);
	constructor.Bind("max_spatial_anchors", &m_dataModel->max_spatial_anchors);
	constructor.BindEventCallback(
		"add_new_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			VRDeviceViewPtr anchorVRDevice = getSelectedAnchorVRTracker();
			const glm::mat4 anchorXform = anchorVRDevice->getCalibrationPose();
			MikanMatrix4f mikanXform = glm_mat4_to_MikanMatrix4f(anchorXform);

			char newAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
			StringUtils::formatString(newAnchorName, sizeof(newAnchorName), "Anchor %d", m_profile->nextAnchorId);

			if (m_profile->addNewAnchor("New Anchor", mikanXform))
			{
				m_dataModel->model_handle.DirtyVariable("spatial_anchors");

				// Tell any connected clients that the anchor list changed
				MikanServer::getInstance()->publishAnchorListChangedEvent();
			}
		});
	constructor.BindEventCallback(
		"update_anchor_pose", 
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			const int anchorIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (anchorIndex >= 0 && anchorIndex < (int)m_dataModel->spatial_anchors.size())
			{
				MikanSpatialAnchorInfo& anchor= m_profile->spatialAnchorList[anchorIndex];
				VRDeviceViewPtr anchorVRDevice = getSelectedAnchorVRTracker();
				if (anchorVRDevice != nullptr)
				{
					const glm::mat4 anchorXform = anchorVRDevice->getCalibrationPose();
					anchor.anchor_xform = glm_mat4_to_MikanMatrix4f(anchorXform);
					m_profile->updateAnchor(anchor);

					// Tell any connected clients that the anchor pose changed
					{
						MikanAnchorPoseUpdateEvent poseUpdateEvent;
						memset(&poseUpdateEvent, 0, sizeof(MikanAnchorPoseUpdateEvent));
						poseUpdateEvent.anchor_id = anchor.anchor_id;
						poseUpdateEvent.transform = anchor.anchor_xform;

						MikanServer::getInstance()->publishAnchorPoseUpdatedEvent(poseUpdateEvent);
					}
				}
			}
		});
	constructor.BindEventCallback(
		"update_anchor_name",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) 
		{
			const int anchorIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (anchorIndex >= 0 && anchorIndex < (int)m_dataModel->spatial_anchors.size())
			{
				const Rml::String stringValue = ev.GetParameter("value", Rml::String());
				MikanSpatialAnchorInfo& anchor = m_profile->spatialAnchorList[anchorIndex];
				StringUtils::formatString(anchor.anchor_name, sizeof(anchor.anchor_name), "%s", stringValue.c_str());

				m_profile->save();

				// Tell any connected clients that the anchor list changed
				MikanServer::getInstance()->publishAnchorListChangedEvent();
			}
		});
	constructor.BindEventCallback(
		"erase_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			const int anchorIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1);
			if (anchorIndex >= 0 && anchorIndex < (int)m_dataModel->spatial_anchors.size())
			{
				const MikanSpatialAnchorInfo& anchor = m_profile->spatialAnchorList[anchorIndex];

				m_profile->removeAnchor(anchor.anchor_id);

				// Tell any connected clients that the anchor list changed
				MikanServer::getInstance()->publishAnchorListChangedEvent();
			}
		});
	constructor.BindEventCallback(
		"goto_main_menu",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			m_app->popAppState();
		});
	m_dataModel->model_handle = constructor.GetModelHandle();

	m_dataModel->tracker_devices.push_back("Select VR Tracker");
	for (VRDeviceViewPtr deviceView : m_vrTrackers)
	{
		const Rml::String friendlyName = deviceView->getTrackerRole() + " - " + deviceView->getSerialNumber();

		m_dataModel->tracker_devices.push_back(friendlyName);
	}

	int anchorListIndex = 0;
	for (const MikanSpatialAnchorInfo& anchorInfo : m_profile->spatialAnchorList)
	{
		m_dataModel->spatial_anchors.push_back(anchorInfo.anchor_name);
		if (anchorInfo.anchor_id == m_profile->cameraParentAnchorId)
		{
			m_dataModel->selected_spatial_anchor = anchorListIndex + 1;
		}

		anchorListIndex++;
	}
	m_dataModel->selected_spatial_anchor = anchorListIndex;
	m_dataModel->anchor_vr_device_path = m_profile->anchorVRDevicePath;

	pushRmlDocument("rml\\spatial_anchor_setup.rml");
}

void AppStage_SpatialAnchors::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("spatial_anchor_settings");

	App::getInstance()->getRenderer()->popCamera();

	for (auto it : m_vrTrackers)
	{
		it->getVRDeviceInterface()->removeFromBoundScene();
	}

	AppStage::exit();
}

void AppStage_SpatialAnchors::update()
{
	if (m_dataModel->model_handle.IsVariableDirty("selected_spatial_anchor"))
	{
		int newSelectedIndex = m_dataModel->selected_spatial_anchor - 1;

		setSelectedAnchorVRTrackerIndex(newSelectedIndex);
	}
}

void AppStage_SpatialAnchors::render()
{
	TextStyle style = getDefaultTextStyle();

	m_scene->render();

	drawGrid(glm::mat4(1.f), 10.f, 10.f, 20, 20, Colors::GhostWhite);
	drawTransformedAxes(glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.001f, 0.f)), 0.5f);

	// Highlight the VR device we are using for anchor placement
	VRDeviceViewPtr anchorVRDevice = getSelectedAnchorVRTracker();
	if (anchorVRDevice != nullptr)
	{
		const glm::mat4 anchorXform= anchorVRDevice->getCalibrationPose();
		const glm::vec3 anchorPos= glm::vec3(anchorXform[3]);

		drawTransformedAxes(anchorXform, 0.2f);
		drawTransformedBox(anchorXform, glm::vec3(-0.1f, -0.1f, -0.1f), glm::vec3(0.1f, 0.1f, 0.1f), Colors::Yellow);
		drawTextAtWorldPosition(style, anchorPos, L"Anchor VR Device");
	}

	// Draw the anchors
	for (const MikanSpatialAnchorInfo& anchor : m_profile->spatialAnchorList)
	{
		wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
		StringUtils::convertMbsToWcs(anchor.anchor_name, wszAnchorName, sizeof(wszAnchorName));
		glm::mat4 anchorXform = MikanMatrix4f_to_glm_mat4(anchor.anchor_xform);
		glm::vec3 anchorPos(anchorXform[3]);

		drawTransformedAxes(anchorXform, 0.1f);
		drawTextAtWorldPosition(style, anchorPos, wszAnchorName);
	}
}

#if 0
void AppStage_SpatialAnchors::renderUI()
{
	const char* k_window_title = locTextUTF8("", "spatial_anchor_setup");
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse;

	const ImVec2 displaySize = ImGui::GetMainViewport()->Size;
	const ImVec2 panelSize = ImVec2(500, 220);

	ImGui::SetNextWindowPos(ImVec2(10, displaySize.y - 10), 0, ImVec2(0.f, 1.0f));
	ImGui::SetNextWindowSize(panelSize);
	ImGui::Begin(k_window_title, nullptr, window_flags);

	ImGui::Text("Anchor VR Device");
	if (m_selectedAnchorVRTrackerIndex != -1)
	{
		int newSelectedIndex = m_selectedAnchorVRTrackerIndex;

		if (ImGui::Button("<##VRAnchorDevice"))
		{
			newSelectedIndex = int_max(m_selectedAnchorVRTrackerIndex - 1, 0);
		}
		ImGui::SameLine();
		{
			VRDeviceViewPtr deviceView = getSelectedAnchorVRTracker();
			ImGui::Text("%s - %s (%d/%d)",
				deviceView->getTrackerRole().c_str(),
				deviceView->getSerialNumber().c_str(),
				newSelectedIndex + 1,
				(int)m_vrTrackers.size());
		}
		ImGui::SameLine();
		if (ImGui::Button(">##VRAnchorDevice"))
		{
			newSelectedIndex = int_min(newSelectedIndex + 1, (int)m_vrTrackers.size() - 1);
		}

		if (newSelectedIndex != m_selectedAnchorVRTrackerIndex)
		{
			setSelectedAnchorVRTrackerIndex(newSelectedIndex);
		}
	}
	else
	{
		ImGui::Text("%s", m_profile->anchorVRDevicePath.c_str());
	}
	VRDeviceViewPtr anchorVRDevice= getSelectedAnchorVRTracker();

	ImGui::Text("Anchors");
	bool bAnchorListChanged= false;
	MikanSpatialAnchorID anchorIdToDelete= INVALID_MIKAN_ID;
	for (auto it= m_profile->spatialAnchorList.begin(); it != m_profile->spatialAnchorList.end(); ++it)
	{
		MikanSpatialAnchorInfo anchor= *it;

		ImGui::PushID(anchor.anchor_id);

		if (anchorVRDevice != nullptr)
		{
			if (ImGui::Button("Set Pose"))
			{
				const glm::mat4 anchorXform = anchorVRDevice->getCalibrationPose();
				anchor.anchor_xform= glm_mat4_to_MikanMatrix4f(anchorXform);
				m_profile->updateAnchor(anchor);

				// Tell any connected clients that the anchor pose changed
				{
					MikanAnchorPoseUpdateEvent poseUpdateEvent;
					memset(&poseUpdateEvent, 0, sizeof(MikanAnchorPoseUpdateEvent));
					poseUpdateEvent.anchor_id= anchor.anchor_id;
					poseUpdateEvent.transform= anchor.anchor_xform;

					MikanServer::getInstance()->publishAnchorPoseUpdatedEvent(poseUpdateEvent);
				}
			}
		}
		else
		{
			ImGui::TextDisabled("Set Pose");
		}
		ImGui::SameLine();
		if (ImGui::InputText("Name", anchor.anchor_name, sizeof(anchor.anchor_name) - 1, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			m_profile->updateAnchor(anchor);
		}
		ImGui::SameLine();
		if (ImGui::Button("x"))
		{
			anchorIdToDelete = anchor.anchor_id;
		}

		ImGui::PopID();
	}		
	if (anchorIdToDelete != INVALID_MIKAN_ID)
	{
		m_profile->removeAnchor(anchorIdToDelete);
		bAnchorListChanged= true;
	}

	if (m_profile->canAddAnchor() && anchorVRDevice != nullptr)
	{
		if (ImGui::Button("New Anchor"))
		{
			const glm::mat4 anchorXform = anchorVRDevice->getCalibrationPose();
			MikanMatrix4f mikanXform = glm_mat4_to_MikanMatrix4f(anchorXform);

			char newAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
			StringUtils::formatString(newAnchorName, sizeof(newAnchorName), "Anchor %d", m_profile->nextAnchorId);

			m_profile->addNewAnchor("New Anchor", mikanXform);
			bAnchorListChanged = true;
		}
	}
	else
	{
		ImGui::TextDisabled("New Anchor");
	}

	// Tell any connected clients that the anchor list changed
	if (bAnchorListChanged)
	{
		MikanServer::getInstance()->publishAnchorListChangedEvent();
	}

	if (ImGui::Button("Return to Main Menu"))
	{
		m_app->popAppState();
	}

	ImGui::End();
}
#endif

void AppStage_SpatialAnchors::buildVRTrackerList()
{
	m_vrTrackers = m_app->getVRDeviceManager()->getFilteredVRDeviceList(eDeviceType::VRTracker);

	// Find the index of the currently selected camera vr device
	{
		std::string cameraDevicePath = m_profile->cameraVRDevicePath;
		auto it = std::find_if(m_vrTrackers.begin(), m_vrTrackers.end(),
			[cameraDevicePath](const VRDeviceViewPtr& view)
		{
			return view->getDevicePath() == cameraDevicePath;
		});

		m_dataModel->selected_tracker_device =
			(it != m_vrTrackers.end())
			? (int)std::distance(m_vrTrackers.begin(), it)
			: -1;
	}

	if (m_dataModel->selected_tracker_device == -1 && m_vrTrackers.size() > 0)
	{
		setSelectedAnchorVRTrackerIndex(0);
	}
}

void AppStage_SpatialAnchors::setSelectedAnchorVRTrackerIndex(int newIndex)
{
	if (newIndex > -1 && newIndex < m_vrTrackers.size())
	{
		m_dataModel->selected_tracker_device= newIndex;
		m_profile->anchorVRDevicePath = m_vrTrackers[newIndex]->getDevicePath();
		m_profile->save();
	}
}

VRDeviceViewPtr AppStage_SpatialAnchors::getSelectedAnchorVRTracker() const
{
	return
		(m_dataModel->selected_tracker_device > -1 && m_dataModel->selected_tracker_device < m_vrTrackers.size())
		? m_vrTrackers[m_dataModel->selected_tracker_device]
		: nullptr;
}