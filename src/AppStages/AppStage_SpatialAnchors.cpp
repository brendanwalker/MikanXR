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
	Rml::Vector<Rml::String> spatial_anchors;
	int max_spatial_anchors = MAX_MIKAN_SPATIAL_ANCHORS;

	void rebuildSpatialAnchors(ProfileConfig *profile)
	{
		spatial_anchors.clear();
		for (const MikanSpatialAnchorInfo& anchorInfo : profile->spatialAnchorList)
		{
			spatial_anchors.push_back(anchorInfo.anchor_name);
		}
		model_handle.DirtyVariable("spatial_anchors");
	}
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

	// Build the list of VR trackers
	m_vrTrackers = m_app->getVRDeviceManager()->getFilteredVRDeviceList(eDeviceType::VRTracker);

	// Bind tracker to 3D scene
	for (auto it : m_vrTrackers)
	{
		it->getVRDeviceInterface()->bindToScene(m_scene);
	}

	// Setup orbit camera
	m_camera = app->getRenderer()->pushCamera();
	m_camera->setCameraOrbitPitch(20.0f);
	m_camera->setCameraOrbitYaw(45.0f);
	m_camera->setCameraOrbitRadius(3.5f);
	m_camera->bindInput();

	// Create Datamodel
	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("spatial_anchor_settings");
	if (!constructor)
		return;

	// Register Data Model Fields
	constructor.Bind("tracker_devices", &m_dataModel->tracker_devices);
	constructor.Bind("selected_tracker_device", &m_dataModel->selected_tracker_device);
	constructor.Bind("anchor_vr_device_path", &m_profile->anchorVRDevicePath);
	constructor.Bind("spatial_anchors", &m_dataModel->spatial_anchors);
	constructor.Bind("max_spatial_anchors", &m_dataModel->max_spatial_anchors);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"add_new_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			VRDeviceViewPtr anchorVRDevice = getSelectedAnchorVRTracker();
			const glm::mat4 anchorXform = (anchorVRDevice) ? anchorVRDevice->getCalibrationPose() : glm::mat4(1.f);
			MikanMatrix4f mikanXform = glm_mat4_to_MikanMatrix4f(anchorXform);

			char newAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
			StringUtils::formatString(newAnchorName, sizeof(newAnchorName), "Anchor %d", m_profile->nextAnchorId);

			if (m_profile->addNewAnchor("New Anchor", mikanXform))
			{
				m_dataModel->rebuildSpatialAnchors(m_profile);

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
				const bool isLineBreak = ev.GetParameter("linebreak", false);

				if (isLineBreak)
				{
					const Rml::String stringValue = ev.GetParameter("value", Rml::String());

					MikanSpatialAnchorInfo& anchor = m_profile->spatialAnchorList[anchorIndex];
					StringUtils::formatString(anchor.anchor_name, sizeof(anchor.anchor_name), "%s", stringValue.c_str());

					m_profile->save();

					// Tell any connected clients that the anchor list changed
					MikanServer::getInstance()->publishAnchorListChangedEvent();
				}
			}
		});
	constructor.BindEventCallback(
		"erase_anchor",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			const int deleteAnchorIndex = (arguments.size() == 1 ? arguments[0].Get<int>(-1) : 0);
			if (deleteAnchorIndex >= 0 && deleteAnchorIndex < (int)m_dataModel->spatial_anchors.size())
			{
				const MikanSpatialAnchorInfo& deleteAnchor = m_profile->spatialAnchorList[deleteAnchorIndex];

				if (m_profile->removeAnchor(deleteAnchor.anchor_id))
				{
					m_dataModel->rebuildSpatialAnchors(m_profile);

					// Tell any connected clients that the anchor list changed
					MikanServer::getInstance()->publishAnchorListChangedEvent();
				}
			}
		});
	constructor.BindEventCallback(
		"goto_main_menu",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) 
		{
			m_app->popAppState();
		});
	m_dataModel->model_handle = constructor.GetModelHandle();

	// Fill in Data Model initial values	
	m_dataModel->selected_tracker_device = 0;
	if (m_vrTrackers.size() > 0)
	{
		for (int deviceIndex= 0; deviceIndex < (int)m_vrTrackers.size(); ++deviceIndex)
		{
			VRDeviceViewPtr deviceView= m_vrTrackers[deviceIndex];
			const Rml::String friendlyName = deviceView->getTrackerRole() + " - " + deviceView->getSerialNumber();

			m_dataModel->tracker_devices.push_back(friendlyName);

			if (deviceView->getDevicePath() == m_profile->anchorVRDevicePath)
			{
				m_dataModel->selected_tracker_device = deviceIndex;
			}
		}
	}

	m_dataModel->rebuildSpatialAnchors(m_profile);

	addRmlDocument("rml\\spatial_anchor_setup.rml");
}

void AppStage_SpatialAnchors::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("spatial_anchor_settings");

	App::getInstance()->getRenderer()->popCamera();

	for (auto it : m_vrTrackers)
	{
		IVRDeviceInterface* vrDevice= it->getVRDeviceInterface();

		if (vrDevice != nullptr)
		{
			vrDevice->removeFromBoundScene();
		}
	}

	AppStage::exit();
}

void AppStage_SpatialAnchors::update()
{
	if (m_dataModel->model_handle.IsVariableDirty("selected_tracker_device"))
	{
		const int newSelectedIndex = m_dataModel->selected_tracker_device;

		if (newSelectedIndex >= 0 && newSelectedIndex < (int)m_vrTrackers.size())
		{
			m_profile->anchorVRDevicePath = m_vrTrackers[newSelectedIndex]->getDevicePath();
			m_profile->save();
		}
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

VRDeviceViewPtr AppStage_SpatialAnchors::getSelectedAnchorVRTracker() const
{
	return
		(m_dataModel->selected_tracker_device >= 0 && m_dataModel->selected_tracker_device < (int)m_vrTrackers.size())
		? m_vrTrackers[m_dataModel->selected_tracker_device]
		: nullptr;
}