//-- inludes -----
#include "VideoSourceSettings/AppStage_VideoSourceSettings.h"
#include "ARKitVideoSourceComponent.h"
#include "Shared/GuiPanel_ARKitVideoSourceComponent.h"
#include "Shared/GuiPanel_USBVideoSourceComponent.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "IMkGraphicsContext.h"
#include "IMkShaderCache.h"
#include "IMkTriangulatedMesh.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "MkGuiScopedWindow.h"
#include "MkMaterialInstance.h"
#include "MulticastDelegate.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectConfig.h"
#include "TextStyle.h"
#include "VideoSourceComponent.h"
#include "VideoFrameDistortionView.h"
#include "USBVideoSourceComponent.h"

#include "imgui.h"

//-- statics ----__
const char* AppStage_VideoSourceSettings::APP_STAGE_NAME= "VideoSourceSettings";

//-- public methods -----
AppStage_VideoSourceSettings::AppStage_VideoSourceSettings(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_VideoSourceSettings::APP_STAGE_NAME)
{
}

AppStage_VideoSourceSettings::~AppStage_VideoSourceSettings() { assert(m_videoBufferView == nullptr); }

void AppStage_VideoSourceSettings::enter()
{
	AppStage::enter();

	VideoSourceComponentPtr videoSourceComponent= m_videoSourceComponent.lock();

	// Create app stage GUI panels
	// (Auto cleaned up on app state exit)
	{
		auto* usbPanel= addGuiPanel<GuiPanel_USBVideoSourceComponent>();
		usbPanel->init();
		if (auto usbVideoSourceComponent= std::dynamic_pointer_cast<USBVideoSourceComponent>(videoSourceComponent))
		{
			usbPanel->setComponent(usbVideoSourceComponent);
		}

		auto* networkPanel= addGuiPanel<GuiPanel_NetworkVideoSourceComponent>();
		networkPanel->init();
		if (auto networkVideoSourceComponent=
				std::dynamic_pointer_cast<NetworkVideoSourceComponent>(videoSourceComponent))
		{
			networkPanel->setComponent(networkVideoSourceComponent);
		}

		// Exposes the JBU depth-upsample tuning sliders right here, alongside the
		// Depth Preview toggle below, so they can be adjusted live while watching
		// the preview for flicker/edge quality (see GuiPanel_ARKitVideoSourceComponent).
		auto* arkitPanel= addGuiPanel<GuiPanel_ARKitVideoSourceComponent>();
		arkitPanel->init();
		if (auto arkitVideoSourceComponent= std::dynamic_pointer_cast<ARKitVideoSourceComponent>(videoSourceComponent))
		{
			arkitPanel->setComponent(arkitVideoSourceComponent);
		}
	}

	if (videoSourceComponent)
	{
		// Create the video buffer view eagerly — it registers itself for OnFrameSizeChanged
		m_videoBufferView=
			std::make_shared<VideoFrameDistortionView>(videoSourceComponent, eVideoFrameProcessorMode::COMPOSITOR);

		// Register as a stream consumer (VideoSourceComponent::update() drives the retry loop)
		videoSourceComponent->startVideoStream(m_videoBufferView.get());
	}

	// ARKit depth preview (debug/verification tool) - only ever selectable when the
	// current video source is ARKit; force back to Color otherwise so switching
	// away from an ARKit source doesn't leave the display stuck on an unavailable mode.
	m_bIsARKitVideoSource= std::dynamic_pointer_cast<ARKitVideoSourceComponent>(videoSourceComponent) != nullptr;
	if (!m_bIsARKitVideoSource)
		m_displayMode= eDisplayMode::Color;

	IMkGraphicsContext* graphicsContext= m_ownerWindow->getGraphicsContext().get();
	m_fullscreenDepthPreviewQuad= createFullscreenQuadMesh(
		graphicsContext,
		graphicsContext->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_VISUALIZE_ARKIT_DEPTH), true);
}

void AppStage_VideoSourceSettings::exit()
{
	VideoSourceComponentPtr videoSourceComponent= m_videoSourceComponent.lock();

	// Restore normal JBU depth production before leaving, so the compositor doesn't keep
	// getting a raw-depth/matte debug view in the shared depth texture.
	if (auto arkitComponent= std::dynamic_pointer_cast<ARKitVideoSourceComponent>(videoSourceComponent))
		arkitComponent->setDepthPreviewMode(eARKitDepthPreviewMode::jbu_upsampled);

	if (videoSourceComponent && m_videoBufferView)
	{
		videoSourceComponent->stopVideoStream(m_videoBufferView.get());
		m_videoBufferView= nullptr;
	}

	AppStage::exit();
}

void AppStage_VideoSourceSettings::pause()
{
	VideoSourceComponentPtr videoSourceComponent= m_videoSourceComponent.lock();

	if (auto arkitComponent= std::dynamic_pointer_cast<ARKitVideoSourceComponent>(videoSourceComponent))
		arkitComponent->setDepthPreviewMode(eARKitDepthPreviewMode::jbu_upsampled);

	if (videoSourceComponent && m_videoBufferView)
	{
		videoSourceComponent->stopVideoStream(m_videoBufferView.get());
	}

	AppStage::pause();
}

void AppStage_VideoSourceSettings::resume()
{
	VideoSourceComponentPtr videoSourceComponent= m_videoSourceComponent.lock();

	if (videoSourceComponent && m_videoBufferView)
	{
		videoSourceComponent->startVideoStream(m_videoBufferView.get());
	}

	AppStage::resume();
}

void AppStage_VideoSourceSettings::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Get the latest video frame
	if (m_videoBufferView != nullptr)
	{
		m_videoBufferView->readAndProcessVideoFrame();
	}
}

void AppStage_VideoSourceSettings::onGui()
{
	AppStage::onGui();

	constexpr float k_panelWidth= 415.f;
	const float displayWidth= m_ownerWindow->getWidth();
	const float displayHeight= m_ownerWindow->getHeight();
	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);

	constexpr ImGuiWindowFlags k_flags=
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

	MkGuiScopedWindow panel("##VideoSourceSettings", nullptr, k_flags);
	if (!panel)
		return;

	if (ImGui::Button("Return"))
		onReturnEvent();
	ImGui::Separator();

	if (m_bIsARKitVideoSource)
	{
		int displayModeInt= (int)m_displayMode;
		ImGui::RadioButton("Color", &displayModeInt, (int)eDisplayMode::Color);
		ImGui::RadioButton("Depth (JBU)", &displayModeInt, (int)eDisplayMode::DepthPreview);
		ImGui::RadioButton("Depth (raw)", &displayModeInt, (int)eDisplayMode::RawDepthPreview);
		ImGui::RadioButton("Matte (raw)", &displayModeInt, (int)eDisplayMode::MattePreview);
		ImGui::RadioButton("Matte (refined)", &displayModeInt, (int)eDisplayMode::RefinedMattePreview);
		m_displayMode= (eDisplayMode)displayModeInt;

		// Push the selected view to the live device so it renders the matching
		// visualization into the shared depth-preview texture this frame.
		applyDepthPreviewMode();

		ImGui::Separator();
	}

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}

void AppStage_VideoSourceSettings::render(IMkViewportPtr targetViewport)
{
	// Every non-Color mode (JBU / raw depth / matte) is drawn from the same device depth
	// surface - the device decides what's rendered into it via setDepthPreviewMode().
	IMkTexturePtr depthPreviewTexture= (m_displayMode != eDisplayMode::Color && m_videoBufferView != nullptr)
										   ? m_videoBufferView->getDirectDepthTexture()
										   : IMkTexturePtr();

	if (depthPreviewTexture && m_fullscreenDepthPreviewQuad)
	{
		MkMaterialInstancePtr materialInstance= m_fullscreenDepthPreviewQuad->getMaterialInstance();
		MkMaterialConstPtr material= materialInstance->getMaterial();

		if (auto materialBinding= material->bindMaterial())
		{
			materialInstance->setTextureBySemantic(eUniformSemantic::depthTexture, depthPreviewTexture);

			if (auto instanceBinding= materialInstance->bindMaterialInstance(materialBinding))
			{
				m_fullscreenDepthPreviewQuad->drawElements();
			}
		}
	}
	else if (m_videoBufferView != nullptr)
	{
		m_videoBufferView->renderSelectedVideoBuffers();
	}

	// Always draw the FPS in the lower right
	TextStyle style= getDefaultTextStyle();
	style.horizontalAlignment= eHorizontalTextAlignment::Left;
	style.verticalAlignment= eVerticalTextAlignment::Bottom;
	drawTextAtScreenPosition(getGraphicsContext(), style, glm::vec2(0.f, m_ownerWindow->getHeight() - 1),
							 L"Camera %.1ffps", m_videoBufferView ? m_videoBufferView->getFPS() : 0.f);
}

void AppStage_VideoSourceSettings::applyDepthPreviewMode()
{
	auto arkitComponent= std::dynamic_pointer_cast<ARKitVideoSourceComponent>(m_videoSourceComponent.lock());
	if (!arkitComponent)
		return;

	// Color and the JBU depth view both leave the device producing normal JBU depth (so
	// the compositor keeps a valid depth); only the explicit raw/matte debug views
	// repurpose the shared surface.
	eARKitDepthPreviewMode deviceMode= eARKitDepthPreviewMode::jbu_upsampled;
	switch (m_displayMode)
	{
	case eDisplayMode::RawDepthPreview:
		deviceMode= eARKitDepthPreviewMode::raw_depth_nearest;
		break;
	case eDisplayMode::MattePreview:
		deviceMode= eARKitDepthPreviewMode::matte_nearest;
		break;
	case eDisplayMode::RefinedMattePreview:
		deviceMode= eARKitDepthPreviewMode::refined_matte_nearest;
		break;
	default:
		deviceMode= eARKitDepthPreviewMode::jbu_upsampled;
		break;
	}

	arkitComponent->setDepthPreviewMode(deviceMode);
}

void AppStage_VideoSourceSettings::onReturnEvent() { getOwnerWindow()->popAppState(); }

// Remote Control
bool AppStage_VideoSourceSettings::handleRemoteControlCommand(const std::string& command,
															  const std::vector<std::string>& parameters,
															  std::vector<std::string>& outResults)
{
	if (command == "get_video_source_component_id")
	{
		return handleGetVideoSourceComponentId(outResults);
	}
	else if (command == "return")
	{
		return handleReturnRequest(outResults);
	}

	return AppStage::handleRemoteControlCommand(command, parameters, outResults);
}

bool AppStage_VideoSourceSettings::handleGetVideoSourceComponentId(std::vector<std::string>& outResults)
{
	VideoSourceComponentPtr videoSource= m_videoSourceComponent.lock();
	MikanVideoSourceID videoSourceId= videoSource ? videoSource->getComponentId() : INVALID_MIKAN_ID;

	outResults.push_back(std::to_string(videoSourceId));

	return true;
}

bool AppStage_VideoSourceSettings::handleReturnRequest(std::vector<std::string>& outResults)
{
	getOwnerWindow()->popAppState();
	outResults.push_back(IRemoteControllable::k_success);
	return true;
}