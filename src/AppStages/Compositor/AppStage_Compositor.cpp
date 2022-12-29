///-- includes -----
#include "App.h"
#include "Compositor/AppStage_Compositor.h"
#include "Compositor/RmlModel_Compositor.h"
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
#include "StringUtils.h"
#include "TextStyle.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoWriter.h"

//#include <imgui.h>
//#include <misc/cpp/imgui_stdlib.h>
#include <easy/profiler.h>

//#include "imfilebrowser.h"

#include "opencv2/opencv.hpp"

std::string g_supportedCodecName[SUPPORTED_CODEC_COUNT] = {
	"MP4V",
	"MJPG",
	"RGBA",
};
std::string g_supportedCodecFileSuffix[SUPPORTED_CODEC_COUNT] = {
	".m4v",
	".avi",
	".avi",
};
int g_supportedCodecFourCC[SUPPORTED_CODEC_COUNT] = {	
	cv::VideoWriter::fourcc('m','p','4','v'),
	cv::VideoWriter::fourcc('M','J','P','G'),
	cv::VideoWriter::fourcc('R','G','B','A'),
};

//-- statics ----
const char* AppStage_Compositor::APP_STAGE_NAME = "Compositor";

//-- public methods -----
AppStage_Compositor::AppStage_Compositor(App* app)
	: AppStage(app, AppStage_Compositor::APP_STAGE_NAME)
	, m_compositorModel(new RmlModel_Compositor)
	, m_scriptContext(new CompositorScriptContext)
	, m_videoWriter(new VideoWriter)
	//, m_modelFileDialog(new ImGui::FileBrowser)
	//, m_scriptFileDialog(new ImGui::FileBrowser)
{
}

AppStage_Compositor::~AppStage_Compositor()
{
	delete m_compositorModel;
	delete m_scriptContext;
	delete m_videoWriter;
	//delete m_modelFileDialog;
	//delete m_scriptFileDialog;
}

void AppStage_Compositor::enter()
{
	AppStage::enter();

	m_camera= Renderer::getInstance()->pushCamera();

	m_frameCompositor= GlFrameCompositor::getInstance();
	m_frameCompositor->start();

	//m_modelFileDialog->SetTitle("Select Stencil Model");
	//m_modelFileDialog->SetTypeFilters({".obj"});

	//m_scriptFileDialog->SetTitle("Select Scene Script");
	//m_scriptFileDialog->SetTypeFilters({ ".lua" });

	// Apply video source camera intrinsics to the camera
	VideoSourceViewPtr videoSourceView = m_frameCompositor->getVideoSource();
	if (videoSourceView != nullptr)
	{
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);

		m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);
	}

	// Load the compositor script
	ProfileConfig* profile = App::getInstance()->getProfileConfig();
	if (!profile->compositorScript.empty())
	{
		if (!m_scriptContext->loadScript(profile->compositorScript))
		{
			profile->compositorScript = "";
			profile->save();
		}
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_compositorModel->init(context);
		m_compositorModel->OnReturnEvent = MakeDelegate(this, &AppStage_Compositor::onReturnEvent);
		m_compositorModel->OnToggleLayersEvent = MakeDelegate(this, &AppStage_Compositor::onToggleLayersEvent);
		m_compositorModel->OnToggleRecordingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleRecordingEvent);
		m_compositorModel->OnToggleScriptingEvent = MakeDelegate(this, &AppStage_Compositor::onToggleScriptingEvent);
		m_compositorModel->OnToggleQuadStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleQuadStencilsEvent);
		m_compositorModel->OnToggleBoxStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleBoxStencilsEvent);
		m_compositorModel->OnToggleModelStencilsEvent = MakeDelegate(this, &AppStage_Compositor::onToggleModelStencilsEvent);

		// Init calibration view now that the dependent model has been created
		m_compositiorView = addRmlDocument("rml\\compositor.rml");
	}
}

void AppStage_Compositor::exit()
{
	m_frameCompositor->stop();
	App::getInstance()->getRenderer()->popCamera();

	AppStage::exit();
}

void AppStage_Compositor::update()
{
	// Copy the compositor's camera pose to the app stage's camera for debug rendering
	glm::mat4 cameraXform;
	if (m_frameCompositor->getVideoSourceCameraPose(cameraXform))
	{
		m_camera->setCameraPose(cameraXform);
	}

	// tick the compositor lua script (if any is active)
	m_scriptContext->updateScript();
}

bool AppStage_Compositor::startRecording()
{
	stopRecording();

	VideoSourceViewPtr videoSource= m_frameCompositor->getVideoSource();
	if (!videoSource)
		return false;

	const GlTexture* compositorTexture= m_frameCompositor->getBGRVideoFrameTexture();
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
	
	const std::string suffix= g_supportedCodecFileSuffix[m_videoCodecIndex];
	const int fourcc = g_supportedCodecFourCC[m_videoCodecIndex];
	const std::string outputFile = App::getInstance()->getProfileConfig()->generateTimestampedFilePath("video", suffix);
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
	m_bIsRecording= true;

	return true;
}

void AppStage_Compositor::stopRecording()
{

	// Stop listening for new frames to write out
	if (m_bIsRecording)
	{
		m_frameCompositor->setGenerateBGRVideoTexture(false);
		m_frameCompositor->OnNewFrameComposited -= MakeDelegate(this, &AppStage_Compositor::onNewFrameComposited);
	}

	m_videoWriter->close();

	m_bIsRecording= false;
}

void AppStage_Compositor::onNewFrameComposited()
{
	EASY_FUNCTION();

	if (m_bIsRecording)
	{		
		GlTexture* bgrTexture = m_frameCompositor->getBGRVideoFrameTexture();

		if (bgrTexture != nullptr && m_videoWriter != nullptr && m_videoWriter->getIsOpened())
		{
			m_videoWriter->write(bgrTexture);
		}
	}
}

// Compositor Model UI Events
void AppStage_Compositor::onReturnEvent()
{
	m_app->popAppState();
}

void AppStage_Compositor::onToggleLayersEvent()
{
}

void AppStage_Compositor::onToggleRecordingEvent()
{
}

void AppStage_Compositor::onToggleScriptingEvent()
{
}

void AppStage_Compositor::onToggleQuadStencilsEvent()
{
}

void AppStage_Compositor::onToggleBoxStencilsEvent()
{
}

void AppStage_Compositor::onToggleModelStencilsEvent()
{
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

	ProfileConfig* profile = App::getInstance()->getProfileConfig();
	
	// Render all stencil quads in view of the tracked camera
	std::vector<const MikanStencilQuad*> quadStencilList;
	MikanServer::getInstance()->getRelevantQuadStencilList(cameraPosition, cameraForward, quadStencilList);
	for (const MikanStencilQuad* stencil : quadStencilList)
	{
		if (!stencil->is_disabled && stencil->is_double_sided)
		{
			const glm::mat4 xform = profile->getQuadStencilWorldTransform(stencil);
			const glm::vec3 position = glm::vec3(xform[3]);
			
			drawTransformedQuad(xform, stencil->quad_width, stencil->quad_height, Colors::Yellow);
			drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
			drawTextAtWorldPosition(style, position, L"Stencil %d", stencil->stencil_id);
		}
	}

	// Render all stencil boxes in view of the tracked camera
	std::vector<const MikanStencilBox*> boxStencilList;
	MikanServer::getInstance()->getRelevantBoxStencilList(cameraPosition, cameraForward, boxStencilList);
	for (const MikanStencilBox* stencil : boxStencilList)
	{
		if (!stencil->is_disabled)
		{
			const glm::mat4 xform = profile->getBoxStencilWorldTransform(stencil);
			const glm::vec3 half_extents(stencil->box_x_size / 2.f, stencil->box_y_size / 2.f, stencil->box_z_size / 2.f);
			const glm::vec3 position = glm::vec3(xform[3]);

			drawTransformedBox(xform, half_extents, Colors::Yellow);
			drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
			drawTextAtWorldPosition(style, position, L"Stencil %d", stencil->stencil_id);
		}
	}

	// Render all relevant stencil models
	std::vector<const MikanStencilModelConfig*> modelStencilList;
	MikanServer::getInstance()->getRelevantModelStencilList(modelStencilList);
	for (const MikanStencilModelConfig* stencil : modelStencilList)
	{
		if (!stencil->modelInfo.is_disabled)
		{
			const glm::mat4 xform = profile->getModelStencilWorldTransform(&stencil->modelInfo);
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

	ProfileConfig* profile = App::getInstance()->getProfileConfig();
	for (const MikanSpatialAnchorInfo& anchor : profile->spatialAnchorList)
	{
		wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
		StringUtils::convertMbsToWcs(anchor.anchor_name, wszAnchorName, sizeof(wszAnchorName));
		glm::mat4 anchorXform = MikanMatrix4f_to_glm_mat4(anchor.anchor_xform);
		glm::vec3 anchorPos(anchorXform[3]);

		drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
	}
}

#if 0
void AppStage_Compositor::renderUI()
{
	ProfileConfig* profile = App::getInstance()->getProfileConfig();

	const float k_panel_width = 300.f;
	const float k_panel_height= ImGui::GetIO().DisplaySize.y;
	const char* k_window_title = "Compositor State";
	const ImGuiWindowFlags window_flags = 0;

	m_modelFileDialog->Display();
	m_scriptFileDialog->Display();

	if (m_scriptFileDialog->IsOpened())
	{
		if (m_scriptFileDialog->HasSelected())
		{
			const std::string filename = m_scriptFileDialog->GetSelected().string();

			if (m_scriptContext->loadScript(filename))
			{
				profile->compositorScript = filename;
				profile->save();
			}

			m_scriptFileDialog->Close();
		}
	}
	else if (m_modelFileDialog->IsOpened())
	{
		if (m_modelFileDialog->HasSelected())
		{
			if (m_pendingModelFilenameStencilID != -1)
			{
				const std::string filename= m_modelFileDialog->GetSelected().string();

				if (profile->updateModelStencilFilename(m_pendingModelFilenameStencilID, filename))
				{
					m_frameCompositor->flushStencilRenderModel(m_pendingModelFilenameStencilID);
				}
				m_pendingModelFilenameStencilID= -1;
			}

			m_modelFileDialog->Close();
		}
	}
	else
	{
		ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
		ImGui::SetNextWindowSize(ImVec2(k_panel_width, k_panel_height));
		ImGui::Begin(k_window_title, nullptr, window_flags);

		{
			VideoSourceViewPtr videoSource= m_frameCompositor->getVideoSource();
			if (videoSource)
			{
				ImGui::Text("Video Source: %s", videoSource->getFriendlyName().c_str());
				ImGui::Text("Video Mode: %s", videoSource->getVideoMode()->modeName.c_str());
			}
			else
			{
				ImGui::Text("<No Video Source>");
			}

			if (ImGui::Button(g_supportedCodecName[m_videoCodecIndex].c_str()))
			{
				m_videoCodecIndex = (m_videoCodecIndex + 1) % SUPPORTED_CODEC_COUNT;
			}
			ImGui::SameLine();
			if (m_bIsRecording)
			{
				if (ImGui::Button("Stop Recording"))
				{
					stopRecording();
				}
			}
			else
			{
				if (ImGui::Button("Start Recording"))
				{
					startRecording();
				}
			}
		}

		ImGui::Separator();

		{
			const std::vector<GlFrameCompositor::Layer> layers = m_frameCompositor->getLayers();
			if (layers.size() > 0)
			{
				for (int layerIndex = 0; layerIndex < layers.size(); ++layerIndex)
				{
					const GlFrameCompositor::Layer& layer = layers[layerIndex];

					if (layerIndex > 0)
					{
						ImGui::Separator();
					}

					ImGui::Text("Layer #%d: %s", layerIndex, layer.clientInfo.applicationName);
					std::string alphaMode = m_frameCompositor->getLayerAlphaModeString(layerIndex);
					if (alphaMode.length() > 0 && ImGui::Button(alphaMode.c_str()))
					{
						m_frameCompositor->cycleNextLayerAlphaMode(layerIndex);
					}
					ImGui::SameLine();
					if (layer.colorTexture != nullptr && ImGui::Button("Screenshot"))
					{
						saveTextureToPNG(layer.colorTexture, "layerScreenshot.png");
					}
				}
			}
			else
			{
				ImGui::Text("<No Connected Clients>");
			}
		}

		ImGui::Separator();

		{
			ImGui::PushItemWidth(-100.f);
			ImGui::InputText("Path:", &profile->compositorScript);
			ImGui::PopItemWidth();

			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				m_scriptFileDialog->Open();
			}

			if (m_scriptContext->hasScriptFilename())
			{
				if (ImGui::Button("Reload Script"))
				{
					m_scriptContext->reloadScript();
				}
			}

			if (m_scriptContext->hasLoadedScript())
			{
				for (const std::string& triggerName : m_scriptContext->getScriptTriggers())
				{
					if (ImGui::Button(triggerName.c_str()))
					{
						m_scriptContext->invokeScriptTrigger(triggerName);
					}
				}
			}
		}

		{
			// Quad Stencils
			for (int stencilIndex = 0; stencilIndex < profile->quadStencilList.size(); ++stencilIndex)
			{
				bool bIsStencilStillValid= true;

				MikanStencilQuad stencil = profile->quadStencilList[stencilIndex];
				bool bStencilChanged = false;

				ImGui::Separator();

				ImGui::PushID(stencil.stencil_id);
				ImGui::Text("Quad Stencil #%d", stencil.stencil_id);

				MikanSpatialAnchorInfo spatialAnchor;
				bool bValidAnchor= profile->getSpatialAnchorInfo(stencil.parent_anchor_id, spatialAnchor);
				const char *anchorName= bValidAnchor ? spatialAnchor.anchor_name : "No Anchor";
				if (ImGui::Button(anchorName))
				{
					stencil.parent_anchor_id= profile->getNextSpatialAnchorId(stencil.parent_anchor_id);
					bStencilChanged = true;
				}

				float position[3] = { stencil.quad_center.x, stencil.quad_center.y, stencil.quad_center.z };
				if (ImGui::InputFloat3("Position", position, "%.2f"))
				{
					stencil.quad_center.x = position[0];
					stencil.quad_center.y = position[1];
					stencil.quad_center.z = position[2];
					bStencilChanged= true;
				}
				
				float angles[3];
				MikanOrientationToEulerAngles(
					stencil.quad_x_axis, stencil.quad_y_axis, stencil.quad_normal,
					angles[0], angles[1], angles[2]);
				if (ImGui::InputFloat3("Rotation", angles, "%.1f"))
				{
					EulerAnglesToMikanOrientation(
						angles[0], angles[1], angles[2],
						stencil.quad_x_axis, stencil.quad_y_axis, stencil.quad_normal);
					bStencilChanged = true;
				}

				float size[2] = {stencil.quad_width, stencil.quad_height};
				if (ImGui::InputFloat2("Size", size, "%.2f"))
				{
					stencil.quad_width = size[0];
					stencil.quad_height = size[1];
					bStencilChanged = true;
				}

				if (ImGui::Checkbox("Double Sided", &stencil.is_double_sided))
				{
					bStencilChanged = true;
				}

				if (ImGui::Checkbox("Disabled", &stencil.is_disabled))
				{
					bStencilChanged= true;
				}

				if (ImGui::Button("Delete"))
				{
					profile->removeStencil(stencil.stencil_id);
					bIsStencilStillValid= false;
				}
				else if (bStencilChanged)
				{
					profile->updateQuadStencil(stencil);
				}

				ImGui::PopID();

				if (!bIsStencilStillValid)
				{
					break;
				}
			}

			// Box Stencils
			for (int stencilIndex = 0; stencilIndex < profile->boxStencilList.size(); ++stencilIndex)
			{
				bool bIsStencilStillValid = true;

				MikanStencilBox stencil = profile->boxStencilList[stencilIndex];
				bool bStencilChanged = false;

				ImGui::Separator();

				ImGui::PushID(stencil.stencil_id);
				ImGui::Text("Box Stencil #%d", stencil.stencil_id);

				MikanSpatialAnchorInfo spatialAnchor;
				bool bValidAnchor = profile->getSpatialAnchorInfo(stencil.parent_anchor_id, spatialAnchor);
				const char* anchorName = bValidAnchor ? spatialAnchor.anchor_name : "No Anchor";
				if (ImGui::Button(anchorName))
				{
					stencil.parent_anchor_id = profile->getNextSpatialAnchorId(stencil.parent_anchor_id);
					bStencilChanged = true;
				}

				float position[3] = { stencil.box_center.x, stencil.box_center.y, stencil.box_center.z };
				if (ImGui::InputFloat3("Position", position, "%.2f"))
				{
					stencil.box_center.x = position[0];
					stencil.box_center.y = position[1];
					stencil.box_center.z = position[2];
					bStencilChanged = true;
				}

				float angles[3];
				MikanOrientationToEulerAngles(
					stencil.box_x_axis, stencil.box_y_axis, stencil.box_z_axis,
					angles[0], angles[1], angles[2]);
				if (ImGui::InputFloat3("Rotation", angles, "%.1f"))
				{
					EulerAnglesToMikanOrientation(
						angles[0], angles[1], angles[2],
						stencil.box_x_axis, stencil.box_y_axis, stencil.box_z_axis);
					bStencilChanged = true;
				}

				float size[3] = { stencil.box_x_size, stencil.box_y_size, stencil.box_z_size };
				if (ImGui::InputFloat3("Size", size, "%.2f"))
				{
					stencil.box_x_size = size[0];
					stencil.box_y_size = size[1];
					stencil.box_z_size = size[2];
					bStencilChanged = true;
				}

				if (ImGui::Checkbox("Disabled", &stencil.is_disabled))
				{
					bStencilChanged = true;
				}

				if (ImGui::Button("Delete"))
				{
					profile->removeStencil(stencil.stencil_id);
					bIsStencilStillValid = false;
				}
				else if (bStencilChanged)
				{
					profile->updateBoxStencil(stencil);
				}

				ImGui::PopID();

				if (!bIsStencilStillValid)
				{
					break;
				}
			}

			// Model Stencils
			for (int stencilIndex = 0; stencilIndex < profile->modelStencilList.size(); ++stencilIndex)
			{
				bool bIsStencilStillValid = true;

				MikanStencilModelConfig& modelConfig= profile->modelStencilList[stencilIndex];
				MikanStencilModel stencil = modelConfig.modelInfo;
				bool bStencilChanged = false;

				ImGui::Separator();

				ImGui::PushID(stencil.stencil_id);
				ImGui::Text("Model Stencil #%d", stencil.stencil_id);

				MikanSpatialAnchorInfo spatialAnchor;
				bool bValidAnchor = profile->getSpatialAnchorInfo(stencil.parent_anchor_id, spatialAnchor);
				const char* anchorName = bValidAnchor ? spatialAnchor.anchor_name : "No Anchor";
				if (ImGui::Button(anchorName))
				{
					stencil.parent_anchor_id = profile->getNextSpatialAnchorId(stencil.parent_anchor_id);
					bStencilChanged = true;
				}

				float position[3] = { stencil.model_position.x, stencil.model_position.y, stencil.model_position.z };
				if (ImGui::InputFloat3("Position", position, "%.2f"))
				{
					stencil.model_position.x = position[0];
					stencil.model_position.y = position[1];
					stencil.model_position.z = position[2];
					bStencilChanged = true;
				}

				float angles[3] = { stencil.model_rotator.x_angle, stencil.model_rotator.y_angle, stencil.model_rotator.z_angle };
				if (ImGui::InputFloat3("Rotation", angles, "%.1f"))
				{
					stencil.model_rotator.x_angle = angles[0];
					stencil.model_rotator.y_angle = angles[1];
					stencil.model_rotator.z_angle = angles[2];
					bStencilChanged = true;
				}

				float scale[3] = { stencil.model_scale.x, stencil.model_scale.y, stencil.model_scale.z };
				if (ImGui::InputFloat3("Scale", scale, "%.2f"))
				{
					stencil.model_scale.x = scale[0];
					stencil.model_scale.y = scale[1];
					stencil.model_scale.z = scale[2];
					bStencilChanged = true;
				}

				if (ImGui::Checkbox("Disabled", &stencil.is_disabled))
				{
					bStencilChanged = true;
				}

				ImGui::PushItemWidth(-100.f);
				ImGui::InputText("Path:", &modelConfig.modelPath);
				ImGui::PopItemWidth();

				ImGui::SameLine();
				if (ImGui::Button("..."))
				{
					m_pendingModelFilenameStencilID = stencil.stencil_id;
					m_modelFileDialog->Open();
				}
				else if (ImGui::Button("Delete"))
				{
					profile->removeStencil(stencil.stencil_id);
					bIsStencilStillValid = false;
				}
				else if (bStencilChanged)
				{
					profile->updateModelStencil(stencil);
				}

				ImGui::PopID();

				if (!bIsStencilStillValid)
				{
					break;
				}
			}

			if ((profile->quadStencilList.size() + profile->modelStencilList.size()) < MAX_MIKAN_STENCILS)
			{
				ImGui::Separator();

				if (ImGui::Button("Add Quad Stencil"))
				{
					MikanStencilQuad quad;
					memset(&quad, 0, sizeof(MikanStencilQuad));

					quad.is_double_sided = true;
					quad.parent_anchor_id= INVALID_MIKAN_ID;
					quad.quad_center = { 0.f, 0.f, 0.f };
					quad.quad_x_axis = { 1.f, 0.f, 0.f };
					quad.quad_y_axis = { 0.f, 1.f, 0.f };
					quad.quad_normal = { 0.f, 0.f, 1.f };
					quad.quad_width = 0.25f;
					quad.quad_height = 0.25f;

					profile->addNewQuadStencil(quad);
				}
				
				if (ImGui::Button("Add Box Stencil"))
				{
					MikanStencilBox box;
					memset(&box, 0, sizeof(MikanStencilBox));

					box.parent_anchor_id = INVALID_MIKAN_ID;
					box.box_center = { 0.f, 0.f, 0.f };
					box.box_x_axis = { 1.f, 0.f, 0.f };
					box.box_y_axis = { 0.f, 1.f, 0.f };
					box.box_z_axis = { 0.f, 0.f, 1.f };
					box.box_x_size = 0.25f;
					box.box_y_size = 0.25f;
					box.box_z_size = 0.25f;

					profile->addNewBoxStencil(box);
				}

				if (ImGui::Button("Add Model Stencil"))
				{
					MikanStencilModel model;
					memset(&model, 0, sizeof(MikanStencilModel));

					model.is_disabled = false;
					model.parent_anchor_id = INVALID_MIKAN_ID;
					model.model_position = { 0.f, 0.f, 0.f };
					model.model_rotator = { 0.f, 0.f, 1.f };
					model.model_scale = { 1.f, 1.f, 1.f };

					profile->addNewModelStencil(model);
				}
			}

			ImGui::Checkbox("Show Origin Debug", &m_bDebugRenderOrigin);
			ImGui::Checkbox("Show Anchor Debug", &m_bDebugRenderAnchors);
			ImGui::Checkbox("Show Stencil Debug", &m_bDebugRenderStencils);
		}

		ImGui::Separator();

		if (ImGui::Button("Exit"))
		{
			m_app->popAppState();
		}

		ImGui::End();
	}
}
#endif