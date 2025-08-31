///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "CameraComponent.h"
#include "ClientSourceManager.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"
#include "Compositor/RmlModel_CompositorCameras.h"
#include "Compositor/RmlModel_CompositorLayers.h"
#include "Compositor/RmlModel_CompositorOutliner.h"
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
AppStage_Compositor::AppStage_Compositor(MainWindow* window)
	: AppStage(window, AppStage_Compositor::APP_STAGE_NAME)
	, m_compositorModel(new RmlModel_Compositor)
	, m_compositorLayersModel(new RmlModel_CompositorLayers)
	, m_compositorCamerasModel(new RmlModel_CompositorCameras)
	//, m_compositorSourcesModel(new RmlModel_CompositorSources)
	, m_compositorScriptingModel(new RmlModel_CompositorScripting)
	, m_compositorOutlinerModel(new RmlModel_CompositorOutliner)
	, m_compositorSelectionModel(new RmlModel_CompositorSelection)
	, m_compositorSettingsModel(new RmlModel_CompositorSettings)
	, m_scriptContext(std::make_shared<CompositorScriptContext>())
	//, m_renderTargetWriteAccessor()
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	//m_renderTargetWriteAccessor= nullptr;
	m_viewport = nullptr;
	m_activeCompositors.clear();

	delete m_compositorModel;
	delete m_compositorLayersModel;
	delete m_compositorCamerasModel;
	//delete m_compositorSourcesModel;
	delete m_compositorScriptingModel;
	delete m_compositorOutlinerModel;
	delete m_compositorSelectionModel;
	delete m_compositorSettingsModel;
	m_scriptContext.reset();
}

void AppStage_Compositor::enter()
{
	AppStage::enter();

	// Cache a ref to the project
	m_project = App::getInstance()->getProfileConfig();
	//m_project->OnMarkedDirty +=
	//	MakeDelegate(this, &AppStage_Compositor::onProjectConfigMarkedDirty);

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
		SceneObjectSystemPtr sceneSystem = SceneObjectSystem::getSystem();

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
		m_compositorModel->OnToggleOutlinerEvent = MakeDelegate(this, &AppStage_Compositor::onToggleOutlinerWindowEvent);
		m_compositorModel->OnToggleLayersEvent = MakeDelegate(this, &AppStage_Compositor::onToggleLayersWindowEvent);
		m_compositorModel->OnToggleCamerasEvent = MakeDelegate(this, &AppStage_Compositor::onToggleCamerasWindowEvent);
		m_compositorModel->OnToggleSourcesEvent = MakeDelegate(this, &AppStage_Compositor::onToggleSourcesEvent);
		m_compositorModel->OnToggleScriptingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleScriptingWindowEvent);
		m_compositorModel->OnToggleSettingsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleSettingsWindowEvent);
		m_compositiorView = addRmlDocument("compositor.rml");

		// Init Outliner UI
		m_compositorOutlinerModel->init(context, m_anchorObjectSystem, m_editorSystem, m_stencilObjectSystem);
		m_compositorSelectionModel->init(context, m_anchorObjectSystem, m_editorSystem, m_stencilObjectSystem);
		m_compositiorOutlinerView = addRmlDocument("compositor_outliner.rml");
		m_compositiorOutlinerView->Show();

		// TODO
		// Init Layers UI
		//m_compositorLayersModel->init(context, m_frameCompositor);
		//m_compositorLayersModel->OnGraphEditEvent = MakeDelegate(this, &AppStage_Compositor::onGraphEditEvent);
		//m_compositorLayersModel->OnGraphFileSelectEvent = MakeDelegate(this, &AppStage_Compositor::onGraphFileSelectEvent);
		//m_compositiorLayersView = addRmlDocument("compositor_layers.rml");
		//m_compositiorLayersView->Hide();

		// Init Cameras UI
		m_compositorCamerasModel->init(context);
		m_compositiorSourcesView = addRmlDocument("compositor_cameras.rml");
		m_compositiorSourcesView->Hide();

		// Init Sources UI
		//m_compositorSourcesModel->init(context);
		//m_compositiorSourcesView = addRmlDocument("compositor_sources.rml");
		//m_compositiorSourcesView->Hide();

		// Init Scripting UI
		m_compositorScriptingModel->init(context, m_project, m_scriptContext);
		m_compositorScriptingModel->OnScriptFileChangeEvent = MakeDelegate(this, &AppStage_Compositor::onScriptFileChangeEvent);
		m_compositorScriptingModel->OnSelectCompositorScriptFileEvent = MakeDelegate(this, &AppStage_Compositor::onSelectCompositorScriptFileEvent);
		m_compositorScriptingModel->OnReloadCompositorScriptFileEvent = MakeDelegate(this, &AppStage_Compositor::onReloadCompositorScriptFileEvent);
		m_compositorScriptingModel->OnInvokeScriptTriggerEvent = MakeDelegate(this, &AppStage_Compositor::onInvokeScriptTriggerEvent);
		m_compositiorScriptingView = addRmlDocument("compositor_scripting.rml");
		m_compositiorScriptingView->Hide();

		// Init Settings UI
		m_compositorSettingsModel->init(context, m_project);
		m_compositiorSettingsView = addRmlDocument("compositor_settings.rml");
		m_compositiorSettingsView->Hide();
	}

	// Setup render target write accessor
	//m_renderTargetWriteAccessor =
	//	createSharedTextureWriteAccessor(m_project->getSpoutOutputName());
	//onSpoutStreamingFlagChanged();
}

void AppStage_Compositor::exit()
{
	// Stop listening for changes to the current active compositor
	//if (m_frameCompositor)
	//{
	//	// Clean up the current compositor state
	//	onCompositorDeactivated(m_frameCompositor);
	//}

	{
		SceneObjectSystemPtr sceneSystem = SceneObjectSystem::getSystem();

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

	// Clean up spout output stream
	//stopStreaming();
	//m_renderTargetWriteAccessor= nullptr;

	m_compositorSelectionModel->dispose();
	m_compositorOutlinerModel->dispose();
	m_compositorLayersModel->dispose();
	//m_compositorSourcesModel->dispose();
	m_compositorScriptingModel->dispose();
	m_compositorModel->dispose();
	m_compositorSettingsModel->dispose();

	// Stop listening for project config changes
	//m_project->OnMarkedDirty -=
	//	MakeDelegate(this, &AppStage_Compositor::onProjectConfigMarkedDirty);

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

	//m_frameCompositor->stop();
}

void AppStage_Compositor::resume()
{
	AppStage::resume();

	//m_frameCompositor->start();

	hideAllSubWindows();
	m_compositiorOutlinerView->Show();
}

void AppStage_Compositor::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the camera pose for the currently active camera
	updateCompositorCameras();

	// tick the compositor lua script (if any is active)
	m_scriptContext->updateScript();
}

//bool AppStage_Compositor::startStreaming()
//{
//	if (getIsStreaming())
//		return true;
//
//	IMkTextureConstPtr compositorTexture = m_frameCompositor->getCompositedFrameTexture();
//	if (compositorTexture == nullptr)
//		return false;
//
//	// Compositing buffer should always be RGBA 32BPP
//	// Spout can only support RGBA32 and BGRA32
//	assert(compositorTexture->getBufferFormat() == GL_RGBA);
//
//	SharedTextureDescriptor sharedTextureDescriptor;
//	sharedTextureDescriptor.color_buffer_type = SharedColorBufferType::RGBA32;
//	sharedTextureDescriptor.depth_buffer_type = SharedDepthBufferType::NODEPTH;
//	sharedTextureDescriptor.width = compositorTexture->getTextureWidth();
//	sharedTextureDescriptor.height = compositorTexture->getTextureHeight();
//	sharedTextureDescriptor.graphicsAPI = SharedClientGraphicsApi::OpenGL;
//
//	m_renderTargetWriteAccessor->initialize(&sharedTextureDescriptor, true, nullptr);
//
//	return true;
//}

//bool AppStage_Compositor::getIsStreaming()
//{
//	return m_renderTargetWriteAccessor->getIsInitialized();
//}

//void AppStage_Compositor::stopStreaming()
//{
//	m_renderTargetWriteAccessor->dispose();
//}

// Scene
void AppStage_Compositor::onSceneDeactivated(SceneComponentPtr oldScene)
{
	disposeCompositorCameras();
}

void AppStage_Compositor::onSceneActivated(SceneComponentPtr newScene)
{
	createCompositorCameras();
}

//void AppStage_Compositor::onNewStreamingFrameReady()
//{
//	EASY_FUNCTION();
//
//	if (getIsStreaming())
//	{
//		IMkTextureConstPtr frameTexture = m_frameCompositor->getCompositedFrameTexture();
//
//		if (frameTexture != nullptr && m_renderTargetWriteAccessor->getIsInitialized())
//		{
//			GLuint textureId= frameTexture->getGlTextureId();
//
//			m_renderTargetWriteAccessor->writeColorFrameTexture(&textureId);
//		}
//	}
//}

// Project Config Events
//void AppStage_Compositor::onProjectConfigMarkedDirty(
//	CommonConfigPtr configPtr,
//	const class ConfigPropertyChangeSet& changedPropertySet)
//{
//	if (changedPropertySet.hasPropertyName(ProjectConfig::k_spoutOutputIsStreamingNamePropertyId))
//	{
//		onSpoutStreamingFlagChanged();
//	}
//	else if (changedPropertySet.hasPropertyName(ProjectConfig::k_spoutOutputNamePropertyId))
//	{
//		onSpoutOutputNameChanged();
//	}
//}

// Spout Streaming Config Events
//void AppStage_Compositor::onSpoutOutputNameChanged()
//{
//	m_renderTargetWriteAccessor->setClientName(m_project->getSpoutOutputName());
//}
//
//void AppStage_Compositor::onSpoutStreamingFlagChanged()
//{
//	const bool bIsStreaming = getIsStreaming();
//	const bool bWantsStreaming = m_project->getIsSpoutOutputStreaming();
//
//	if (!bIsStreaming && bWantsStreaming)
//	{
//		startStreaming();
//	}
//	else if (bIsStreaming && !bWantsStreaming)
//	{
//		stopStreaming();
//	}
//}

void AppStage_Compositor::createCompositorCameras()
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

void AppStage_Compositor::disposeCompositorCameras()
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

void AppStage_Compositor::onToggleCamerasWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorSourcesView) m_compositiorCamerasView->Show();
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

//-- Deprecated --
void AppStage_Compositor::onToggleOutlinerWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorOutlinerView) m_compositiorOutlinerView->Show();
}

void AppStage_Compositor::onToggleLayersWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorLayersView) m_compositiorLayersView->Show();
}

void AppStage_Compositor::onToggleScriptingWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorScriptingView) m_compositiorScriptingView->Show();
}
//-- Deprecated --

// Compositor Layers UI Events
//void AppStage_Compositor::onGraphEditEvent()
//{
//	App* app= App::getInstance();	
//
//	if (!app->hasWindowOfType<CompositorNodeEditorWindow>())
//	{
//		auto* compositorNodeEditor= app->createAppWindow<CompositorNodeEditorWindow>();
//
//		// Bind the current compositor graph to the editor
//		NodeGraphPtr nodeGraph = compositorNodeEditor->getNodeGraph();
//		if (nodeGraph)
//		{
//			auto compositorNodeGraph = std::static_pointer_cast<CompositorNodeGraph>(nodeGraph);
//			compositorNodeGraph->bindToCompositorComponent(m_frameCompositor);
//		}
//	}
//}

//void AppStage_Compositor::onGraphFileSelectEvent()
//{
//	const char* filterItems[1] = {"*.graph"};
//	const char* filterDesc = "Graph Files (*.graph)";
//	auto path = tinyfd_openFileDialog("Load Compositor Graph", "", 1, filterItems, filterDesc, 1);
//	if (path)
//	{
//		m_frameCompositor->getCompositorDefinition()->setCompositorGraphPath(path);
//	}
//}

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
	if (m_compositiorOutlinerView) m_compositiorOutlinerView->Hide();
	if (m_compositiorLayersView) m_compositiorLayersView->Hide();
	if (m_compositiorSourcesView) m_compositiorSourcesView->Hide();
	if (m_compositiorScriptingView) m_compositiorScriptingView->Hide();
	if (m_compositiorSettingsView) m_compositiorSettingsView->Hide();
}

// Scripting UI Events
void AppStage_Compositor::onScriptFileChangeEvent(
	const std::filesystem::path& filepath)
{
	if (m_scriptContext->loadScript(filepath))
	{
		m_project->compositorScriptFilePath = filepath;
		m_project->save();

		m_compositorScriptingModel->setCompositorScriptPath(filepath);
	}
}

void AppStage_Compositor::onSelectCompositorScriptFileEvent()
{
	std::string defaultFileAndPath;
	if (!m_project->compositorScriptFilePath.empty())
	{
		defaultFileAndPath= m_project->compositorScriptFilePath.string();
	}
	else
	{
		defaultFileAndPath= PathUtils::getHomeDirectory().string();
	}

	const char* filterItems[1] = {"*.lua"};
	const char* filterDesc = "Scene Scripts (*.lua)";
	char* path = 
		tinyfd_openFileDialog(
			"Select Scene Script", 
			defaultFileAndPath.c_str(), 
			1, filterItems, 
			filterDesc, 
			0); // Don't allow multiple selects
	if (path)
	{
		onScriptFileChangeEvent(path);
	}
}

void AppStage_Compositor::onReloadCompositorScriptFileEvent()
{
	if (m_scriptContext->hasScriptFilename())
	{
		if (m_scriptContext->reloadScript())
		{
			m_compositorScriptingModel->rebuildScriptTriggers();
		}
	}
}

void AppStage_Compositor::onInvokeScriptTriggerEvent(const std::string& triggerEvent)
{
	if (m_scriptContext->hasLoadedScript())
	{
		m_scriptContext->invokeScriptTrigger(triggerEvent);
	}
}

void AppStage_Compositor::render(IMkViewportPtr targetViewport)
{
	SceneComponentConstPtr editorScene= EditorObjectSystem::getSystem()->getEditorScene();

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