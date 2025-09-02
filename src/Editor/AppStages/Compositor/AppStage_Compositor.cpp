///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "CameraComponent.h"
#include "ClientSourceManager.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"

#include "Compositor/RmlModel_CompositorScenes.h"
#include "Compositor/RmlModel_CompositorStages.h"

#include "Compositor/RmlModel_CompositorLayers.h"
#include "Compositor/RmlModel_CompositorSources.h"
#include "Compositor/RmlModel_CompositorScripting.h"
#include "Compositor/RmlModel_CompositorSelection.h"
#include "Compositor/RmlModel_CompositorSettings.h"
#include "CompositorObjectSystem.h"
#include "CompositorComponent.h"
#include "EditorObjectSystem.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "Colors.h"
#include "CompositorScriptContext.h"
#include "Graphs/CompositorNodeGraph.h"
#include "SdlCommon.h"
#include "MikanCamera.h"
#include "IMkLineRenderer.h"
#include "IMkTextRenderer.h"
#include "MikanRenderModelResource.h"
#include "MikanViewport.h"
#include "IMkWireframeMesh.h"
#include "IMkTexture.h"
#include "SharedTextureReader.h"
#include "InputManager.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "MikanObjectSystem.h"
#include "MathTypeConversion.h"
#include "MathMikan.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "SceneComponent.h"
#include "ObjectSystemManager.h"
#include "ProjectConfig.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "SdlUtility.h"
#include "TransformComponent.h"
#include "ScriptRequestHandler.h"
#include "StringUtils.h"
#include "StencilObjectSystem.h"
#include "SceneObjectSystem.h"
#include "TextStyle.h"
#include "VideoSourceComponent.h"
#include "Windows/CompositorNodeEditorWindow.h"

#include <RmlUi/Core/Context.h>
#include "RmlUI/Core/ElementDocument.h"
#include "RmlUI/Core/Elements/ElementFormControlSelect.h"

#include "tinyfiledialogs.h"

#include <easy/profiler.h>

#include "opencv2/opencv.hpp"

//-- statics ----
const char* AppStage_Compositor::APP_STAGE_NAME = "Compositor";

//-- public methods -----
AppStage_Compositor::AppStage_Compositor(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_Compositor::APP_STAGE_NAME)
	, m_compositorModel(new RmlModel_Compositor)
	, m_compositorScenesModel(new RmlModel_CompositorScenes)
	, m_compositorStagesModel(new RmlModel_CompositorStages)
	, m_compositorSourcesModel(new RmlModel_CompositorSources)
	// TODO: Tracking
	// TODO: Markers
	, m_compositorSelectionModel(new RmlModel_CompositorSelection)
	, m_compositorSettingsModel(new RmlModel_CompositorSettings)
	, m_scriptContext(std::make_shared<CompositorScriptContext>())
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	m_viewport = nullptr;
	m_activeCompositors.clear();

	delete m_compositorModel;
	delete m_compositorStagesModel;
	delete m_compositorSourcesModel;
	// TODO: Tracking
	// TODO: Markers
	delete m_compositorScenesModel;
	delete m_compositorSelectionModel;
	delete m_compositorSettingsModel;
	m_scriptContext.reset();
}

void AppStage_Compositor::enter()
{
	AppStage::enter();

	// Cache a ref to the project
	m_project = App::getInstance()->getProfileConfig();

	// Cache object systems we'll be accessing
	ObjectSystemManagerPtr objectSystemManager = m_ownerWindow->getObjectSystemManager();
	m_anchorObjectSystem = objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_stencilObjectSystem = objectSystemManager->getSystemOfType<StencilObjectSystem>();
	m_sceneObjectSystem = objectSystemManager->getSystemOfType<SceneObjectSystem>();
	m_compositorSystem = objectSystemManager->getSystemOfType<CompositorObjectSystem>();

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
			MakeDelegate(this, &AppStage_Compositor::onSceneActivated);
		sceneSystem->OnSceneDeactivated +=
			MakeDelegate(this, &AppStage_Compositor::onSceneDeactivated);
		
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
			MakeDelegate(this, &AppStage_Compositor::cyclePreviousCompositorCamera);
		inputManager->fetchOrAddKeyBindings(SDLK_PERIOD)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Compositor::cycleNextCompositorCamera);
	}

	// Register the script context with the mikan server
	MikanServer::getInstance()->getScriptRequestHandler()->bindScriptContect(m_scriptContext);

	// Load the compositor script
	if (!m_project->compositorScriptFilePath.empty())
	{
		if (!m_scriptContext->loadScript(m_project->compositorScriptFilePath))
		{
			m_project->compositorScriptFilePath = "";
			m_project->save();
		}
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init main compositor UI
		m_compositorModel->init(context);
		m_compositorModel->OnReturnEvent = MakeDelegate(this, &AppStage_Compositor::onReturnEvent);
		m_compositorModel->OnToggleOutlinerEvent = MakeDelegate(this, &AppStage_Compositor::onToggleScenesWindowEvent);
		m_compositorModel->OnToggleCamerasEvent = MakeDelegate(this, &AppStage_Compositor::onToggleStagesWindowEvent);
		m_compositorModel->OnToggleSourcesEvent = MakeDelegate(this, &AppStage_Compositor::onToggleSourcesEvent);
		// TODO: Tracking
		// TODO: Markers
		m_compositorModel->OnToggleSettingsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleSettingsWindowEvent);
		m_compositiorView = addRmlDocument("compositor.rml");

		// Init Outliner UI
		m_compositorScenesModel->init(context, m_anchorObjectSystem, m_editorSystem, m_stencilObjectSystem);
		m_compositorSelectionModel->init(context, m_anchorObjectSystem, m_editorSystem, m_stencilObjectSystem);
		m_compositiorScenesView = addRmlDocument("compositor_outliner.rml");
		m_compositiorScenesView->Show();

		// Init Cameras UI
		m_compositorStagesModel->init(context);
		m_compositiorSourcesView = addRmlDocument("compositor_cameras.rml");
		m_compositiorSourcesView->Hide();

		// Init Sources UI
		m_compositorSourcesModel->init(context);
		m_compositiorSourcesView = addRmlDocument("compositor_sources.rml");
		m_compositiorSourcesView->Hide();

		// Init Settings UI
		// TODO: Need to pass CompositorDefinition to settings model after refactoring
		m_compositorSettingsModel->init(
			context, 
			m_project, 
			getSystemOfType<StencilObjectSystem>(),
			CompositorDefinitionPtr());
		m_compositiorSettingsView = addRmlDocument("compositor_settings.rml");
		m_compositiorSettingsView->Hide();
	}
}

void AppStage_Compositor::exit()
{
	{
		SceneObjectSystemPtr sceneSystem = getSystemOfType<SceneObjectSystem>();

		// Rebuild compositor viewports for the active scene
		SceneComponentPtr activeScene = sceneSystem->getCurrentScene();
		if (activeScene)
		{
			onSceneDeactivated(activeScene);
		}

		sceneSystem->OnSceneActivated -=
			MakeDelegate(this, &AppStage_Compositor::onSceneActivated);
		sceneSystem->OnSceneDeactivated -=
			MakeDelegate(this, &AppStage_Compositor::onSceneDeactivated);
	}

	// Unregister all viewports from the editor
	EditorObjectSystemPtr editorSystem = m_ownerWindow->getObjectSystemManager()->getSystemOfType<EditorObjectSystem>();
	editorSystem->clearViewports();

	// Unregister the script context with the mikan server
	MikanServer::getInstance()->getScriptRequestHandler()->unbindScriptContect(m_scriptContext);

	m_compositorSelectionModel->dispose();
	m_compositorScenesModel->dispose();
	m_compositorStagesModel->dispose();
	m_compositorSourcesModel->dispose();
	// TODO: Tracking
	// TODO: Markers
	m_compositorSettingsModel->dispose();
	m_compositorModel->dispose();

	// Clear cached object systems
	m_anchorObjectSystem = nullptr;
	m_editorSystem = nullptr;
	m_stencilObjectSystem = nullptr;
	m_sceneObjectSystem = nullptr;
	m_compositorSystem = nullptr;

	AppStage::exit();
}

void AppStage_Compositor::pause()
{
	AppStage::pause();
}

void AppStage_Compositor::resume()
{
	AppStage::resume();

	hideAllSubWindows();
	m_compositiorScenesView->Show();
}

void AppStage_Compositor::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the camera pose for the currently active camera
	updateCompositorCameras();

	// tick the compositor lua script (if any is active)
	m_scriptContext->updateScript();
}

// Scene
void AppStage_Compositor::onSceneDeactivated(SceneComponentPtr oldScene)
{
	disposeCompositorViewportCameras();
}

void AppStage_Compositor::onSceneActivated(SceneComponentPtr newScene)
{
	createCompositorViewportCameras();
}

void AppStage_Compositor::createCompositorViewportCameras()
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

void AppStage_Compositor::disposeCompositorViewportCameras()
{
	// Remove all but the first camera from the main viewport
	while (m_viewport->getCameraCount() > 1)
	{
		m_viewport->removeCameraByIndex(m_viewport->getCameraCount() - 1);
	}
}

void AppStage_Compositor::updateCompositorCameras()
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

void AppStage_Compositor::cyclePreviousCompositorCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		int newCameraIndex = m_viewport->getCurrentCameraIndex() - 1;
		if (newCameraIndex < 0)
			newCameraIndex = m_viewport->getCameraCount() - 1;

		m_viewport->setCurrentCamera(newCameraIndex);
	}
}

void AppStage_Compositor::cycleNextCompositorCamera()
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
void AppStage_Compositor::onReturnEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_Compositor::onToggleScenesWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorScenesView) m_compositiorScenesView->Show();
}

void AppStage_Compositor::onToggleStagesWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorSourcesView) m_compositiorStagesView->Show();
}

void AppStage_Compositor::onToggleSourcesEvent()
{
	hideAllSubWindows();
	if (m_compositiorSourcesView) m_compositiorSourcesView->Show();
}

void AppStage_Compositor::onToggleSettingsWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorSettingsView) m_compositiorSettingsView->Show();
}

void AppStage_Compositor::onScreenshotClientSourceEvent(const std::string& clientSourceName)
{
	auto* clientSourceManager = ClientSourceManager::getInstance();

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

void AppStage_Compositor::hideAllSubWindows()
{
	if (m_compositiorScenesView) m_compositiorScenesView->Hide();
	if (m_compositiorStagesView) m_compositiorStagesView->Hide();
	if (m_compositiorSourcesView) m_compositiorSourcesView->Hide();
	// TODO: Tracking
	// TODO: Markers
	if (m_compositiorSettingsView) m_compositiorSettingsView->Hide();
}

void AppStage_Compositor::render(IMkViewportPtr targetViewport)
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
	m_ownerWindow->getObjectSystemManager()->customRender();

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

void AppStage_Compositor::debugRenderOrigin() const
{
	TextStyle style = getDefaultTextStyle();

	drawTransformedAxes(glm::mat4(1.f), 1.f, 1.f, 1.f);
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
}