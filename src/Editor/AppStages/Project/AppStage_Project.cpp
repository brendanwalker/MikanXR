///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "QuadStencilSystem.h"
#include "ModelStencilSystem.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "ClientSourceManager.h"
#include "Colors.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "Graphs/CompositorNodeGraph.h"
#include "InputManager.h"
#include "IMkLineRenderer.h"
#include "IMkTextRenderer.h"
#include "IMkTexture.h"
#include "IMkWireframeMesh.h"
#include "MathGLM.h"
#include "MainWindow.h"
#include "MarkerObjectSystem.h"
#include "MikanCamera.h"
#include "MikanRenderModelResource.h"
#include "MikanViewport.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "MikanObjectSystem.h"
#include "MathTypeConversion.h"
#include "MathMikan.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "Project/RmlModel_Project.h"
#include "Project/RmlModel_ProjectMarkers.h"
#include "Project/RmlModel_ProjectScenes.h"
#include "Project/RmlModel_ProjectStages.h"
#include "Project/RmlModel_ProjectSources.h"
#include "Project/RmlModel_ProjectTracking.h"
#include "Project/RmlModel_ProjectSettings.h"
#include "Shared/RmlModel_MarkerComponent.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "SceneComponent.h"
#include "SdlCommon.h"
#include "SdlUtility.h"
#include "SharedTextureReader.h"
#include "SceneObjectSystem.h"
#include "StageObjectSystem.h"
#include "StringUtils.h"
#include "TrackingMountObjectSystem.h"
#include "TransformComponent.h"
#include "TextStyle.h"
#include "VideoSourceComponent.h"
#include "Windows/CompositorNodeEditorWindow.h"
#include "USBVideoSourceSystem.h"
#include "NetworkVideoSourceSystem.h"
#include "ClientTextureSourceSystem.h"
#include "SpoutTextureSourceSystem.h"
#include "VRTrackingVolumeSystem.h"
#include "MarkerTrackingVolumeSystem.h"
#include "VRObjectSystem.h"

#include <RmlUi/Core/Context.h>
#include "RmlUI/Core/ElementDocument.h"
#include "RmlUI/Core/Elements/ElementFormControlSelect.h"

#include <easy/profiler.h>

#include "opencv2/opencv.hpp"

//-- statics ----
const char* AppStage_Project::APP_STAGE_NAME = "Compositor";

//-- public methods -----
AppStage_Project::AppStage_Project(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_Project::APP_STAGE_NAME)	
{
}

AppStage_Project::~AppStage_Project()
{
	m_viewport = nullptr;
	m_activeCompositors.clear();
}

void AppStage_Project::enter()
{
	AppStage::enter();

	// Cache a ref to the project
	m_project = getProjectConfig();

	// Cache object systems we'll be accessing
	ProjectManagerPtr objectSystemManager = m_ownerWindow->getProjectManager();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_sceneObjectSystem = objectSystemManager->getSystemOfType<SceneObjectSystem>();

	// Setup Scene viewport
	{
		const glm::i32vec2 viewportOrigin = { 0, 45 };
		const glm::i32vec2 viewportSize = { 1280, 720 };

		m_viewport = getFirstViewport();
		m_viewport->setViewport(viewportOrigin, viewportSize);

		MikanCameraPtr sceneCamera = m_viewport->getCurrentMikanCamera();
		sceneCamera->setCameraMovementMode(eCameraMovementMode::fly);
		sceneCamera->setName("scene camera");
	}

	// Listen for changes to the current active scene
	{
		SceneObjectSystemPtr sceneSystem = getSystemOfType<SceneObjectSystem>();

		sceneSystem->OnSceneActivated +=
			MakeDelegate(this, &AppStage_Project::onSceneActivated);
		sceneSystem->OnSceneDeactivated +=
			MakeDelegate(this, &AppStage_Project::onSceneDeactivated);
		
		// Rebuild compositor viewports for the active scene
		SceneComponentPtr activeScene= sceneSystem->getCurrentScene();
		if (activeScene)
		{
			onSceneActivated(activeScene);
		}
	}

	// Setup hotkeys
	{
		InputManager* inputManager = InputManager::getInstance();

		// Hotkeys for switching between viewport modes
		inputManager->fetchOrAddKeyBindings(SDLK_COMMA)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Project::cyclePreviousCompositorCamera);
		inputManager->fetchOrAddKeyBindings(SDLK_PERIOD)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Project::cycleNextCompositorCamera);
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		m_projectRmlModelContext = new ProjectRmlModelContext(this);
		m_projectRmlModelContext->init();

		// Register Component Models
		m_projectRmlModelContext->getMarkerModel()->OnMarkerSelected = 
			MakeDelegate(this, &AppStage_Project::onMarkerSelected);

		// Register Project Panel Models
		m_projectModel = addRmlModel<RmlModel_Project>();
		m_projectModel->init(context);
		m_projectModel->OnReturnEvent = MakeDelegate(this, &AppStage_Project::onReturnEvent);
		m_projectModel->OnToggleScenesEvent = MakeDelegate(this, &AppStage_Project::onToggleScenesWindowEvent);
		m_projectModel->OnToggleStagesEvent = MakeDelegate(this, &AppStage_Project::onToggleStagesWindowEvent);
		m_projectModel->OnToggleSourcesEvent = MakeDelegate(this, &AppStage_Project::onToggleSourcesEvent);
		m_projectModel->OnToggleTrackingEvent = MakeDelegate(this, &AppStage_Project::onToggleTrackingEvent);
		m_projectModel->OnToggleMarkersEvent = MakeDelegate(this, &AppStage_Project::onToggleMarkersEvent);
		m_projectModel->OnToggleSettingsEvent = MakeDelegate(this, &AppStage_Project::onToggleSettingsWindowEvent);

		m_projectScenesModel = addRmlModel<RmlModel_ProjectScenes>();
		m_projectScenesModel->init(m_projectRmlModelContext);

		m_projectStagesModel = addRmlModel<RmlModel_ProjectStages>();
		m_projectStagesModel->init(m_projectRmlModelContext);

		m_projectSourcesModel = addRmlModel<RmlModel_ProjectSources>();
		m_projectSourcesModel->init(m_projectRmlModelContext);

		m_projectTrackingModel = addRmlModel<RmlModel_ProjectTracking>();
		m_projectTrackingModel->init(m_projectRmlModelContext);

		m_projectMarkersModel = addRmlModel<RmlModel_ProjectMarkers>();
		m_projectMarkersModel->init(m_projectRmlModelContext);

		m_projectSettingsModel = addRmlModel<RmlModel_ProjectSettings>();
		m_projectSettingsModel->init(m_projectRmlModelContext);

		// Load the Rml views
		m_projectScenesView = addRmlDocument("project_scenes.rml");
		m_projectStagesView = addRmlDocument("project_stages.rml");
		m_projectSourcesView = addRmlDocument("project_sources.rml");
		m_projectTrackingView = addRmlDocument("project_tracking.rml");
		m_projectMarkersView = addRmlDocument("project_markers.rml");
		m_projectSettingsView = addRmlDocument("project_settings.rml");

		// Show the main project view by default
		m_projectStagesView->Hide();
		m_projectSourcesView->Hide();
		m_projectTrackingView->Hide();
		m_projectMarkersView->Hide();
		m_projectSettingsView->Hide();
		m_projectScenesView->Show();
		m_projectScenesView->PullToFront();
	}
}

void AppStage_Project::exit()
{
	// Clean up the Rml Model Context
	delete m_projectRmlModelContext;
	m_projectRmlModelContext = nullptr;

	{
		SceneObjectSystemPtr sceneSystem = m_sceneObjectSystem.lock();

		// Rebuild compositor viewports for the active scene
		SceneComponentPtr activeScene = sceneSystem->getCurrentScene();
		if (activeScene)
		{
			onSceneDeactivated(activeScene);
		}

		sceneSystem->OnSceneActivated -=
			MakeDelegate(this, &AppStage_Project::onSceneActivated);
		sceneSystem->OnSceneDeactivated -=
			MakeDelegate(this, &AppStage_Project::onSceneDeactivated);
	}

	// Unregister all viewports from the editor
	m_editorSystem.lock()->clearViewports();

	// RmlModels are now disposed automatically by AppStage::exit()
	AppStage::exit();
}

void AppStage_Project::pause()
{
	AppStage::pause();
}

void AppStage_Project::resume()
{
	AppStage::resume();

	hideAllSubWindows();

	m_projectScenesView->Show();
	m_projectScenesView->PullToFront();
}

void AppStage_Project::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the camera pose for the currently active camera
	updateCompositorCameras();
}

// Scene
void AppStage_Project::onSceneDeactivated(SceneComponentPtr oldScene)
{
	disposeCompositorViewportCameras();
}

void AppStage_Project::onSceneActivated(SceneComponentPtr newScene)
{
	createCompositorViewportCameras();
}

void AppStage_Project::createCompositorViewportCameras()
{
	// Create a camera for each active compositor in the scene
	for (const CompositorComponentWeakPtr compositorWeakPtr : m_activeCompositors)
	{
		CompositorComponentPtr compositor= compositorWeakPtr.lock();
		MikanCameraPtr mikanCamera= m_viewport->addMikanCamera();

		mikanCamera->setName(compositor->getName());

		// Camera transform will be updated each frame in updateCompositorCameras()
		mikanCamera->setCameraMovementMode(eCameraMovementMode::stationary);

		// Apply video source camera intrinsics to the camera
		VideoSourceComponentPtr videoSourceComponent = compositor->getVideoSourceComponent();
		if (videoSourceComponent != nullptr)
		{
			MikanVideoSourceIntrinsics cameraIntrinsics;
			videoSourceComponent->getCameraIntrinsics(cameraIntrinsics);

			mikanCamera->applyMonoCameraIntrinsics(&cameraIntrinsics);
		}
	}

	// Apply initial camera transforms from the compositors
	updateCompositorCameras();
}

void AppStage_Project::disposeCompositorViewportCameras()
{
	// Remove all but the first camera from the main viewport
	while (m_viewport->getCameraCount() > 1)
	{
		m_viewport->removeCameraByIndex(m_viewport->getCameraCount() - 1);
	}
}

void AppStage_Project::updateCompositorCameras()
{
	for (size_t compositorIndex = 0; compositorIndex < m_activeCompositors.size(); compositorIndex++)
	{
		size_t cameraIndex = compositorIndex + 1; // Skip the first camera which is the vr camera
		CompositorComponentPtr compositor = m_activeCompositors[compositorIndex].lock();
		MikanCameraPtr camera = m_viewport->getMikanCameraByIndex((int)cameraIndex);

		if (compositor && camera)
		{
			CameraComponentPtr cameraComponent = compositor->getCameraComponent();
			if (cameraComponent)
			{
				glm::mat4 cameraXform;
				if (cameraComponent->getAperturePose(cameraXform))
				{
					camera->setCameraTransform(cameraXform);
				}
			}
		}
	}
}

void AppStage_Project::cyclePreviousCompositorCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		int newCameraIndex = m_viewport->getCurrentCameraIndex() - 1;
		if (newCameraIndex < 0)
			newCameraIndex = m_viewport->getCameraCount() - 1;

		m_viewport->setCurrentCamera(newCameraIndex);
	}
}

void AppStage_Project::cycleNextCompositorCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		int newCameraIndex = m_viewport->getCurrentCameraIndex() + 1;
		if (newCameraIndex >= m_viewport->getCameraCount())
			newCameraIndex = 0;

		m_viewport->setCurrentCamera(newCameraIndex);
	}
}

// Compositor Model UI Events
void AppStage_Project::onReturnEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_Project::onToggleScenesWindowEvent()
{
	hideAllSubWindows();
	if (m_projectScenesView) {
		m_projectScenesView->Show();
		m_projectScenesView->PullToFront();
	}
}

void AppStage_Project::onToggleStagesWindowEvent()
{
	hideAllSubWindows();
	if (m_projectStagesView) {
		m_projectStagesView->Show();
		m_projectStagesView->PullToFront();
	}
}

void AppStage_Project::onToggleSourcesEvent()
{
	hideAllSubWindows();
	if (m_projectSourcesView) {
		m_projectSourcesView->Show();
		m_projectSourcesView->PullToFront();
	}
}

void AppStage_Project::onToggleTrackingEvent()
{
	hideAllSubWindows();
	if (m_projectTrackingView) {
		m_projectTrackingView->Show();
		m_projectTrackingView->PullToFront();
	}
}

void AppStage_Project::onToggleMarkersEvent()
{
	hideAllSubWindows();
	if (m_projectMarkersView) {
		m_projectMarkersView->Show();
		m_projectMarkersView->PullToFront();
	}
}

void AppStage_Project::onToggleSettingsWindowEvent()
{
	hideAllSubWindows();
	if (m_projectSettingsView) {
		m_projectSettingsView->Show();
		m_projectSettingsView->PullToFront();
	}
}

void AppStage_Project::onScreenshotClientSourceEvent(const std::string& clientSourceName)
{
	auto* clientSourceManager = getOwnerWindow()->getClientSourceManager();

	const auto& clientSources= clientSourceManager->getClientSources();
	if (ClientSourceManager::ClientSource* clientSource = nullptr;
		clientSources.tryGetValue(clientSourceName, clientSource))
	{
		if (clientSource->colorTexture != nullptr)
		{
			SdlUtility::saveTextureToPNG(clientSource->colorTexture, "layerScreenshot.png");
		}
	}
}

void AppStage_Project::onMarkerSelected(int arucoId)
{
	// Update the ArUco preview decorator in the project_markers view
	if (m_projectMarkersView)
	{
		if (auto element = m_projectMarkersView->GetElementById("aruco_preview"))
		{
			auto markerObjectSystem= getObjectSystemOfType<MarkerObjectSystem>();
			const auto markerSystemConfig= markerObjectSystem->getTypedDefinitionConst();
			const int dictionaryType= (int)markerSystemConfig->getArucoDictionaryType();
			const int markerUIPixelSize = 115;

			// Set the decorator with the specific aruco_id
			std::string decoratorValue =
				"aruco-marker(" 
				+ std::to_string(dictionaryType)
				+ ", "
				+ std::to_string(arucoId) 
				+ ", "
				+ std::to_string(markerUIPixelSize)
				+")";
			element->SetProperty("decorator", decoratorValue.c_str());
		}
	}
}

void AppStage_Project::hideAllSubWindows()
{
	if (m_projectScenesView) m_projectScenesView->Hide();
	if (m_projectStagesView) m_projectStagesView->Hide();
	if (m_projectSourcesView) m_projectSourcesView->Hide();
	if (m_projectTrackingView) m_projectTrackingView->Hide();
	if (m_projectMarkersView) m_projectMarkersView->Hide();
	if (m_projectSettingsView) m_projectSettingsView->Hide();
}

void AppStage_Project::render(IMkViewportPtr targetViewport)
{
	MikanCameraPtr viewportCamera = m_viewport->getCurrentMikanCamera();
	int viewportCameraIndex = m_viewport->getCurrentCameraIndex();

	// If we are looking through a compositor camera,
	// we need to render the compositor output to a quad first
	int compositorIndex = viewportCameraIndex - 1; // Skip the first camera which is the vr camera
	if (compositorIndex >= 0 && compositorIndex < (int)m_activeCompositors.size())
	{
		CompositorComponentPtr compositor= m_activeCompositors[compositorIndex].lock();
		compositor->renderToViewportQuad();
	}

	// Render the editor scene
	SceneComponentConstPtr currentScene = getSystemOfType<SceneObjectSystem>()->getCurrentScene();
	if (currentScene)
	{
		currentScene->renderEditorScene(
			viewportCamera,
			m_ownerWindow->getMkStateStack());
	}

	// Perform component custom rendering
	m_ownerWindow->getProjectManager()->customRender();

	// Special debug rendering for just the scene view
	if (compositorIndex == 0)
	{
		// Draw the mouse cursor ray from the pov of the xr camera
		const glm::mat4 glmCameraXform= viewportCamera->getCameraTransformFromViewMatrix();

		// Draw the frustum for the initial camera pose
		const float hfov_radians = degrees_to_radians(viewportCamera->getHorizontalFOVDegrees());
		const float vfov_radians = degrees_to_radians(viewportCamera->getVerticalFOVDegrees());
		const float zNear = fmaxf(viewportCamera->getZNear(), 0.1f);
		const float zFar = fminf(viewportCamera->getZFar(), 2.0f);

		drawTransformedFrustum(
			glmCameraXform,
			hfov_radians, vfov_radians,
			zNear, zFar,
			Colors::Yellow);
		drawTransformedAxes(glmCameraXform, 0.1f);
		
		// Draw tracking space
		drawGrid(glm::mat4(1.f), 10.f, 10.f, 20, 20, Colors::GhostWhite);
	}

	if (m_project->getRenderOriginFlag())
	{
		debugRenderOrigin();
	}
}

void AppStage_Project::debugRenderOrigin() const
{
	TextStyle style = getDefaultTextStyle();

	drawTransformedAxes(glm::mat4(1.f), 1.f, 1.f, 1.f);
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
}

// -- IRemoteControllable Interface -- //
namespace {
	// Template helper for add commands
	template<typename SystemType>
	bool handleAddCommand(AppStage_Project* appStage)
	{
		auto system = appStage->getObjectSystemOfType<SystemType>();
		system->addNewObject();
		return true;
	}

	// Template helper for remove commands
	template<typename SystemType, typename IDType>
	bool handleRemoveCommand(AppStage_Project* appStage, const std::vector<std::string>& parameters)
	{
		if (parameters.size() >= 1)
		{
			int componentId = std::stoi(parameters[0]);
			auto system = appStage->getObjectSystemOfType<SystemType>();
			system->removeObject(static_cast<IDType>(componentId));
			return true;
		}
		return false;
	}
}

bool AppStage_Project::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	// Scene CRUD
	if (command == "add_scene")
		return handleAddCommand<SceneObjectSystem>(this);
	else if (command == "remove_scene")
		return handleRemoveCommand<SceneObjectSystem, MikanSceneID>(this, parameters);
	// Stage CRUD
	else if (command == "add_new_stage")
		return handleAddCommand<StageObjectSystem>(this);
	else if (command == "remove_stage")
		return handleRemoveCommand<StageObjectSystem, MikanStageID>(this, parameters);
	// Camera CRUD
	else if (command == "add_new_camera")
		return handleAddCommand<CameraObjectSystem>(this);
	else if (command == "remove_camera")
		return handleRemoveCommand<CameraObjectSystem, MikanCameraID>(this, parameters);
	// Compositor CRUD
	else if (command == "add_new_compositor")
		return handleAddCommand<CompositorObjectSystem>(this);
	else if (command == "remove_compositor")
		return handleRemoveCommand<CompositorObjectSystem, MikanCompositorID>(this, parameters);
	// Video Source CRUD
	else if (command == "add_new_usb_video_source")
		return handleAddCommand<USBVideoSourceSystem>(this);
	else if (command == "add_new_network_video_source")
		return handleAddCommand<NetworkVideoSourceSystem>(this);
	else if (command == "remove_usb_video_source")
		return handleRemoveCommand<USBVideoSourceSystem, MikanVideoSourceID>(this, parameters);
	else if (command == "remove_network_video_source")
		return handleRemoveCommand<NetworkVideoSourceSystem, MikanVideoSourceID>(this, parameters);
	// Texture Source CRUD
	else if (command == "add_new_client_texture_source")
		return handleAddCommand<ClientTextureSourceSystem>(this);
	else if (command == "add_new_spout_texture_source")
		return handleAddCommand<SpoutTextureSourceSystem>(this);
	else if (command == "remove_client_texture_source")
		return handleRemoveCommand<ClientTextureSourceSystem, MikanTextureSourceID>(this, parameters);
	else if (command == "remove_spout_texture_source")
		return handleRemoveCommand<SpoutTextureSourceSystem, MikanTextureSourceID>(this, parameters);
	// Tracking Volume CRUD
	else if (command == "add_new_steamvr_tracking_volume")
		return handleAddCommand<VRTrackingVolumeSystem>(this);
	else if (command == "add_new_marker_tracking_volume")
		return handleAddCommand<MarkerTrackingVolumeSystem>(this);
	else if (command == "remove_vr_tracking_volume")
		return handleRemoveCommand<VRTrackingVolumeSystem, MikanTrackingVolumeID>(this, parameters);
	else if (command == "remove_marker_tracking_volume")
		return handleRemoveCommand<MarkerTrackingVolumeSystem, MikanTrackingVolumeID>(this, parameters);
	// Tracking Mount CRUD
	else if (command == "add_new_tracking_mount")
		return handleAddCommand<TrackingMountObjectSystem>(this);
	else if (command == "remove_tracking_mount")
		return handleRemoveCommand<TrackingMountObjectSystem, MikanTrackingMountID>(this, parameters);
	// Marker CRUD
	else if (command == "add_new_marker")
		return handleAddCommand<MarkerObjectSystem>(this);
	else if (command == "remove_marker")
		return handleRemoveCommand<MarkerObjectSystem, MikanMarkerID>(this, parameters);
	// Scene Object CRUD (anchors, stencils, scripts)
	else if (command == "add_new_anchor")
		return handleAddCommand<AnchorObjectSystem>(this);
	else if (command == "add_new_quad")
		return handleAddCommand<QuadStencilSystem>(this);
	else if (command == "remove_quad")
		return handleRemoveCommand<QuadStencilSystem, MikanStencilID>(this, parameters);
	else if (command == "add_new_box")
		return handleAddCommand<BoxStencilSystem>(this);
	else if (command == "remove_box")
		return handleRemoveCommand<BoxStencilSystem, MikanStencilID>(this, parameters);
	else if (command == "add_new_model")
		return handleAddCommand<ModelStencilSystem>(this);
	else if (command == "remove_model")
		return handleRemoveCommand<ModelStencilSystem, MikanStencilID>(this, parameters);
	else if (command == "add_new_script")
	{
		// TODO: Implement script removal - need to determine component ID parameter handling
		return true;
	}
	else if (command == "remove_script")
	{
		// TODO: Implement script removal - need to determine component ID parameter handling
		return true;
	}

	return AppStage::handleRemoteControlCommand(command, parameters, outResults);
}