///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"
#include "Compositor/RmlModel_CompositorLayers.h"
#include "Compositor/RmlModel_CompositorOutliner.h"
#include "Compositor/RmlModel_CompositorRecording.h"
#include "Compositor/RmlModel_CompositorScripting.h"
#include "Compositor/RmlModel_CompositorSelection.h"
#include "Compositor/RmlModel_CompositorSettings.h"
#include "EditorObjectSystem.h"
#include "FileBrowser/ModalDialog_FileBrowser.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "Colors.h"
#include "CompositorScriptContext.h"
#include "EditorObjectSystem.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlFrameCompositor.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "GlRenderModelResource.h"
#include "GlViewport.h"
#include "GlWireframeMesh.h"
#include "GlTexture.h"
#include "InterprocessRenderTargetWriter.h"
#include "InputManager.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "MikanObjectSystem.h"
#include "MathTypeConversion.h"
#include "MathMikan.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "MikanScene.h"
#include "ObjectSystemManager.h"
#include "ProfileConfig.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "SdlUtility.h"
#include "SceneComponent.h"
#include "StringUtils.h"
#include "StencilObjectSystem.h"
#include "TextStyle.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoWriter.h"
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
	, m_compositorRecordingModel(new RmlModel_CompositorRecording)
	, m_compositorScriptingModel(new RmlModel_CompositorScripting)
	, m_compositorOutlinerModel(new RmlModel_CompositorOutliner)
	, m_compositorSelectionModel(new RmlModel_CompositorSelection)
	, m_compositorSettingsModel(new RmlModel_CompositorSettings)
	, m_scriptContext(std::make_shared<CompositorScriptContext>())
	, m_videoWriter(new VideoWriter)
	, m_renderTargetWriteAccessor(new InterprocessRenderTargetWriteAccessor("MikanXR"))
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	m_viewport= nullptr;

	delete m_compositorModel;
	delete m_compositorLayersModel;
	delete m_compositorRecordingModel;
	delete m_compositorScriptingModel;
	delete m_compositorOutlinerModel;
	delete m_compositorSelectionModel;
	delete m_compositorSettingsModel;
	m_scriptContext.reset();
	delete m_videoWriter;
	delete m_renderTargetWriteAccessor;
}

void AppStage_Compositor::enter()
{
	AppStage::enter();

	// Cache object systems we'll be accessing
	ObjectSystemManagerPtr objectSystemManager = m_ownerWindow->getObjectSystemManager();
	m_anchorObjectSystem = objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_stencilObjectSystem = objectSystemManager->getSystemOfType<StencilObjectSystem>();

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

		for (GlViewportPtr viewport : getViewportList())
		{
			GlCameraPtr camera= getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);

			camera->applyMonoCameraIntrinsics(&cameraIntrinsics);
		}
	}

	// Register the script context with the mikan server
	MikanServer::getInstance()->bindScriptContect(m_scriptContext);

	// Load the compositor script
	m_profile = App::getInstance()->getProfileConfig();
	if (!m_profile->compositorScriptFilePath.empty())
	{
		if (!m_scriptContext->loadScript(m_profile->compositorScriptFilePath))
		{
			m_profile->compositorScriptFilePath = "";
			m_profile->save();
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
		m_compositorModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingWindowEvent);
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

		// Init Recording UI
		m_compositorRecordingModel->init(context, m_frameCompositor);
		m_compositorRecordingModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingEvent);
		m_compositorRecordingModel->OnToggleStreamingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleStreamingEvent);
		m_compositiorRecordingView = addRmlDocument("compositor_recording.rml");
		m_compositiorRecordingView->Hide();

		// Init Scripting UI
		m_compositorScriptingModel->init(context, m_profile, m_scriptContext);
		m_compositorScriptingModel->OnScriptFileChangeEvent = MakeDelegate(this, &AppStage_Compositor::onScriptFileChangeEvent);
		m_compositorScriptingModel->OnSelectCompositorScriptFileEvent = MakeDelegate(this, &AppStage_Compositor::onSelectCompositorScriptFileEvent);
		m_compositorScriptingModel->OnReloadCompositorScriptFileEvent = MakeDelegate(this, &AppStage_Compositor::onReloadCompositorScriptFileEvent);
		m_compositorScriptingModel->OnInvokeScriptTriggerEvent = MakeDelegate(this, &AppStage_Compositor::onInvokeScriptTriggerEvent);
		m_compositiorScriptingView = addRmlDocument("compositor_scripting.rml");
		m_compositiorScriptingView->Hide();

		// Init Settings UI
		m_compositorSettingsModel->init(context, m_profile);
		m_compositiorSettingsView = addRmlDocument("compositor_settings.rml");
		m_compositiorSettingsView->Hide();
	}
}

void AppStage_Compositor::exit()
{
	// Unregister all viewports from the editor
	App* app= App::getInstance();
	EditorObjectSystemPtr editorSystem = m_ownerWindow->getObjectSystemManager()->getSystemOfType<EditorObjectSystem>();
	editorSystem->clearViewports();

	// Unregister the script context with the mikan server
	MikanServer::getInstance()->unbindScriptContect(m_scriptContext);

	stopRecording();
	stopStreaming();

	m_compositorSelectionModel->dispose();
	m_compositorOutlinerModel->dispose();
	m_compositorLayersModel->dispose();
	m_compositorRecordingModel->dispose();
	m_compositorScriptingModel->dispose();
	m_compositorModel->dispose();
	m_compositorSettingsModel->dispose();

	m_frameCompositor->stop();

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

bool AppStage_Compositor::startRecording()
{
	stopRecording();

	VideoSourceViewPtr videoSource= m_frameCompositor->getVideoSource();
	if (!videoSource)
		return false;

	GlTexturePtr compositorTexture= m_frameCompositor->getBGRVideoFrameTexture();
	if (compositorTexture == nullptr)
		return false;

	unsigned int matType= 0;
	unsigned int bpp= 0;
	switch (compositorTexture->getBufferFormat())
	{
	case GL_RGB:
	case GL_BGR:
		matType = CV_8UC3;
		bpp = 24;
		break;
	case GL_RGBA:
	case GL_BGRA:
		matType = CV_8UC4;
		bpp = 32;
		break;
	default:
		break;
	}

	if (matType == 0)
		return false;
	
	const eSupportedCodec selectedCodex= m_compositorRecordingModel->getSelectedVideoCodec();
	const std::string suffix= k_supportedCodecFileSuffix[(int)selectedCodex];
	const int fourcc = k_supportedCodecFourCC[(int)selectedCodex];
	const std::filesystem::path outputFile = m_profile->generateTimestampedFilePath("video", suffix);
	const int width = compositorTexture->getTextureWidth();
	const int height = compositorTexture->getTextureHeight();
	const cv::Size size(width, height);
	//TODO: $HACK - Force write out 30fps
	const double fps = 30.0; //videoSource->getFrameRate();

	m_videoWriter->open(outputFile, fps, width, height, bpp);

	// Tell the frame compositor to also generate BGR video frames
	m_frameCompositor->setGenerateBGRVideoTexture(true);

	// Listen for new frames to write out
	m_frameCompositor->OnNewFrameComposited+= MakeDelegate(this, &AppStage_Compositor::onNewRecordingFrameReady);
	m_compositorRecordingModel->setIsRecording(true);

	return true;
}

void AppStage_Compositor::stopRecording()
{
	// Stop listening for new frames to write out
	if (m_compositorRecordingModel->getIsRecording())
	{
		m_frameCompositor->setGenerateBGRVideoTexture(false);
		m_frameCompositor->OnNewFrameComposited -= MakeDelegate(this, &AppStage_Compositor::onNewRecordingFrameReady);
	}

	m_videoWriter->close();
	m_compositorRecordingModel->setIsRecording(false);
}

void AppStage_Compositor::onNewRecordingFrameReady()
{
	EASY_FUNCTION();

	if (m_compositorRecordingModel->getIsRecording())
	{		
		GlTexturePtr bgrTexture = m_frameCompositor->getBGRVideoFrameTexture();

		if (bgrTexture != nullptr && m_videoWriter != nullptr && m_videoWriter->getIsOpened())
		{
			m_videoWriter->write(bgrTexture.get());
		}
	}
}

bool AppStage_Compositor::startStreaming()
{
	if (m_compositorRecordingModel->getIsStreaming())
		return true;

	GlTextureConstPtr compositorTexture = m_frameCompositor->getCompositedFrameTexture();
	if (compositorTexture == nullptr)
		return false;

	if (compositorTexture->getBufferFormat() != GL_RGBA)
		return false;

	MikanRenderTargetDescriptor descriptor;
	memset(&descriptor, 0, sizeof(MikanRenderTargetDescriptor));
	descriptor.color_buffer_type= MikanColorBuffer_RGBA32;
	descriptor.depth_buffer_type= MikanDepthBuffer_NODEPTH;
	descriptor.width= compositorTexture->getTextureWidth();
	descriptor.height= compositorTexture->getTextureHeight();
	descriptor.graphicsAPI= MikanClientGraphicsApi_OpenGL;

	m_renderTargetWriteAccessor->initialize(&descriptor, true, nullptr);

	// Listen for new frames to write out
	m_frameCompositor->OnNewFrameComposited += MakeDelegate(this, &AppStage_Compositor::onNewStreamingFrameReady);
	m_compositorRecordingModel->setIsStreaming(true);

	return true;
}

void AppStage_Compositor::stopStreaming()
{
	// Stop listening for new frames to write out
	if (m_compositorRecordingModel->getIsStreaming())
	{
		m_frameCompositor->OnNewFrameComposited -= MakeDelegate(this, &AppStage_Compositor::onNewStreamingFrameReady);
	}

	m_renderTargetWriteAccessor->dispose();
	m_compositorRecordingModel->setIsStreaming(false);
}

void AppStage_Compositor::onNewStreamingFrameReady()
{
	EASY_FUNCTION();

	if (m_compositorRecordingModel->getIsStreaming())
	{
		GlTextureConstPtr frameTexture = m_frameCompositor->getCompositedFrameTexture();

		if (frameTexture != nullptr && m_renderTargetWriteAccessor->getIsInitialized())
		{
			GLuint textureId= frameTexture->getGlTextureId();

			m_renderTargetWriteAccessor->writeRenderTargetTexture(&textureId);
		}
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
		GlCameraPtr camera= m_viewport->getCameraByIndex(cameraIndex);
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

GlCameraPtr AppStage_Compositor::getViewpointCamera(eCompositorViewpointMode viewportMode) const
{
	return m_viewport->getCameraByIndex((int)viewportMode);
}

void AppStage_Compositor::updateCamera()
{
	// Copy the compositor's camera pose to the app stage's camera for debug rendering
	GlCameraPtr camera = getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);
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

void AppStage_Compositor::onToggleRecordingWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorRecordingView) m_compositiorRecordingView->Show();
}

void AppStage_Compositor::onToggleScriptingWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorScriptingView) m_compositiorScriptingView->Show();
}

void AppStage_Compositor::onToggleSettingsWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorSettingsView) m_compositiorSettingsView->Show();
}

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
			SdlUtility::saveTextureToPNG(clientSource->colorTexture.get(), "layerScreenshot.png");
		}
	}
}

void AppStage_Compositor::hideAllSubWindows()
{
	if (m_compositiorOutlinerView) m_compositiorOutlinerView->Hide();
	if (m_compositiorLayersView) m_compositiorLayersView->Hide();
	if (m_compositiorRecordingView) m_compositiorRecordingView->Hide();
	if (m_compositiorScriptingView) m_compositiorScriptingView->Hide();
	if (m_compositiorSettingsView) m_compositiorSettingsView->Hide();
}

// Recording UI Events
void AppStage_Compositor::onToggleRecordingEvent()
{
	if (m_compositorRecordingModel->getIsRecording())
		stopRecording();
	else
		startRecording();
}

void AppStage_Compositor::onToggleStreamingEvent()
{
	if (m_compositorRecordingModel->getIsStreaming())
		stopStreaming();
	else
		startStreaming();
}

// Scripting UI Events
void AppStage_Compositor::onScriptFileChangeEvent(
	const std::filesystem::path& filepath)
{
	if (m_scriptContext->loadScript(filepath))
	{
		m_profile->compositorScriptFilePath = filepath;
		m_profile->save();

		m_compositorScriptingModel->setCompositorScriptPath(filepath);
	}
}

void AppStage_Compositor::onSelectCompositorScriptFileEvent()
{
	std::filesystem::path current_dir;
	std::filesystem::path current_file;

	if (!m_profile->compositorScriptFilePath.empty())
	{
		current_file = m_profile->compositorScriptFilePath;
		current_dir = current_file.remove_filename();
	}

	ModalDialog_FileBrowser::browseFile(
		"Select Scene Script",
		current_dir,
		current_file,
		{".lua"},
		[this](const std::filesystem::path& filepath) {
			onScriptFileChangeEvent(filepath);
		});
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
	GlCameraPtr currentCamera= m_viewport->getCurrentCamera();

	switch (getCurrentCameraMode())
	{
	case eCompositorViewpointMode::mixedRealityViewpoint:
		{
			// Render the video frame + composited frame buffers
			m_frameCompositor->render();

			// Render the editor scene
			EditorObjectSystem::getSystem()->getEditorScene()->render(currentCamera);

			// Perform component custom rendering
			m_ownerWindow->getObjectSystemManager()->customRender();
		}
		break;
	case eCompositorViewpointMode::vrViewpoint:
		{
			// Render the editor scene
			EditorObjectSystem::getSystem()->getEditorScene()->render(currentCamera);

			// Perform component custom rendering
			m_ownerWindow->getObjectSystemManager()->customRender();

			// Draw the mouse cursor ray from the pov of the xr camera
			GlCameraPtr xrCamera = getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);
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
			if (m_profile->getRenderOriginFlag())
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