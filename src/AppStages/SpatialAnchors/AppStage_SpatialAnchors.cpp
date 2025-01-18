//-- inludes -----
#include "SpatialAnchors/AppStage_SpatialAnchors.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "App.h"
#include "Colors.h"
#include "GlCamera.h"
#include "GlScene.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "GlViewport.h"
#include "MainWindow.h"
#include "MathUtility.h"
#include "ObjectSystemManager.h"
#include "RmlModel_SpatialAnchors.h"
#include "StringUtils.h"
#include "VRDeviceView.h"
#include "VRDeviceManager.h"

#include <glm/gtc/matrix_transform.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----__
const char* AppStage_SpatialAnchors::APP_STAGE_NAME = "Spatial Anchor Setup";

//-- public methods -----
AppStage_SpatialAnchors::AppStage_SpatialAnchors(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_SpatialAnchors::APP_STAGE_NAME)
	, m_dataModel(new RmlModel_SpatialAnchors)
	, m_scene(std::make_shared<GlScene>())
	, m_camera(nullptr)
	, m_profile(nullptr)
{ 
}

AppStage_SpatialAnchors::~AppStage_SpatialAnchors()
{
	delete m_dataModel;
}

void AppStage_SpatialAnchors::enter()
{
	AppStage::enter();

	App* app= App::getInstance();
	m_profile = app->getProfileConfig();
	m_anchorSystem= m_ownerWindow->getObjectSystemManager()->getSystemOfType<AnchorObjectSystem>();
	m_anchorSystemConfig = m_anchorSystem->getAnchorSystemConfig();

	// Build the list of VR trackers
	m_vrTrackers = m_ownerWindow->getVRDeviceManager()->getFilteredVRDeviceList(eDeviceType::VRTracker);

	// Bind tracker to 3D scene
	for (auto it : m_vrTrackers)
	{
		it->getVRDeviceInterface()->bindToScene(m_scene);
	}

	// Setup orbit camera
	m_camera = getFirstViewport()->getCurrentCamera();
	m_camera->setOrbitPitch(20.0f);
	m_camera->setOrbitYaw(45.0f);
	m_camera->setOrbitRadius(3.5f);

	Rml::Context* context = getRmlContext();

	m_dataModel->init(context, m_anchorSystem);
	m_dataModel->OnAddNewAnchor= MakeDelegate(this, &AppStage_SpatialAnchors::onAddNewAnchor);
	m_dataModel->OnUpdateAnchorPose= MakeDelegate(this, &AppStage_SpatialAnchors::onUpdateAnchorPose);
	m_dataModel->OnUpdateAnchorName= MakeDelegate(this, &AppStage_SpatialAnchors::onUpdateAnchorName);
	m_dataModel->OnUpdateAnchorVRDevicePath= MakeDelegate(this, &AppStage_SpatialAnchors::onUpdateAnchorVRDevicePath);
	m_dataModel->OnDeleteAnchor= MakeDelegate(this, &AppStage_SpatialAnchors::onDeleteAnchor);
	m_dataModel->OnGotoMainMenu= MakeDelegate(this, &AppStage_SpatialAnchors::onGotoMainMenu);

	addRmlDocument("spatial_anchor_setup.rml");
}

void AppStage_SpatialAnchors::onAddNewAnchor()
{
	VRDevicePoseViewPtr anchorVRDevicePose = getSelectedAnchorVRTrackerPoseView();

	glm::mat4 anchorXform = glm::mat4(1.f);
	if (anchorVRDevicePose == nullptr)
	{
		anchorVRDevicePose->getPose(anchorXform);
	}
	const std::string newAnchorName= StringUtils::stringify("Anchor ",m_anchorSystemConfig->nextAnchorId);

	if (m_anchorSystem->addNewAnchor(newAnchorName, anchorXform))
	{
		m_dataModel->rebuildAnchorList();
	}
}

void AppStage_SpatialAnchors::onUpdateAnchorPose(int anchorId)
{
	AnchorComponentPtr anchorComponent = m_anchorSystem->getSpatialAnchorById(anchorId);
	if (anchorComponent != nullptr)
	{
		glm::mat4 anchorXform;
		VRDevicePoseViewPtr anchorVRDevicePoseView = getSelectedAnchorVRTrackerPoseView();
		if (anchorVRDevicePoseView != nullptr &&
			anchorVRDevicePoseView->getPose(anchorXform))
		{
			anchorComponent->setWorldTransform(anchorXform);
		}
	}
}

void AppStage_SpatialAnchors::onUpdateAnchorName(int anchorId, const std::string& anchorName)
{
	AnchorComponentPtr anchorComponent = m_anchorSystem->getSpatialAnchorById(anchorId);
	if (anchorComponent != nullptr)
	{
		anchorComponent->setName(anchorName);
	}
}

void AppStage_SpatialAnchors::onUpdateAnchorVRDevicePath(const std::string& vrDevicePath)
{
	AnchorObjectSystemConfigPtr configPtr= m_anchorSystem->getAnchorSystemConfig();
	if (configPtr->anchorVRDevicePath != vrDevicePath)
	{
		configPtr->anchorVRDevicePath= vrDevicePath;
		configPtr->markDirty(
			ConfigPropertyChangeSet()
			.addPropertyName(AnchorObjectSystemConfig::k_anchorVRDevicePathPropertyId));

		m_dataModel->rebuildAnchorList();
	}
}

void AppStage_SpatialAnchors::onDeleteAnchor(int deleteAnchorId)
{
	if (m_anchorSystem->removeAnchor(deleteAnchorId))
	{
		m_dataModel->rebuildAnchorList();
	}
}

void AppStage_SpatialAnchors::onGotoMainMenu()
{
	m_ownerWindow->popAppState();
}

void AppStage_SpatialAnchors::exit()
{
	// Clean up the data model
	m_dataModel->dispose();

	m_camera= nullptr;

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

void AppStage_SpatialAnchors::render()
{
	TextStyle style = getDefaultTextStyle();

	m_scene->render(m_camera, m_ownerWindow->getGlStateStack());

	drawGrid(glm::mat4(1.f), 10.f, 10.f, 20, 20, Colors::GhostWhite);
	drawTransformedAxes(glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.001f, 0.f)), 0.5f);

	// Highlight the VR device we are using for anchor placement
	glm::mat4 anchorXform;
	VRDevicePoseViewPtr anchorVRDevice = getSelectedAnchorVRTrackerPoseView();
	if (anchorVRDevice != nullptr && 
		anchorVRDevice->getPose(anchorXform))
	{
		const glm::vec3 anchorPos= glm::vec3(anchorXform[3]);

		drawTransformedAxes(anchorXform, 0.2f);
		drawTransformedBox(anchorXform, glm::vec3(-0.1f, -0.1f, -0.1f), glm::vec3(0.1f, 0.1f, 0.1f), Colors::Yellow);
		drawTextAtWorldPosition(style, anchorPos, L"Anchor VR Device");
	}

	// Draw the anchors
	for (auto it : m_anchorSystem->getAnchorMap())
	{
		AnchorComponentPtr anchor= it.second.lock();

		wchar_t wszAnchorName[256];
		StringUtils::convertMbsToWcs(anchor->getName().c_str(), wszAnchorName, sizeof(wszAnchorName));
		glm::mat4 anchorXform = anchor->getWorldTransform();
		glm::vec3 anchorPos(anchorXform[3]);

		drawTransformedAxes(anchorXform, 0.1f);
		drawTextAtWorldPosition(style, anchorPos, wszAnchorName);
	}
}

VRDeviceViewPtr AppStage_SpatialAnchors::getSelectedAnchorVRTracker() const
{
	const std::string vrDevicePath= m_dataModel->getAnchorVRDevicePath();

	return VRDeviceManager::getInstance()->getVRDeviceViewByPath(vrDevicePath);
}

VRDevicePoseViewPtr AppStage_SpatialAnchors::getSelectedAnchorVRTrackerPoseView() const
{
	VRDeviceViewPtr anchorVRDevice = getSelectedAnchorVRTracker();

	return (anchorVRDevice) ? anchorVRDevice->makePoseView(eVRDevicePoseSpace::MikanScene) : nullptr;
}