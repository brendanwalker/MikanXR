///-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "FastenerCalibration/AppStage_FastenerCalibration.h"
#include "ModelFastenerCalibration/AppStage_ModelFastenerCalibration.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"
#include "Compositor/RmlModel_CompositorLayers.h"
#include "Compositor/RmlModel_CompositorAnchors.h"
#include "Compositor/RmlModel_CompositorBoxes.h"
#include "Compositor/RmlModel_CompositorQuads.h"
#include "Compositor/RmlModel_CompositorModels.h"
#include "Compositor/RmlModel_CompositorRecording.h"
#include "Compositor/RmlModel_CompositorScripting.h"
#include "Compositor/RmlModel_CompositorSources.h"
#include "EditorObjectSystem.h"
#include "FastenerComponent.h"
#include "FastenerObjectSystem.h"
#include "FileBrowser/ModalDialog_FileBrowser.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "ModalSnap/ModalDialog_Snap.h"
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
#include "InputManager.h"
#include "MathGLM.h"
#include "MathFastener.h"
#include "MikanObjectSystem.h"
#include "MathTypeConversion.h"
#include "MathMikan.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "MikanScene.h"
#include "ModelStencilComponent.h"
#include "ObjectSystemManager.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "Renderer.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "SceneComponent.h"
#include "StringUtils.h"
#include "StencilObjectSystem.h"
#include "TextStyle.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoWriter.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include <RmlUi/Core/Context.h>
#include "RmlUI/Core/ElementDocument.h"
#include "RmlUI/Core/Elements/ElementFormControlSelect.h"

#include <easy/profiler.h>

#include "opencv2/opencv.hpp"

//-- statics ----
const char* AppStage_Compositor::APP_STAGE_NAME = "Compositor";

//-- public methods -----
AppStage_Compositor::AppStage_Compositor(App* app)
	: AppStage(app, AppStage_Compositor::APP_STAGE_NAME)
	, m_compositorModel(new RmlModel_Compositor)
	, m_compositorLayersModel(new RmlModel_CompositorLayers)
	, m_compositorAnchorsModel(new RmlModel_CompositorAnchors)
	, m_compositorQuadsModel(new RmlModel_CompositorQuads)
	, m_compositorBoxesModel(new RmlModel_CompositorBoxes)
	, m_compositorModelsModel(new RmlModel_CompositorModels)
	, m_compositorRecordingModel(new RmlModel_CompositorRecording)
	, m_compositorScriptingModel(new RmlModel_CompositorScripting)
	, m_compositorSourcesModel(new RmlModel_CompositorSources)
	, m_scriptContext(std::make_shared<CompositorScriptContext>())
	, m_videoWriter(new VideoWriter)
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	m_viewport= nullptr;

	delete m_compositorModel;
	delete m_compositorLayersModel;
	delete m_compositorAnchorsModel;
	delete m_compositorQuadsModel;
	delete m_compositorBoxesModel;
	delete m_compositorModelsModel;
	delete m_compositorRecordingModel;
	delete m_compositorScriptingModel;
	delete m_compositorSourcesModel;
	m_scriptContext.reset();
	delete m_videoWriter;
}

void AppStage_Compositor::enter()
{
	AppStage::enter();
	App* app= App::getInstance();

	// Cache object systems we'll be accessing
	ObjectSystemManagerPtr objectSystemManager = app->getObjectSystemManager();
	m_anchorObjectSystem = objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_fastenerObjectSystem = objectSystemManager->getSystemOfType<FastenerObjectSystem>();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_stencilObjectSystem = objectSystemManager->getSystemOfType<StencilObjectSystem>();

	// Start the frame compositor
	m_frameCompositor= GlFrameCompositor::getInstance();
	m_frameCompositor->start();
	m_frameCompositor->OnCompositorShadersReloaded += MakeDelegate(this, &AppStage_Compositor::onCompositorShadersReloaded);

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

	// Load the compositor script
	m_profile = app->getProfileConfig();
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
		m_compositorModel->OnToggleLayersEvent = MakeDelegate(this, &AppStage_Compositor::onToggleLayersWindowEvent);
		m_compositorModel->OnToggleAnchorsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleAnchorsWindowEvent);
		m_compositorModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingWindowEvent);
		m_compositorModel->OnToggleScriptingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleScriptingWindowEvent);
		m_compositorModel->OnToggleQuadStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleQuadStencilsWindowEvent);
		m_compositorModel->OnToggleBoxStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleBoxStencilsWindowEvent);
		m_compositorModel->OnToggleModelStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleModelStencilsWindowEvent);
		m_compositorModel->OnToggleSourcesEvent = MakeDelegate(this, &AppStage_Compositor::onToggleSourcesWindowEvent);
		m_compositiorView = addRmlDocument("compositor.rml");

		// Init Layers UI
		m_compositorLayersModel->init(context, m_frameCompositor);
		m_compositorLayersModel->OnConfigAddEvent = MakeDelegate(this, &AppStage_Compositor::onConfigAddEvent);
		m_compositorLayersModel->OnConfigDeleteEvent = MakeDelegate(this, &AppStage_Compositor::onConfigDeleteEvent);
		m_compositorLayersModel->OnConfigNameChangeEvent = MakeDelegate(this, &AppStage_Compositor::onConfigNameChangeEvent);
		m_compositorLayersModel->OnConfigSelectEvent = MakeDelegate(this, &AppStage_Compositor::onConfigSelectEvent);
		m_compositorLayersModel->OnLayerAddEvent = MakeDelegate(this, &AppStage_Compositor::onLayerAddEvent);
		m_compositorLayersModel->OnLayerDeleteEvent = MakeDelegate(this, &AppStage_Compositor::onLayerDeleteEvent);
		m_compositorLayersModel->OnMaterialNameChangeEvent = MakeDelegate(this, &AppStage_Compositor::onMaterialNameChangeEvent);
		m_compositorLayersModel->OnVerticalFlipChangeEvent = MakeDelegate(this, &AppStage_Compositor::onVerticalFlipChangedEvent);
		m_compositorLayersModel->OnBlendModeChangeEvent = MakeDelegate(this, &AppStage_Compositor::onBlendModeChangedEvent);
		m_compositorLayersModel->OnInvertQuadsFlagChangeEvent = MakeDelegate(this, &AppStage_Compositor::onInvertQuadsFlagChangeEvent);
		m_compositorLayersModel->OnQuadStencilModeChangeEvent = MakeDelegate(this, &AppStage_Compositor::onQuadStencilModeChangeEvent);
		m_compositorLayersModel->OnBoxStencilModeChangeEvent = MakeDelegate(this, &AppStage_Compositor::onBoxStencilModeChangeEvent);
		m_compositorLayersModel->OnModelStencilModeChangeEvent = MakeDelegate(this, &AppStage_Compositor::onModelStencilModeChangeEvent);
		m_compositorLayersModel->OnStencilRefAddedEvent = MakeDelegate(this, &AppStage_Compositor::onStencilRefAddedEvent);
		m_compositorLayersModel->OnStencilRefRemovedEvent = MakeDelegate(this, &AppStage_Compositor::onStencilRefRemovedEvent);
		m_compositorLayersModel->OnFloatMappingChangedEvent = MakeDelegate(this, &AppStage_Compositor::onFloatMappingChangedEvent);
		m_compositorLayersModel->OnFloat2MappingChangedEvent = MakeDelegate(this, &AppStage_Compositor::onFloat2MappingChangedEvent);
		m_compositorLayersModel->OnFloat3MappingChangedEvent = MakeDelegate(this, &AppStage_Compositor::onFloat3MappingChangedEvent);
		m_compositorLayersModel->OnFloat4MappingChangedEvent = MakeDelegate(this, &AppStage_Compositor::onFloat4MappingChangedEvent);
		m_compositorLayersModel->OnMat4MappingChangedEvent = MakeDelegate(this, &AppStage_Compositor::onMat4MappingChangedEvent);
		m_compositorLayersModel->OnColorTextureMappingChangedEvent = MakeDelegate(this, &AppStage_Compositor::onColorTextureMappingChangedEvent);
		m_compositiorLayersView = addRmlDocument("compositor_layers.rml");
		m_compositiorLayersView->Show();

		// Init Anchors UI
		m_compositorAnchorsModel->init(context, m_anchorObjectSystem, m_fastenerObjectSystem);
		m_compositorAnchorsModel->OnUpdateOriginPose = MakeDelegate(this, &AppStage_Compositor::onUpdateOriginEvent);
		m_compositorAnchorsModel->OnAddFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onAddAnchorFastenerEvent);
		m_compositorAnchorsModel->OnEditFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onEditAnchorFastenerEvent);
		m_compositorAnchorsModel->OnDeleteFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteAnchorFastenerEvent);
		m_compositiorAnchorsView = addRmlDocument("compositor_anchors.rml");
		m_compositiorAnchorsView->Hide();

		// Init Quad Stencils UI
		m_compositorQuadsModel->init(context, m_anchorObjectSystem, m_stencilObjectSystem);
		m_compositorQuadsModel->OnAddQuadStencilEvent = MakeDelegate(this, &AppStage_Compositor::onAddQuadStencilEvent);
		m_compositorQuadsModel->OnDeleteQuadStencilEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteQuadStencilEvent);
		m_compositiorQuadsView = addRmlDocument("compositor_quads.rml");
		m_compositiorQuadsView->Hide();

		// Init Box Stencils UI
		m_compositorBoxesModel->init(context, m_anchorObjectSystem, m_stencilObjectSystem);
		m_compositorBoxesModel->OnAddBoxStencilEvent = MakeDelegate(this, &AppStage_Compositor::onAddBoxStencilEvent);
		m_compositorBoxesModel->OnDeleteBoxStencilEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteBoxStencilEvent);
		m_compositiorBoxesView = addRmlDocument("compositor_boxes.rml");
		m_compositiorBoxesView->Hide();

		// Init Models Stencils UI
		m_compositorModelsModel->init(context, m_anchorObjectSystem, m_stencilObjectSystem, m_fastenerObjectSystem);
		m_compositorModelsModel->OnAddModelStencilEvent = MakeDelegate(this, &AppStage_Compositor::onAddModelStencilEvent);
		m_compositorModelsModel->OnDeleteModelStencilEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteModelStencilEvent);
		m_compositorModelsModel->OnSnapFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onSnapFastenerEvent);
		m_compositorModelsModel->OnAddFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onAddModelStencilFastenerEvent);
		m_compositorModelsModel->OnEditFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onEditModelStencilFastenerEvent);
		m_compositorModelsModel->OnDeleteFastenerEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteModelStencilFastenerEvent);
		m_compositiorModelsView = addRmlDocument("compositor_models.rml");
		m_compositiorModelsView->Hide();

		// Init Recording UI
		m_compositorRecordingModel->init(context, m_frameCompositor);
		m_compositorRecordingModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingEvent);
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

		// Init Sources UI
		m_compositorSourcesModel->init(context, m_frameCompositor);
		m_compositorSourcesModel->OnScreenshotClientSourceEvent = MakeDelegate(this, &AppStage_Compositor::onScreenshotClientSourceEvent);
		m_compositiorSourcesView = addRmlDocument("compositor_sources.rml");
		m_compositiorSourcesView->Hide();
	}
}

void AppStage_Compositor::exit()
{
	// Unregister all viewports from the editor
	App* app= App::getInstance();
	EditorObjectSystemPtr editorSystem = app->getObjectSystemManager()->getSystemOfType<EditorObjectSystem>();
	editorSystem->clearViewports();

	m_frameCompositor->OnCompositorShadersReloaded -= MakeDelegate(this, &AppStage_Compositor::onCompositorShadersReloaded);

	m_compositorLayersModel->dispose();
	m_compositorAnchorsModel->dispose();
	m_compositorBoxesModel->dispose();
	m_compositorModelsModel->dispose();
	m_compositorQuadsModel->dispose();
	m_compositorRecordingModel->dispose();
	m_compositorScriptingModel->dispose();
	m_compositorModel->dispose();
	m_compositorSourcesModel->dispose();

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
	m_compositiorLayersView->Show();
}

void AppStage_Compositor::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the camera pose for the currently active camera
	updateCamera();

	// Update objects in the object system
	m_app->getObjectSystemManager()->update();

	// tick the compositor lua script (if any is active)
	m_scriptContext->updateScript();

	// Update the sources model now that the app stage has updated
	if (Rml::Utilities::IsElementDocumentVisible(m_compositiorSourcesView))
	{
		m_compositorSourcesModel->update();
	}
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
		matType = CV_8UC3;
		bpp = 24;
		break;
	case GL_RGBA:
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
	m_frameCompositor->OnNewFrameComposited+= MakeDelegate(this, &AppStage_Compositor::onNewFrameComposited);
	m_compositorRecordingModel->setIsRecording(true);

	return true;
}

void AppStage_Compositor::stopRecording()
{
	// Stop listening for new frames to write out
	if (m_compositorRecordingModel->getIsRecording())
	{
		m_frameCompositor->setGenerateBGRVideoTexture(false);
		m_frameCompositor->OnNewFrameComposited -= MakeDelegate(this, &AppStage_Compositor::onNewFrameComposited);
	}

	m_videoWriter->close();
	m_compositorRecordingModel->setIsRecording(false);
}

void AppStage_Compositor::onCompositorShadersReloaded()
{
	m_compositorLayersModel->rebuild(m_frameCompositor);
}

void AppStage_Compositor::onNewFrameComposited()
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

// Camera
void AppStage_Compositor::setupCameras()
{
	m_viewport = getFirstViewport();
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
	setXRCamera();

	// Bind viewpoint hot keys
	{
		InputManager* inputManager = InputManager::getInstance();

		inputManager->fetchOrAddKeyBindings(SDLK_1)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Compositor::setXRCamera);
		inputManager->fetchOrAddKeyBindings(SDLK_2)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Compositor::setVRCamera);
	}
}

void AppStage_Compositor::setXRCamera()
{
	setCurrentCameraMode(eCompositorViewpointMode::mixedRealityViewpoint);
}

void AppStage_Compositor::setVRCamera()
{
	setCurrentCameraMode(eCompositorViewpointMode::vrViewpoint);
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
	switch (getCurrentCameraMode())
	{
		case eCompositorViewpointMode::mixedRealityViewpoint:
			{
				// Copy the compositor's camera pose to the app stage's camera for debug rendering
				glm::mat4 cameraXform;
				if (m_frameCompositor->getVideoSourceCameraPose(cameraXform))
				{
					GlCameraPtr camera = getViewpointCamera(eCompositorViewpointMode::mixedRealityViewpoint);

					camera->setCameraTransform(cameraXform);
				}
			}
			break;
		case eCompositorViewpointMode::vrViewpoint:
			{
				// Nothing to do
			}
			break;
	}
}

// Compositor Model UI Events
void AppStage_Compositor::onReturnEvent()
{
	m_app->popAppState();
}

void AppStage_Compositor::onToggleLayersWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorLayersView) m_compositiorLayersView->Show();
}

void AppStage_Compositor::onToggleAnchorsWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorAnchorsView) m_compositiorAnchorsView->Show();
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

void AppStage_Compositor::onToggleQuadStencilsWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorQuadsView) m_compositiorQuadsView->Show();
}

void AppStage_Compositor::onToggleBoxStencilsWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorBoxesView) m_compositiorBoxesView->Show();
}

void AppStage_Compositor::onToggleModelStencilsWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorModelsView) m_compositiorModelsView->Show();
}

void AppStage_Compositor::onToggleSourcesWindowEvent()
{
	hideAllSubWindows();
	if (m_compositiorSourcesView) m_compositiorSourcesView->Show();
}

// Compositor Layers UI Events
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
	const CompositorPreset* preset= m_frameCompositor->getCurrentPresetConfig();
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

void AppStage_Compositor::onLayerAddEvent()
{
	if (m_frameCompositor->addLayerToCurrentPreset())
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onLayerDeleteEvent(const int layerIndex)
{
	if (m_frameCompositor->removeLayerFromCurrentPreset(layerIndex))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onConfigSelectEvent(const std::string& configName)
{
	// Ignore this UI event if we are in the middle of adding a new config
	if (m_bAddingNewConfig)
		return;

	if (m_frameCompositor->selectPreset(configName))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onMaterialNameChangeEvent(
	const int layerIndex, 
	const std::string& materialName)
{
	if (m_frameCompositor->setLayerMaterialName(layerIndex, materialName))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onVerticalFlipChangedEvent(
	const int layerIndex,
	bool bIsFlipped)
{
	m_frameCompositor->setIsLayerVerticalFlipped(layerIndex, bIsFlipped);
}

void AppStage_Compositor::onBlendModeChangedEvent(
	const int layerIndex, 
	eCompositorBlendMode blendMode)
{
	m_frameCompositor->setLayerBlendMode(layerIndex, blendMode);
}

void AppStage_Compositor::onInvertQuadsFlagChangeEvent(const int layerIndex, bool bInvertFlag)
{
	m_frameCompositor->setInvertQuadsWhenCameraInside(layerIndex, bInvertFlag);
}

void AppStage_Compositor::onQuadStencilModeChangeEvent(const int layerIndex, eCompositorStencilMode stencilMode)
{
	m_frameCompositor->setQuadStencilMode(layerIndex, stencilMode);
}

void AppStage_Compositor::onBoxStencilModeChangeEvent(const int layerIndex, eCompositorStencilMode stencilMode)
{
	m_frameCompositor->setBoxStencilMode(layerIndex, stencilMode);
}

void AppStage_Compositor::onModelStencilModeChangeEvent(const int layerIndex, eCompositorStencilMode stencilMode)
{
	m_frameCompositor->setModelStencilMode(layerIndex, stencilMode);
}

void AppStage_Compositor::onStencilRefAddedEvent(const int layerIndex, int stencilId)
{
	eStencilType stencilType= m_stencilObjectSystem->getStencilType(stencilId);
	if (m_frameCompositor->addLayerStencilRef(layerIndex, stencilType, stencilId))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onStencilRefRemovedEvent(const int layerIndex, int stencilId)
{
	eStencilType stencilType = m_stencilObjectSystem->getStencilType(stencilId);
	if (m_frameCompositor->removeLayerStencilRef(layerIndex, stencilType, stencilId))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onFloatMappingChangedEvent(
	const int layerIndex, 
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	m_frameCompositor->setFloatMapping(layerIndex, uniformName, dataSourceName);
}

void AppStage_Compositor::onFloat2MappingChangedEvent(
	const int layerIndex, 
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	m_frameCompositor->setFloat2Mapping(layerIndex, uniformName, dataSourceName);
}

void AppStage_Compositor::onFloat3MappingChangedEvent(
	const int layerIndex, 
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	m_frameCompositor->setFloat3Mapping(layerIndex, uniformName, dataSourceName);
}

void AppStage_Compositor::onFloat4MappingChangedEvent(
	const int layerIndex, 
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	m_frameCompositor->setFloat4Mapping(layerIndex, uniformName, dataSourceName);
}

void AppStage_Compositor::onMat4MappingChangedEvent(
	const int layerIndex, 
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	m_frameCompositor->setMat4Mapping(layerIndex, uniformName, dataSourceName);
}

void AppStage_Compositor::onColorTextureMappingChangedEvent(
	const int layerIndex,
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	m_frameCompositor->setColorTextureMapping(layerIndex, uniformName, dataSourceName);
}

void AppStage_Compositor::onScreenshotClientSourceEvent(const std::string& clientSourceName)
{
	const NamedValueTable<GlFrameCompositor::ClientSource*>& clientSources = m_frameCompositor->getClientSources();

	GlFrameCompositor::ClientSource* clientSource= nullptr;
	if (clientSources.tryGetValue(clientSourceName, clientSource))
	{		
		if (clientSource->colorTexture != nullptr)
		{
			saveTextureToPNG(clientSource->colorTexture.get(), "layerScreenshot.png");
		}
	}
}

void AppStage_Compositor::hideAllSubWindows()
{
	if (m_compositiorLayersView) m_compositiorLayersView->Hide();
	if (m_compositiorAnchorsView) m_compositiorAnchorsView->Hide();
	if (m_compositiorQuadsView) m_compositiorQuadsView->Hide();
	if (m_compositiorBoxesView) m_compositiorBoxesView->Hide();
	if (m_compositiorModelsView) m_compositiorModelsView->Hide();
	if (m_compositiorRecordingView) m_compositiorRecordingView->Hide();
	if (m_compositiorScriptingView) m_compositiorScriptingView->Hide();
	if (m_compositiorSourcesView) m_compositiorSourcesView->Hide();
}

// Anchors UI Events
void AppStage_Compositor::onUpdateOriginEvent()
{
	VRDeviceViewPtr vrDeviceView =
		VRDeviceListIterator(eDeviceType::VRTracker, m_profile->originVRDevicePath).getCurrent();

	if (vrDeviceView != nullptr)
	{
		AnchorComponentPtr originSpatialAnchor= m_anchorObjectSystem->getOriginSpatialAnchor();
		if (originSpatialAnchor)
		{
			const glm::mat4 devicePose = vrDeviceView->getCalibrationPose();

			glm::mat4 anchorXform= devicePose;
			if (m_profile->originVerticalAlignFlag)
			{
				const glm::vec3 deviceForward = glm_mat4_get_x_axis(devicePose);
				const glm::vec3 devicePosition = glm_mat4_get_position(devicePose);
				const glm::quat yawOnlyOrientation = glm::quatLookAt(deviceForward, glm::vec3(0.f, 1.f, 0.f));

				anchorXform = glm_mat4_from_pose(yawOnlyOrientation, devicePosition);
			}

			// Update origin anchor transform
			originSpatialAnchor->setWorldTransform(anchorXform);
		}
	}
}

void AppStage_Compositor::onAddAnchorFastenerEvent(int parentAnchorId)
{
	MikanSpatialFastenerInfo fastenerInfo;
	memset(&fastenerInfo, 0, sizeof(MikanSpatialFastenerInfo));

	const MikanStencilID nextStencilId= m_stencilObjectSystem->getStencilSystemConfigConst()->nextStencilId;
	StringUtils::formatString(
		fastenerInfo.fastener_name, sizeof(fastenerInfo.fastener_name),
		"Fastener_%d", nextStencilId);
	fastenerInfo.parent_object_type = MikanFastenerParentType_SpatialAnchor;
	fastenerInfo.parent_object_id = parentAnchorId;
	fastenerInfo.fastener_id= INVALID_MIKAN_ID;

	FastenerComponentPtr fastenerComponent= m_fastenerObjectSystem->addNewFastener(fastenerInfo);
	if (fastenerComponent != nullptr)
	{
		// Show Fastener calibration tool
		AppStage_FastenerCalibration* fastenerCalibration = m_app->pushAppStage<AppStage_FastenerCalibration>();
		fastenerCalibration->setTargetFastener(fastenerInfo);
	}
}

void AppStage_Compositor::onEditAnchorFastenerEvent(int fastenerID)
{
	// Show Fastener calibration tool
	MikanSpatialFastenerInfo fastenerInfo;
	FastenerComponentPtr fastenerComponent= m_fastenerObjectSystem->getSpatialFastenerById(fastenerID);
	if (fastenerComponent != nullptr)
	{
		const MikanSpatialFastenerInfo& fastenerInfo= fastenerComponent->getConfig()->getFastenerInfo();

		AppStage_FastenerCalibration* fastenerCalibration = m_app->pushAppStage<AppStage_FastenerCalibration>();
		fastenerCalibration->setTargetFastener(fastenerInfo);
	}
}

void AppStage_Compositor::onDeleteAnchorFastenerEvent(int parentAnchorId, int fastenerID)
{
	if (m_fastenerObjectSystem->removeFastener(fastenerID))
	{
		m_compositorAnchorsModel->rebuildAnchorList();
	}
}

// Quad Stencils UI Events
void AppStage_Compositor::onAddQuadStencilEvent()
{
	MikanStencilQuad quad;
	memset(&quad, 0, sizeof(MikanStencilQuad));

	quad.is_double_sided = true;
	quad.parent_anchor_id = INVALID_MIKAN_ID;
	quad.quad_center = {0.f, 0.f, 0.f};
	quad.quad_x_axis = {1.f, 0.f, 0.f};
	quad.quad_y_axis = {0.f, 1.f, 0.f};
	quad.quad_normal = {0.f, 0.f, 1.f};
	quad.quad_width = 0.25f;
	quad.quad_height = 0.25f;

	if (m_stencilObjectSystem->addNewQuadStencil(quad) != nullptr)
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onDeleteQuadStencilEvent(int stencilID)
{
	if (m_stencilObjectSystem->removeQuadStencil(stencilID))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

// Box Stencils UI Events
void AppStage_Compositor::onAddBoxStencilEvent()
{
	MikanStencilBox box;
	memset(&box, 0, sizeof(MikanStencilBox));

	box.parent_anchor_id = INVALID_MIKAN_ID;
	box.box_center = {0.f, 0.f, 0.f};
	box.box_x_axis = {1.f, 0.f, 0.f};
	box.box_y_axis = {0.f, 1.f, 0.f};
	box.box_z_axis = {0.f, 0.f, 1.f};
	box.box_x_size = 0.25f;
	box.box_y_size = 0.25f;
	box.box_z_size = 0.25f;

	if (m_stencilObjectSystem->addNewBoxStencil(box) != nullptr)
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onDeleteBoxStencilEvent(int stencilID)
{
	if (m_stencilObjectSystem->removeBoxStencil(stencilID))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

// Model Stencils UI Events
void AppStage_Compositor::onAddModelStencilEvent()
{
	MikanStencilModel model;
	memset(&model, 0, sizeof(MikanStencilModel));

	model.is_disabled = false;
	model.parent_anchor_id = INVALID_MIKAN_ID;
	model.model_position = {0.f, 0.f, 0.f};
	model.model_rotator = {0.f, 0.f, 1.f};
	model.model_scale = {1.f, 1.f, 1.f};

	if (m_stencilObjectSystem->addNewModelStencil(model) != nullptr)
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onDeleteModelStencilEvent(int stencilID)
{
	if (m_stencilObjectSystem->removeModelStencil(stencilID))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor);
	}
}

void AppStage_Compositor::onSnapFastenerEvent(int fastenerID)
{
	ModalDialog_Snap::selectSnapTarget(
		fastenerID,
		[this](MikanSpatialFastenerID sourceId, MikanSpatialFastenerID targetId) {

			FastenerComponentPtr sourceFastener= m_fastenerObjectSystem->getSpatialFastenerById(sourceId);
			FastenerComponentPtr targetFastener= m_fastenerObjectSystem->getSpatialFastenerById(targetId);
			if (sourceFastener != nullptr)
			{
				glm::mat4 newStencilXform;
				glm::vec3 newStencilPoints[3];

				if (align_stencil_fastener_to_anchor_fastener(
					sourceFastener, targetFastener,
					newStencilXform, newStencilPoints))
				{
					SceneComponentPtr sourceFastenerSceneComponent=
						sourceFastener->getOwnerObject()->getRootComponent();
					SceneComponentPtr stencilSceneComponent=
						sourceFastenerSceneComponent->getParentComponent();

					if (stencilSceneComponent != nullptr)
					{
						stencilSceneComponent->setRelativeTransform(GlmTransform(newStencilXform));
					}
				}
			}
		});
}

void AppStage_Compositor::onAddModelStencilFastenerEvent(int parentStencilID)
{
	MikanSpatialFastenerInfo fastenerInfo;
	memset(&fastenerInfo, 0, sizeof(MikanSpatialFastenerInfo));

	MikanSpatialFastenerID nextFastenerId= m_fastenerObjectSystem->getFastenerSystemConfigConst()->nextFastenerId;

	StringUtils::formatString(
		fastenerInfo.fastener_name, sizeof(fastenerInfo.fastener_name),
		"Fastener_%d", nextFastenerId);
	fastenerInfo.parent_object_type = MikanFastenerParentType_Stencil;
	fastenerInfo.parent_object_id = parentStencilID;
	fastenerInfo.fastener_id = INVALID_MIKAN_ID;

	if (m_fastenerObjectSystem->getFastenerSystemConfig()->canAddFastener())
	{
		// Show Fastener calibration tool
		AppStage_ModelFastenerCalibration* fastenerCalibration = m_app->pushAppStage<AppStage_ModelFastenerCalibration>();
		fastenerCalibration->setTargetFastener(fastenerInfo);
	}
}

void AppStage_Compositor::onEditModelStencilFastenerEvent(int fastenerID)
{
	// Show Fastener calibration tool
	FastenerComponentPtr fastenerComponent= m_fastenerObjectSystem->getSpatialFastenerById(fastenerID);
	if (fastenerComponent != nullptr)
	{
		const MikanSpatialFastenerInfo& fastenerInfo= fastenerComponent->getConfig()->getFastenerInfo();

		AppStage_ModelFastenerCalibration* fastenerCalibration = m_app->pushAppStage<AppStage_ModelFastenerCalibration>();
		fastenerCalibration->setTargetFastener(fastenerInfo);
	}
}

void AppStage_Compositor::onDeleteModelStencilFastenerEvent(int stencilID, int fastenerID)
{
	m_fastenerObjectSystem->removeFastener(fastenerID);
}

// Recording UI Events
void AppStage_Compositor::onToggleRecordingEvent()
{
	if (m_compositorRecordingModel->getIsRecording())
		stopRecording();
	else
		startRecording();
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
	switch (getCurrentCameraMode())
	{
	case eCompositorViewpointMode::mixedRealityViewpoint:
		{
			// Render the video frame + composited frame buffers
			m_frameCompositor->render();

			// Render the editor scene
			EditorObjectSystem::getSystem()->getEditorScene()->render();

			// Perform component custom rendering
			m_app->getObjectSystemManager()->customRender();
		}
		break;
	case eCompositorViewpointMode::vrViewpoint:
		{
			// Render the editor scene
			EditorObjectSystem::getSystem()->getEditorScene()->render();

			// Perform component custom rendering
			m_app->getObjectSystemManager()->customRender();

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
			debugRenderOrigin();
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