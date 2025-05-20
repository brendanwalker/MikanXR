///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"
#include "Compositor/RmlModel_CompositorCameras.h"
#include "Compositor/RmlModel_CompositorLayers.h"
#include "Compositor/RmlModel_CompositorOutliner.h"
#include "Compositor/RmlModel_CompositorSources.h"
#include "Compositor/RmlModel_CompositorScripting.h"
#include "Compositor/RmlModel_CompositorSelection.h"
#include "Compositor/RmlModel_CompositorSettings.h"
#include "EditorObjectSystem.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "Colors.h"
#include "CompositorScriptContext.h"
#include "SdlCommon.h"
#include "MikanCamera.h"
#include "GlFrameCompositor.h"
#include "IMkLineRenderer.h"
#include "IMkTextRenderer.h"
#include "MikanRenderModelResource.h"
#include "MikanViewport.h"
#include "IMkWireframeMesh.h"
#include "IMkTexture.h"
#include "SharedTextureReader.h"
#include "SharedTextureWriter.h"
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
#include "MikanScene.h"
#include "ObjectSystemManager.h"
#include "ProjectConfig.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "SdlUtility.h"
#include "TransformComponent.h"
#include "StringUtils.h"
#include "StencilObjectSystem.h"
#include "SceneObjectSystem.h"
#include "TextStyle.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"
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
	, m_compositorSourcesModel(new RmlModel_CompositorSources)
	, m_compositorScriptingModel(new RmlModel_CompositorScripting)
	, m_compositorOutlinerModel(new RmlModel_CompositorOutliner)
	, m_compositorSelectionModel(new RmlModel_CompositorSelection)
	, m_compositorSettingsModel(new RmlModel_CompositorSettings)
	, m_scriptContext(std::make_shared<CompositorScriptContext>())
	, m_renderTargetWriteAccessor()
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	m_renderTargetWriteAccessor= nullptr;
	m_viewport= nullptr;

	delete m_compositorModel;
	delete m_compositorLayersModel;
	delete m_compositorCamerasModel;
	delete m_compositorSourcesModel;
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
	m_project->OnMarkedDirty +=
		MakeDelegate(this, &AppStage_Compositor::onProjectConfigMarkedDirty);

	// Cache object systems we'll be accessing
	ObjectSystemManagerPtr objectSystemManager = m_ownerWindow->getObjectSystemManager();
	m_anchorObjectSystem = objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_stencilObjectSystem = objectSystemManager->getSystemOfType<StencilObjectSystem>();
	m_sceneObjectSystem = objectSystemManager->getSystemOfType<SceneObjectSystem>();

	// Start the frame compositor
	m_frameCompositor= GlFrameCompositor::getInstance();
	m_frameCompositor->start();

	// Setup viewport
	m_viewport = getFirstViewport();
	const glm::i32vec2 viewportOrigin = {0, 45};
	const glm::i32vec2 viewportSize = {1280, 720};
	m_viewport->setViewport(viewportOrigin, viewportSize);

	// Create and bind cameras
	setupCameras();

	// Register the scene with the primary viewport
	m_editorSystem->bindViewport(m_viewport);

	// Apply video source camera intrinsics to the camera
	VideoSourceViewPtr videoSourceView = m_frameCompositor->getVideoSource();
	if (videoSourceView != nullptr)
	{
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);

		for (MikanViewportPtr viewport : getViewportList())
		{
			MikanCameraPtr camera= getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);

			camera->applyMonoCameraIntrinsics(&cameraIntrinsics);
		}
	}

	// Register the script context with the mikan server
	MikanServer::getInstance()->bindScriptContect(m_scriptContext);

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

		// Init Layers UI
		m_compositorLayersModel->init(context, m_frameCompositor);
		m_compositorLayersModel->OnGraphEditEvent = MakeDelegate(this, &AppStage_Compositor::onGraphEditEvent);
		m_compositorLayersModel->OnGraphFileSelectEvent = MakeDelegate(this, &AppStage_Compositor::onGraphFileSelectEvent);
		m_compositorLayersModel->OnConfigAddEvent = MakeDelegate(this, &AppStage_Compositor::onConfigAddEvent);
		m_compositorLayersModel->OnConfigDeleteEvent = MakeDelegate(this, &AppStage_Compositor::onConfigDeleteEvent);
		m_compositorLayersModel->OnConfigNameChangeEvent = MakeDelegate(this, &AppStage_Compositor::onConfigNameChangeEvent);
		m_compositorLayersModel->OnConfigSelectEvent = MakeDelegate(this, &AppStage_Compositor::onConfigSelectEvent);
		m_compositiorLayersView = addRmlDocument("compositor_layers.rml");
		m_compositiorLayersView->Hide();

		// Init Cameras UI
		m_compositorCamerasModel->init(context, m_frameCompositor);
		m_compositiorSourcesView = addRmlDocument("compositor_cameras.rml");
		m_compositiorSourcesView->Hide();

		// Init Sources UI
		m_compositorSourcesModel->init(context, m_frameCompositor);
		m_compositiorSourcesView = addRmlDocument("compositor_sources.rml");
		m_compositiorSourcesView->Hide();

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
	m_renderTargetWriteAccessor =
		createSharedTextureWriteAccessor(m_project->getSpoutOutputName());
	onSpoutStreamingFlagChanged();
}

void AppStage_Compositor::exit()
{
	// Unregister all viewports from the editor
	App* app= App::getInstance();
	EditorObjectSystemPtr editorSystem = m_ownerWindow->getObjectSystemManager()->getSystemOfType<EditorObjectSystem>();
	editorSystem->clearViewports();

	// Unregister the script context with the mikan server
	MikanServer::getInstance()->unbindScriptContect(m_scriptContext);

	// Clean up spout output stream
	stopStreaming();
	m_renderTargetWriteAccessor= nullptr;

	m_compositorSelectionModel->dispose();
	m_compositorOutlinerModel->dispose();
	m_compositorLayersModel->dispose();
	m_compositorSourcesModel->dispose();
	m_compositorScriptingModel->dispose();
	m_compositorModel->dispose();
	m_compositorSettingsModel->dispose();

	m_frameCompositor->stop();

	// Stop listening for project config changes
	m_project->OnMarkedDirty -=
		MakeDelegate(this, &AppStage_Compositor::onProjectConfigMarkedDirty);

	AppStage::exit();
}

void AppStage_Compositor::pause()
{
	AppStage::pause();

	m_frameCompositor->stop();
}

void AppStage_Compositor::resume()
{
	AppStage::resume();

	m_frameCompositor->start();

	hideAllSubWindows();
	m_compositiorOutlinerView->Show();
}

void AppStage_Compositor::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the camera pose for the currently active camera
	updateCamera();

	// Update objects in the object system
	m_ownerWindow->getObjectSystemManager()->update();

	// tick the compositor lua script (if any is active)
	m_scriptContext->updateScript();
}

bool AppStage_Compositor::startStreaming()
{
	if (getIsStreaming())
		return true;

	IMkTextureConstPtr compositorTexture = m_frameCompositor->getCompositedFrameTexture();
	if (compositorTexture == nullptr)
		return false;

	// Compositing buffer should always be RGBA 32BPP
	// Spout can only support RGBA32 and BGRA32
	assert(compositorTexture->getBufferFormat() == GL_RGBA);

	SharedTextureDescriptor sharedTextureDescriptor;
	sharedTextureDescriptor.color_buffer_type = SharedColorBufferType::RGBA32;
	sharedTextureDescriptor.depth_buffer_type = SharedDepthBufferType::NODEPTH;
	sharedTextureDescriptor.width = compositorTexture->getTextureWidth();
	sharedTextureDescriptor.height = compositorTexture->getTextureHeight();
	sharedTextureDescriptor.graphicsAPI = SharedClientGraphicsApi::OpenGL;

	m_renderTargetWriteAccessor->initialize(&sharedTextureDescriptor, true, nullptr);

	// Listen for new frames to write out
	m_frameCompositor->OnNewFrameComposited += MakeDelegate(this, &AppStage_Compositor::onNewStreamingFrameReady);

	return true;
}

bool AppStage_Compositor::getIsStreaming()
{
	return m_renderTargetWriteAccessor->getIsInitialized();
}

void AppStage_Compositor::stopStreaming()
{
	// Stop listening for new frames to write out
	if (getIsStreaming())
	{
		m_frameCompositor->OnNewFrameComposited -= MakeDelegate(this, &AppStage_Compositor::onNewStreamingFrameReady);
	}

	m_renderTargetWriteAccessor->dispose();
}

void AppStage_Compositor::onNewStreamingFrameReady()
{
	EASY_FUNCTION();

	if (getIsStreaming())
	{
		IMkTextureConstPtr frameTexture = m_frameCompositor->getCompositedFrameTexture();

		if (frameTexture != nullptr && m_renderTargetWriteAccessor->getIsInitialized())
		{
			GLuint textureId= frameTexture->getGlTextureId();

			m_renderTargetWriteAccessor->writeColorFrameTexture(&textureId);
		}
	}
}

// Project Config Events
void AppStage_Compositor::onProjectConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(ProjectConfig::k_spoutOutputIsStreamingNamePropertyId))
	{
		onSpoutStreamingFlagChanged();
	}
	else if (changedPropertySet.hasPropertyName(ProjectConfig::k_spoutOutputNamePropertyId))
	{
		onSpoutOutputNameChanged();
	}
}

// Spout Streaming Config Events
void AppStage_Compositor::onSpoutOutputNameChanged()
{
	m_renderTargetWriteAccessor->setClientName(m_project->getSpoutOutputName());
}

void AppStage_Compositor::onSpoutStreamingFlagChanged()
{
	const bool bIsStreaming = getIsStreaming();
	const bool bWantsStreaming = m_project->getIsSpoutOutputStreaming();

	if (!bIsStreaming && bWantsStreaming)
	{
		startStreaming();
	}
	else if (bIsStreaming && !bWantsStreaming)
	{
		stopStreaming();
	}
}

// Camera
void AppStage_Compositor::setupCameras()
{
	for (int cameraIndex = 0; cameraIndex < (int)eCompositorViewpointMode::COUNT; ++cameraIndex)
	{
		if (cameraIndex == m_viewport->getCameraCount())
		{
			m_viewport->addCamera();
		}

		// Use fly-cam input control for every camera except for the first one (the XR camera)
		MikanCameraPtr camera= m_viewport->getMikanCameraByIndex(cameraIndex);
		if (cameraIndex == 0)
			camera->setCameraMovementMode(eCameraMovementMode::stationary);
		else
			camera->setCameraMovementMode(eCameraMovementMode::fly);
	}
	
	// Default to the XR Camera view
	setCurrentCameraMode(eCompositorViewpointMode::mixedRealityViewpoint);

	// Bind viewpoint hot keys
	{
		InputManager* inputManager = InputManager::getInstance();

		inputManager->fetchOrAddKeyBindings(SDLK_1)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Compositor::setXRCamera);
		inputManager->fetchOrAddKeyBindings(SDLK_2)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Compositor::setVRCamera);
	}

	// Set camera names
	getViewpointCamera(eCompositorViewpointMode::vrViewpoint)->setName("vrViewpoint");
	getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint)->setName("mixedRealityViewpoint");
}

void AppStage_Compositor::setXRCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		setCurrentCameraMode(eCompositorViewpointMode::mixedRealityViewpoint);
	}
}

void AppStage_Compositor::setVRCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		setCurrentCameraMode(eCompositorViewpointMode::vrViewpoint);
	}
}

void AppStage_Compositor::setCurrentCameraMode(eCompositorViewpointMode viewportMode)
{
	m_viewportMode= viewportMode;
	m_viewport->setCurrentCamera((int)m_viewportMode);
}

eCompositorViewpointMode AppStage_Compositor::getCurrentCameraMode() const
{
	return (eCompositorViewpointMode)m_viewport->getCurrentCameraIndex();
}

MikanCameraPtr AppStage_Compositor::getViewpointCamera(eCompositorViewpointMode viewportMode) const
{
	return m_viewport->getMikanCameraByIndex((int)viewportMode);
}

void AppStage_Compositor::updateCamera()
{
	// Copy the compositor's camera pose to the app stage's camera for debug rendering
	MikanCameraPtr camera = getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);
	if (camera)
	{
		glm::mat4 cameraXform;
		if (m_frameCompositor->getVideoSourceCameraPose(cameraXform))
		{
			camera->setCameraTransform(cameraXform);
		}
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
void AppStage_Compositor::onGraphEditEvent()
{
	App* app= App::getInstance();	

	if (!app->hasWindowOfType<CompositorNodeEditorWindow>())
	{
		app->createAppWindow<CompositorNodeEditorWindow>();
	}
}

void AppStage_Compositor::onGraphFileSelectEvent()
{
	const char* filterItems[1] = {"*.graph"};
	const char* filterDesc = "Graph Files (*.graph)";
	auto path = tinyfd_openFileDialog("Load Compositor Graph", "", 1, filterItems, filterDesc, 1);
	if (path)
	{
		m_frameCompositor->setCompositorGraphAssetPath(path);
	}
}

void AppStage_Compositor::onConfigAddEvent()
{
	m_bAddingNewConfig= true;

	if (m_frameCompositor->addNewPreset())
	{	
		// Get the name of the newly created preset
		const std::string newPresetName = m_frameCompositor->getCurrentPresetName();

		// Rebuild the layers UI to make sure the new layer exists
		m_compositorLayersModel->rebuild(m_frameCompositor);
		getRmlContext()->Update();

		// Force select the new preset (by default RML preserves the current selection)
		Rml::ElementFormControlSelect* select_element= 
			rmlui_dynamic_cast< Rml::ElementFormControlSelect* >(
				m_compositiorLayersView->GetElementById("config_select"));
		if (select_element != nullptr)
		{
			select_element->SetValue(newPresetName);
		}
	}

	m_bAddingNewConfig= false;
}

void AppStage_Compositor::onConfigDeleteEvent()
{
	CompositorPresetConstPtr preset= m_frameCompositor->getCurrentPresetConfig();
	if (preset == nullptr)
		return;

	char szQuestion[512];
	StringUtils::formatString(
		szQuestion, sizeof(szQuestion), 
		"Are you sure you want to delete config \'%s\'", 
		preset->name.c_str());

	ModalDialog_Confirm::confirmQuestion(
		"Delete Config", szQuestion,
		[this]() {
			if (m_frameCompositor->deleteCurrentPreset())
			{
				m_compositorLayersModel->rebuild(m_frameCompositor);
			}
		});
}

void AppStage_Compositor::onConfigNameChangeEvent(const std::string& newConfigName)
{
	if (m_frameCompositor->setCurrentPresetName(newConfigName))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onConfigSelectEvent(const std::string& configName)
{
	// Ignore this UI event if we are in the middle of adding a new config
	if (m_bAddingNewConfig)
		return;

	m_frameCompositor->selectPreset(configName);
}

void AppStage_Compositor::onScreenshotClientSourceEvent(const std::string& clientSourceName)
{
	const NamedValueTable<GlFrameCompositor::ClientSource*>& clientSources = m_frameCompositor->getClientSources();

	GlFrameCompositor::ClientSource* clientSource= nullptr;
	if (clientSources.tryGetValue(clientSourceName, clientSource))
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

void AppStage_Compositor::render()
{
	MikanCameraPtr currentCamera= m_viewport->getCurrentMikanCamera();

	switch (getCurrentCameraMode())
	{
	case eCompositorViewpointMode::mixedRealityViewpoint:
		{
			// Render the video frame + composited frame buffers
			m_frameCompositor->render();

			// Render the editor scene
			EditorObjectSystem::getSystem()->getEditorScene()->render(
				currentCamera, 
				m_ownerWindow->getMkStateStack());

			// Perform component custom rendering
			m_ownerWindow->getObjectSystemManager()->customRender();
		}
		break;
	case eCompositorViewpointMode::vrViewpoint:
		{
			// Render the editor scene
			EditorObjectSystem::getSystem()->getEditorScene()->render(
				currentCamera,
				m_ownerWindow->getMkStateStack());

			// Perform component custom rendering
			m_ownerWindow->getObjectSystemManager()->customRender();

			// Draw the mouse cursor ray from the pov of the xr camera
			MikanCameraPtr xrCamera = getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);
			if (xrCamera)
			{
				const glm::mat4 glmCameraXform= xrCamera->getCameraTransformFromViewMatrix();

				// Draw the frustum for the initial camera pose
				const float hfov_radians = degrees_to_radians(xrCamera->getHorizontalFOVDegrees());
				const float vfov_radians = degrees_to_radians(xrCamera->getVerticalFOVDegrees());
				const float zNear = fmaxf(xrCamera->getZNear(), 0.1f);
				const float zFar = fminf(xrCamera->getZFar(), 2.0f);

				drawTransformedFrustum(
					glmCameraXform,
					hfov_radians, vfov_radians,
					zNear, zFar,
					Colors::Yellow);
				drawTransformedAxes(glmCameraXform, 0.1f);
			}

			// Draw tracking space
			drawGrid(glm::mat4(1.f), 10.f, 10.f, 20, 20, Colors::GhostWhite);
			if (m_project->getRenderOriginFlag())
			{
				debugRenderOrigin();
			}
		}
		break;
	}
}

void AppStage_Compositor::debugRenderOrigin() const
{
	TextStyle style = getDefaultTextStyle();

	drawTransformedAxes(glm::mat4(1.f), 1.f, 1.f, 1.f);
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
}