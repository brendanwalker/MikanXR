///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
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
#include "Project/RmlModel_Project.h"
#include "Project/RmlModel_ProjectMarkers.h"
#include "Project/RmlModel_ProjectScenes.h"
#include "Project/RmlModel_ProjectStages.h"
#include "Project/RmlModel_ProjectSources.h"
#include "Project/RmlModel_ProjectTracking.h"
#include "Project/RmlModel_ProjectSettings.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "SceneComponent.h"
#include "SdlCommon.h"
#include "SdlUtility.h"
#include "SharedTextureReader.h"
#include "StencilObjectSystem.h"
#include "SceneObjectSystem.h"
#include "StageObjectSystem.h"
#include "StringUtils.h"
#include "TextureSourceSystem.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingVolumeObjectSystem.h"
#include "TransformComponent.h"
#include "TextStyle.h"
#include "VideoSourceComponent.h"
#include "VideoSourceSystem.h"
#include "Windows/CompositorNodeEditorWindow.h"

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
	, m_projectModel(new RmlModel_Project)
	, m_projectScenesModel(new RmlModel_ProjectScenes)
	, m_projectStagesModel(new RmlModel_ProjectStages)
	, m_projectSourcesModel(new RmlModel_ProjectSources)
	, m_projectTrackingModel(new RmlModel_ProjectTracking)
	, m_projectMarkersModel(new RmlModel_ProjectMarkers)
	, m_projectSettingsModel(new RmlModel_ProjectSettings)
{
}

AppStage_Project::~AppStage_Project()
{
	m_viewport = nullptr;
	m_activeCompositors.clear();

	delete m_projectModel;
	delete m_projectScenesModel;
	delete m_projectStagesModel;
	delete m_projectSourcesModel;
	delete m_projectTrackingModel;
	delete m_projectMarkersModel;
	delete m_projectSettingsModel;
}

void AppStage_Project::enter()
{
	AppStage::enter();

	// Cache a ref to the project
	m_project = getProjectConfig();

	// Cache object systems we'll be accessing
	ProjectManagerPtr objectSystemManager = m_ownerWindow->getProjectManager();
	m_anchorObjectSystem = objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_cameraSystem = objectSystemManager->getSystemOfType<CameraObjectSystem>();
	m_compositorSystem = objectSystemManager->getSystemOfType<CompositorObjectSystem>();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_markerObjectSystem = objectSystemManager->getSystemOfType<MarkerObjectSystem>();
	m_sceneObjectSystem = objectSystemManager->getSystemOfType<SceneObjectSystem>();
	m_stageSystem = objectSystemManager->getSystemOfType<StageObjectSystem>();
	m_stencilObjectSystem = objectSystemManager->getSystemOfType<StencilObjectSystem>();
	m_textureSourceSystem = objectSystemManager->getSystemOfType<TextureSourceSystem>();
	m_trackingMountSystem = objectSystemManager->getSystemOfType<TrackingMountObjectSystem>();
	m_trackingVolumeSystem = objectSystemManager->getSystemOfType<TrackingVolumeObjectSystem>();
	m_videoObjectSystem = objectSystemManager->getSystemOfType<VideoSourceSystem>();

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

		// Init Data Models
		m_projectModel->init(context);
		m_projectModel->OnReturnEvent = MakeDelegate(this, &AppStage_Project::onReturnEvent);
		m_projectModel->OnToggleScenesEvent = MakeDelegate(this, &AppStage_Project::onToggleScenesWindowEvent);
		m_projectModel->OnToggleStagesEvent = MakeDelegate(this, &AppStage_Project::onToggleStagesWindowEvent);
		m_projectModel->OnToggleSourcesEvent = MakeDelegate(this, &AppStage_Project::onToggleSourcesEvent);
		m_projectModel->OnToggleTrackingEvent = MakeDelegate(this, &AppStage_Project::onToggleTrackingEvent);
		m_projectModel->OnToggleMarkersEvent = MakeDelegate(this, &AppStage_Project::onToggleMarkersEvent);
		m_projectModel->OnToggleSettingsEvent = MakeDelegate(this, &AppStage_Project::onToggleSettingsWindowEvent);

		m_projectScenesModel->init(
			context, 
			m_anchorObjectSystem.lock(),
			m_compositorSystem.lock(),
			m_editorSystem.lock(), 
			m_sceneObjectSystem.lock(),
			m_stageSystem.lock(),
			m_stencilObjectSystem.lock());

		m_projectStagesModel->init(
			context, m_project, m_stageSystem.lock(), m_cameraSystem.lock(), m_compositorSystem.lock());

		m_projectSourcesModel->init(
			context, m_project, m_textureSourceSystem.lock(), m_videoObjectSystem.lock());

		m_projectTrackingModel->init(
			context, m_project, m_trackingVolumeSystem.lock(), m_trackingMountSystem.lock());

		m_projectMarkersModel->init(
			context, m_project, m_markerObjectSystem.lock());

		m_projectSettingsModel->init(
			context, m_project, m_stencilObjectSystem.lock());

		// Load the Rml views
		m_projectView = addRmlDocument("project.rml");
		m_projectScenesView = addRmlDocument("project_scenes.rml");
		m_projectSourcesView = addRmlDocument("project_stages.rml");
		m_projectSourcesView = addRmlDocument("project_sources.rml");
		m_projectTrackingView = addRmlDocument("project_tracking.rml");
		m_projectMarkersView = addRmlDocument("project_markers.rml");
		m_projectSettingsView = addRmlDocument("project_settings.rml");

		// Show the main project view by default
		m_projectScenesView->Show();
		m_projectSourcesView->Hide();
		m_projectSourcesView->Hide();
		m_projectTrackingView->Hide();
		m_projectMarkersView->Hide();
		m_projectSettingsView->Hide();
	}
}

void AppStage_Project::exit()
{
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

	m_projectScenesModel->dispose();
	m_projectStagesModel->dispose();
	m_projectSourcesModel->dispose();
	m_projectTrackingModel->dispose();
	m_projectMarkersModel->dispose();
	m_projectSettingsModel->dispose();
	m_projectModel->dispose();

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
	if (m_projectScenesView) m_projectScenesView->Show();
}

void AppStage_Project::onToggleStagesWindowEvent()
{
	hideAllSubWindows();
	if (m_projectSourcesView) m_projectStagesView->Show();
}

void AppStage_Project::onToggleSourcesEvent()
{
	hideAllSubWindows();
	if (m_projectSourcesView) m_projectSourcesView->Show();
}

void AppStage_Project::onToggleTrackingEvent()
{
	hideAllSubWindows();
	if (m_projectTrackingView) m_projectTrackingView->Show();
}

void AppStage_Project::onToggleMarkersEvent()
{
	hideAllSubWindows();
	if (m_projectMarkersView) m_projectMarkersView->Show();
}

void AppStage_Project::onToggleSettingsWindowEvent()
{
	hideAllSubWindows();
	if (m_projectSettingsView) m_projectSettingsView->Show();
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
	SceneComponentConstPtr editorScene= getSystemOfType<EditorObjectSystem>()->getEditorScene();

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
	editorScene->renderEditorScene(
		viewportCamera,
		m_ownerWindow->getMkStateStack());

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