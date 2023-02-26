///-- includes -----
#include "App.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"
#include "Compositor/RmlModel_CompositorLayers.h"
#include "Compositor/RmlModel_CompositorBoxes.h"
#include "Compositor/RmlModel_CompositorQuads.h"
#include "Compositor/RmlModel_CompositorModels.h"
#include "Compositor/RmlModel_CompositorRecording.h"
#include "Compositor/RmlModel_CompositorScripting.h"
#include "Compositor/RmlModel_CompositorSources.h"
#include "FileBrowser/ModalDialog_FileBrowser.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "Colors.h"
#include "CompositorScriptContext.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlFrameCompositor.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "GlRenderModelResource.h"
#include "GlWireframeMesh.h"
#include "GlTexture.h"
#include "MathTypeConversion.h"
#include "MathMikan.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "PathUtils.h"
#include "RmlUtility.h"
#include "StringUtils.h"
#include "TextStyle.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoWriter.h"

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
	, m_compositorQuadsModel(new RmlModel_CompositorQuads)
	, m_compositorBoxesModel(new RmlModel_CompositorBoxes)
	, m_compositorModelsModel(new RmlModel_CompositorModels)
	, m_compositorRecordingModel(new RmlModel_CompositorRecording)
	, m_compositorScriptingModel(new RmlModel_CompositorScripting)
	, m_compositorSourcesModel(new RmlModel_CompositorSources)
	, m_scriptContext(new CompositorScriptContext)
	, m_videoWriter(new VideoWriter)
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	delete m_compositorModel;
	delete m_compositorLayersModel;
	delete m_compositorQuadsModel;
	delete m_compositorBoxesModel;
	delete m_compositorModelsModel;
	delete m_compositorRecordingModel;
	delete m_compositorScriptingModel;
	delete m_compositorSourcesModel;
	delete m_scriptContext;
	delete m_videoWriter;
}

void AppStage_Compositor::enter()
{
	AppStage::enter();

	m_camera= Renderer::getInstance()->pushCamera();

	m_frameCompositor= GlFrameCompositor::getInstance();
	m_frameCompositor->start();
	m_frameCompositor->OnCompositorShadersReloaded += MakeDelegate(this, &AppStage_Compositor::onCompositorShadersReloaded);

	// Apply video source camera intrinsics to the camera
	VideoSourceViewPtr videoSourceView = m_frameCompositor->getVideoSource();
	if (videoSourceView != nullptr)
	{
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);

		m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);
	}

	// Load the compositor script
	m_profile = App::getInstance()->getProfileConfig();
	if (!m_profile->compositorScript.empty())
	{
		if (!m_scriptContext->loadScript(m_profile->compositorScript))
		{
			m_profile->compositorScript = "";
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
		m_compositorModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingWindowEvent);
		m_compositorModel->OnToggleScriptingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleScriptingWindowEvent);
		m_compositorModel->OnToggleQuadStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleQuadStencilsWindowEvent);
		m_compositorModel->OnToggleBoxStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleBoxStencilsWindowEvent);
		m_compositorModel->OnToggleModelStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleModelStencilsWindowEvent);
		m_compositorModel->OnToggleSourcesEvent = MakeDelegate(this, &AppStage_Compositor::onToggleSourcesWindowEvent);
		m_compositiorView = addRmlDocument("rml\\compositor.rml");

		// Init Layers UI
		m_compositorLayersModel->init(context, m_frameCompositor, m_profile);
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
		m_compositiorLayersView = addRmlDocument("rml\\compositor_layers.rml");
		m_compositiorLayersView->Show();

		// Init Quad Stencils UI
		m_compositorQuadsModel->init(context, m_profile);
		m_compositorQuadsModel->OnAddQuadStencilEvent = MakeDelegate(this, &AppStage_Compositor::onAddQuadStencilEvent);
		m_compositorQuadsModel->OnDeleteQuadStencilEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteQuadStencilEvent);
		m_compositorQuadsModel->OnModifyQuadStencilEvent = MakeDelegate(this, &AppStage_Compositor::onModifyQuadStencilEvent);
		m_compositiorQuadsView = addRmlDocument("rml\\compositor_quads.rml");
		m_compositiorQuadsView->Hide();

		// Init Box Stencils UI
		m_compositorBoxesModel->init(context, m_profile);
		m_compositorBoxesModel->OnAddBoxStencilEvent = MakeDelegate(this, &AppStage_Compositor::onAddBoxStencilEvent);
		m_compositorBoxesModel->OnDeleteBoxStencilEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteBoxStencilEvent);
		m_compositorBoxesModel->OnModifyBoxStencilEvent = MakeDelegate(this, &AppStage_Compositor::onModifyBoxStencilEvent);
		m_compositiorBoxesView = addRmlDocument("rml\\compositor_boxes.rml");
		m_compositiorBoxesView->Hide();

		// Init Models Stencils UI
		m_compositorModelsModel->init(context, m_profile);
		m_compositorModelsModel->OnAddModelStencilEvent = MakeDelegate(this, &AppStage_Compositor::onAddModelStencilEvent);
		m_compositorModelsModel->OnDeleteModelStencilEvent = MakeDelegate(this, &AppStage_Compositor::onDeleteModelStencilEvent);
		m_compositorModelsModel->OnModifyModelStencilEvent = MakeDelegate(this, &AppStage_Compositor::onModifyModelStencilEvent);
		m_compositorModelsModel->OnSelectModelStencilPathEvent = MakeDelegate(this, &AppStage_Compositor::onSelectModelStencilPathEvent);
		m_compositiorModelsView = addRmlDocument("rml\\compositor_models.rml");
		m_compositiorModelsView->Hide();

		// Init Recording UI
		m_compositorRecordingModel->init(context, m_frameCompositor);
		m_compositorRecordingModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingEvent);
		m_compositiorRecordingView = addRmlDocument("rml\\compositor_recording.rml");
		m_compositiorRecordingView->Hide();

		// Init Scripting UI
		m_compositorScriptingModel->init(context, m_profile, m_scriptContext);
		m_compositorScriptingModel->OnSelectCompositorScriptFileEvent = MakeDelegate(this, &AppStage_Compositor::onSelectCompositorScriptFileEvent);
		m_compositorScriptingModel->OnReloadCompositorScriptFileEvent = MakeDelegate(this, &AppStage_Compositor::onReloadCompositorScriptFileEvent);
		m_compositorScriptingModel->OnInvokeScriptTriggerEvent = MakeDelegate(this, &AppStage_Compositor::onInvokeScriptTriggerEvent);
		m_compositiorScriptingView = addRmlDocument("rml\\compositor_scripting.rml");
		m_compositiorScriptingView->Hide();

		// Init Sources UI
		m_compositorSourcesModel->init(context, m_frameCompositor);
		m_compositorSourcesModel->OnScreenshotClientSourceEvent = MakeDelegate(this, &AppStage_Compositor::onScreenshotClientSourceEvent);
		m_compositiorSourcesView = addRmlDocument("rml\\compositor_sources.rml");
		m_compositiorSourcesView->Hide();
	}
}

void AppStage_Compositor::exit()
{
	m_frameCompositor->OnCompositorShadersReloaded -= MakeDelegate(this, &AppStage_Compositor::onCompositorShadersReloaded);

	m_compositorLayersModel->dispose();
	m_compositorBoxesModel->dispose();
	m_compositorModelsModel->dispose();
	m_compositorQuadsModel->dispose();
	m_compositorRecordingModel->dispose();
	m_compositorScriptingModel->dispose();
	m_compositorModel->dispose();
	m_compositorSourcesModel->dispose();

	m_frameCompositor->stop();
	App::getInstance()->getRenderer()->popCamera();

	AppStage::exit();
}

void AppStage_Compositor::update()
{
	AppStage::update();

	// Copy the compositor's camera pose to the app stage's camera for debug rendering
	glm::mat4 cameraXform;
	if (m_frameCompositor->getVideoSourceCameraPose(cameraXform))
	{
		m_camera->setCameraPose(cameraXform);
	}

	// tick the compositor lua script (if any is active)
	m_scriptContext->updateScript();

	// Update the sources model now that the app stage has updated
	m_compositorLayersModel->update();
	m_compositorSourcesModel->update();
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
	const std::string outputFile = m_profile->generateTimestampedFilePath("video", suffix);
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
	m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
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
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
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
				m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
			}
		});
}

void AppStage_Compositor::onConfigNameChangeEvent(const std::string& newConfigName)
{
	if (m_frameCompositor->setCurrentPresetName(newConfigName))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
	}
}

void AppStage_Compositor::onLayerAddEvent()
{
	if (m_frameCompositor->addLayerToCurrentPreset())
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
	}
}

void AppStage_Compositor::onLayerDeleteEvent(const int layerIndex)
{
	if (m_frameCompositor->removeLayerFromCurrentPreset(layerIndex))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
	}
}

void AppStage_Compositor::onConfigSelectEvent(const std::string& configName)
{
	// Ignore this UI event if we are in the middle of adding a new config
	if (m_bAddingNewConfig)
		return;

	if (m_frameCompositor->selectPreset(configName))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
	}
}

void AppStage_Compositor::onMaterialNameChangeEvent(
	const int layerIndex, 
	const std::string& materialName)
{
	if (m_frameCompositor->setLayerMaterialName(layerIndex, materialName))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
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
	eStencilType stencilType= m_profile->getStencilType(stencilId);
	if (m_frameCompositor->addLayerStencilRef(layerIndex, stencilType, stencilId))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
	}
}

void AppStage_Compositor::onStencilRefRemovedEvent(const int layerIndex, int stencilId)
{
	eStencilType stencilType = m_profile->getStencilType(stencilId);
	if (m_frameCompositor->removeLayerStencilRef(layerIndex, stencilType, stencilId))
	{
		m_compositorLayersModel->rebuild(m_frameCompositor, m_profile);
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
	if (m_compositiorQuadsView) m_compositiorQuadsView->Hide();
	if (m_compositiorBoxesView) m_compositiorBoxesView->Hide();
	if (m_compositiorModelsView) m_compositiorModelsView->Hide();
	if (m_compositiorRecordingView) m_compositiorRecordingView->Hide();
	if (m_compositiorScriptingView) m_compositiorScriptingView->Hide();
	if (m_compositiorSourcesView) m_compositiorSourcesView->Hide();
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

	if (m_profile->addNewQuadStencil(quad) != INVALID_MIKAN_ID)
	{
		m_compositorQuadsModel->rebuildUIQuadsFromProfile(m_profile);
	}
}

void AppStage_Compositor::onDeleteQuadStencilEvent(int stencilID)
{
	if (m_profile->removeStencil(stencilID))
	{
		m_compositorQuadsModel->rebuildUIQuadsFromProfile(m_profile);
	}
}

void AppStage_Compositor::onModifyQuadStencilEvent(int stencilID)
{
	m_compositorQuadsModel->copyUIQuadToProfile(stencilID, m_profile);
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

	if (m_profile->addNewBoxStencil(box) != INVALID_MIKAN_ID)
	{
		m_compositorBoxesModel->rebuildUIBoxesFromProfile(m_profile);
	}
}

void AppStage_Compositor::onDeleteBoxStencilEvent(int stencilID)
{
	if (m_profile->removeStencil(stencilID))
	{
		m_compositorBoxesModel->rebuildUIBoxesFromProfile(m_profile);
	}
}

void AppStage_Compositor::onModifyBoxStencilEvent(int stencilID)
{
	m_compositorBoxesModel->copyUIBoxToProfile(stencilID, m_profile);
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

	if (m_profile->addNewModelStencil(model) != INVALID_MIKAN_ID)
	{
		m_compositorModelsModel->rebuildUIModelsFromProfile(m_profile);
	}
}

void AppStage_Compositor::onDeleteModelStencilEvent(int stencilID)
{
	if (m_profile->removeStencil(stencilID))
	{
		m_compositorModelsModel->rebuildUIModelsFromProfile(m_profile);
	}
}

void AppStage_Compositor::onModifyModelStencilEvent(int stencilID)
{
	m_compositorModelsModel->copyUIModelToProfile(stencilID, m_profile);	
}

void AppStage_Compositor::onSelectModelStencilPathEvent(int stencilID)
{
	ModalDialog_FileBrowser::browseFile(
		"Select Stencil Model", 
		PathUtils::getCurrentDirectory(), 
		{"obj"}, 
		[this, stencilID](const std::string& filepath) {
			if (m_profile->updateModelStencilFilename(stencilID, filepath))
			{
				m_frameCompositor->flushStencilRenderModel(stencilID);
				m_compositorModelsModel->rebuildUIModelsFromProfile(m_profile);
			}
		});
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
void AppStage_Compositor::onSelectCompositorScriptFileEvent()
{
	ModalDialog_FileBrowser::browseFile(
		"Select Scene Script",
		PathUtils::getCurrentDirectory(),
		{"lua"},
		[this](const std::string& filepath) {

			if (m_scriptContext->loadScript(filepath))
			{
				m_profile->compositorScript = filepath;
				m_profile->save();

				m_compositorScriptingModel->setCompositorScriptPath(filepath);
			}
		});
}

void AppStage_Compositor::onReloadCompositorScriptFileEvent()
{
	if (m_scriptContext->hasScriptFilename())
	{
		if (m_scriptContext->reloadScript())
		{
			m_compositorScriptingModel->rebuildScriptTriggers(m_scriptContext);
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
	// Render the video frame + composited frame buffers
	m_frameCompositor->render();

	if (m_bDebugRenderOrigin || 
		m_bDebugRenderAnchors ||
		m_bDebugRenderStencils)
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		if (m_bDebugRenderOrigin)
		{
			debugRenderOrigin();
		}

		if (m_bDebugRenderAnchors)
		{
			debugRenderAnchors();
		}

		if (m_bDebugRenderStencils)
		{
			debugRenderStencils();
		}

	}
}

void AppStage_Compositor::debugRenderOrigin() const
{
	TextStyle style = getDefaultTextStyle();

	drawTransformedAxes(glm::mat4(1.f), 1.f, 1.f, 1.f);
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
}

void AppStage_Compositor::debugRenderStencils() const
{
	TextStyle style = getDefaultTextStyle();

	// Get camera properties
	const glm::vec3 cameraPosition = m_camera->getCameraPosition();
	const glm::vec3 cameraForward = m_camera->getCameraForward();


	// Render all stencil quads in view of the tracked camera
	std::vector<const MikanStencilQuad*> quadStencilList;
	MikanServer::getInstance()->getRelevantQuadStencilList(nullptr, cameraPosition, cameraForward, quadStencilList);
	for (const MikanStencilQuad* stencil : quadStencilList)
	{
		if (!stencil->is_disabled && stencil->is_double_sided)
		{
			const glm::mat4 xform = m_profile->getQuadStencilWorldTransform(stencil);
			const glm::vec3 position = glm::vec3(xform[3]);
			
			drawTransformedQuad(xform, stencil->quad_width, stencil->quad_height, Colors::Yellow);
			drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
			drawTextAtWorldPosition(style, position, L"Stencil %d", stencil->stencil_id);
		}
	}

	// Render all stencil boxes in view of the tracked camera
	std::vector<const MikanStencilBox*> boxStencilList;
	MikanServer::getInstance()->getRelevantBoxStencilList(nullptr, cameraPosition, cameraForward, boxStencilList);
	for (const MikanStencilBox* stencil : boxStencilList)
	{
		if (!stencil->is_disabled)
		{
			const glm::mat4 xform = m_profile->getBoxStencilWorldTransform(stencil);
			const glm::vec3 half_extents(stencil->box_x_size / 2.f, stencil->box_y_size / 2.f, stencil->box_z_size / 2.f);
			const glm::vec3 position = glm::vec3(xform[3]);

			drawTransformedBox(xform, half_extents, Colors::Yellow);
			drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
			drawTextAtWorldPosition(style, position, L"Stencil %d", stencil->stencil_id);
		}
	}

	// Render all relevant stencil models
	std::vector<const MikanStencilModelConfig*> modelStencilList;
	MikanServer::getInstance()->getRelevantModelStencilList(nullptr, modelStencilList);
	for (const MikanStencilModelConfig* stencil : modelStencilList)
	{
		if (!stencil->modelInfo.is_disabled)
		{
			const glm::mat4 xform = m_profile->getModelStencilWorldTransform(&stencil->modelInfo);
			const glm::vec3 position = glm::vec3(xform[3]);

			// Draw the wireframes for the stencil mesh
			const GlRenderModelResource* modelResource= m_frameCompositor->getStencilRenderModel(stencil->modelInfo.stencil_id);
			if (modelResource != nullptr)
			{
				for (int meshIndex = 0; meshIndex < modelResource->getWireframeMeshCount(); ++meshIndex)
				{
					const GlWireframeMesh* mesh= modelResource->getWireframeMesh(meshIndex);

					if (mesh != nullptr)
					{
						drawTransformedWireframeMesh(xform, mesh, Colors::Yellow);
					}
				}
			}

			drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
			drawTextAtWorldPosition(style, position, L"Stencil %d", stencil->modelInfo.stencil_id);
		}
	}
}

void AppStage_Compositor::debugRenderAnchors() const
{
	TextStyle style = getDefaultTextStyle();

	for (const MikanSpatialAnchorInfo& anchor : m_profile->spatialAnchorList)
	{
		wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
		StringUtils::convertMbsToWcs(anchor.anchor_name, wszAnchorName, sizeof(wszAnchorName));
		glm::mat4 anchorXform = MikanMatrix4f_to_glm_mat4(anchor.anchor_xform);
		glm::vec3 anchorPos(anchorXform[3]);

		drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
	}
}