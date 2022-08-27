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
#include <imgui.h>

//-- statics ----__
const char* AppStage_SpatialAnchors::APP_STAGE_NAME = "Spatial Anchor Setup";

//-- public methods -----
AppStage_SpatialAnchors::AppStage_SpatialAnchors(App* app)
	: AppStage(app, AppStage_SpatialAnchors::APP_STAGE_NAME)
	, m_scene(new GlScene)
	, m_camera(nullptr)
	, m_profile(nullptr)
	, m_selectedAnchorVRTrackerIndex(-1)
{ 
}

AppStage_SpatialAnchors::~AppStage_SpatialAnchors()
{
	delete m_scene;
}

void AppStage_SpatialAnchors::enter()
{
	App* app= App::getInstance();
	m_profile = app->getProfileConfig();

	buildVRTrackerList();

	for (auto it : m_vrTrackers)
	{
		it->getVRDeviceInterface()->bindToScene(m_scene);
	}
	//TODO: Bind events for device connect / disconnect

	m_camera = app->getRenderer()->pushCamera();
	m_camera->setCameraOrbitPitch(20.0f);
	m_camera->setCameraOrbitYaw(45.0f);
	m_camera->setCameraOrbitRadius(3.5f);
	m_camera->bindInput();
}

void AppStage_SpatialAnchors::exit()
{
	App::getInstance()->getRenderer()->popCamera();

	for (auto it : m_vrTrackers)
	{
		it->getVRDeviceInterface()->removeFromBoundScene();
	}
}

void AppStage_SpatialAnchors::update()
{
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

		m_selectedAnchorVRTrackerIndex =
			(it != m_vrTrackers.end())
			? (int)std::distance(m_vrTrackers.begin(), it)
			: -1;
	}

	if (m_selectedAnchorVRTrackerIndex == -1 && m_vrTrackers.size() > 0)
	{
		setSelectedAnchorVRTrackerIndex(0);
	}
}

void AppStage_SpatialAnchors::setSelectedAnchorVRTrackerIndex(int newIndex)
{
	if (newIndex > -1 && newIndex < m_vrTrackers.size())
	{
		m_selectedAnchorVRTrackerIndex= newIndex;
		m_profile->anchorVRDevicePath = m_vrTrackers[newIndex]->getDevicePath();
		m_profile->save();
	}
}

VRDeviceViewPtr AppStage_SpatialAnchors::getSelectedAnchorVRTracker() const
{
	return
		(m_selectedAnchorVRTrackerIndex != -1)
		? m_vrTrackers[m_selectedAnchorVRTrackerIndex]
		: nullptr;
}