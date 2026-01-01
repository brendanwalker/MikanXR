//-- inludes -----
#include "VideoSourceSettings/AppStage_VideoSourceSettings.h"
#include "VideoSourceSettings/RmlModel_VideoSourceSettings.h"
#include "Shared/RmlModel_VideoSourceComponent.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "MulticastDelegate.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectConfig.h"
#include "TextStyle.h"
#include "VideoSourceComponent.h"
#include "VideoSourceSystem.h"
#include "VideoFrameDistortionView.h"
#include "USBVideoSourceComponent.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----__
const char* AppStage_VideoSourceSettings::APP_STAGE_NAME = "VideoSourceSettings";

//-- public methods -----
AppStage_VideoSourceSettings::AppStage_VideoSourceSettings(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_VideoSourceSettings::APP_STAGE_NAME)
{
}

AppStage_VideoSourceSettings::~AppStage_VideoSourceSettings()
{
	assert(m_videoBufferView == nullptr);
}

void AppStage_VideoSourceSettings::enter()
{
	AppStage::enter();

	VideoSourceComponentPtr videoSourceComponent= m_videoSourceComponent.lock();

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init Data Models
		auto* videoSourceSettingsModel = addRmlModel<RmlModel_VideoSourceSettings>();
		videoSourceSettingsModel->init(context);
		videoSourceSettingsModel->OnReturnEvent = MakeDelegate(this, &AppStage_VideoSourceSettings::onReturnEvent);

		auto* usbVideoSourceComponentModel = addRmlModel<RmlModel_USBVideoSourceComponent>();
		usbVideoSourceComponentModel->init(context);
		if (auto usbVideoSourceComponent =
			std::dynamic_pointer_cast<USBVideoSourceComponent>(videoSourceComponent))
		{
			usbVideoSourceComponentModel->setComponent(usbVideoSourceComponent);
		}

		auto* networkVideoSourceComponentModel = addRmlModel<RmlModel_NetworkVideoSourceComponent>();
		networkVideoSourceComponentModel->init(context);
		if (auto networkVideoSourceComponent =
			std::dynamic_pointer_cast<NetworkVideoSourceComponent>(videoSourceComponent))
		{
			networkVideoSourceComponentModel->setComponent(networkVideoSourceComponent);
		}

		// Load the Rml view for the settings
		m_videoSourceSettingsView = addRmlDocument("video_source_settings.rml");

		// Show the main project view by default
		m_videoSourceSettingsView->Show();
		m_videoSourceSettingsView->PullToFront();
	}

	if (videoSourceComponent)
	{
		// Listen for video source events
		videoSourceComponent->OnStarted +=
			MakeDelegate(this, &AppStage_VideoSourceSettings::onVideoSourceStarted);
		videoSourceComponent->OnStopped +=
			MakeDelegate(this, &AppStage_VideoSourceSettings::onVideoSourceStopped);
		videoSourceComponent->OnFrameSizeChanged +=
			MakeDelegate(this, &AppStage_VideoSourceSettings::onVideoSourceFrameSizeChanged);

		// Fire up the video stream
		videoSourceComponent->startVideoStream();
	}
}

void AppStage_VideoSourceSettings::exit()
{
	VideoSourceComponentPtr videoSourceComponent = m_videoSourceComponent.lock();

	if (videoSourceComponent)
	{
		// Fire up the video stream
		videoSourceComponent->stopVideoStream();

		// Listen for video source events
		videoSourceComponent->OnStarted -=
			MakeDelegate(this, &AppStage_VideoSourceSettings::onVideoSourceStarted);
		videoSourceComponent->OnStopped -=
			MakeDelegate(this, &AppStage_VideoSourceSettings::onVideoSourceStopped);
		videoSourceComponent->OnFrameSizeChanged -=
			MakeDelegate(this, &AppStage_VideoSourceSettings::onVideoSourceFrameSizeChanged);
	}

	AppStage::exit();
}

void AppStage_VideoSourceSettings::pause()
{
	VideoSourceComponentPtr videoSourceComponent = m_videoSourceComponent.lock();

	if (videoSourceComponent)
	{
		videoSourceComponent->stopVideoStream();
	}

	AppStage::pause();
}

void AppStage_VideoSourceSettings::resume()
{
	VideoSourceComponentPtr videoSourceComponent = m_videoSourceComponent.lock();

	if (videoSourceComponent)
	{
		videoSourceComponent->startVideoStream();
	}

	AppStage::resume();
}

void AppStage_VideoSourceSettings::onVideoSourceStarted(VideoSourceComponentPtr videoSource)
{
	assert(videoSource != nullptr);

	// Create a texture to hold the video frame
	m_videoBufferView = std::make_shared<VideoFrameDistortionView>(
		m_ownerWindow,
		videoSource,
		VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);
}

void AppStage_VideoSourceSettings::onVideoSourceFrameSizeChanged(VideoSourceComponentPtr videoSource)
{
	assert(videoSource != nullptr);

	// Create a texture to hold the video frame
	m_videoBufferView = std::make_shared<VideoFrameDistortionView>(
		m_ownerWindow,
		videoSource,
		VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);
}

void AppStage_VideoSourceSettings::onVideoSourceStopped(VideoSourceComponentPtr videoSource)
{
	// Free the distortion view buffers
	m_videoBufferView = nullptr;
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

void AppStage_VideoSourceSettings::render(IMkViewportPtr targetViewport)
{
	if (m_videoBufferView != nullptr)
	{
		m_videoBufferView->renderSelectedVideoBuffers();
	}

	// Always draw the FPS in the lower right
	TextStyle style = getDefaultTextStyle();
	style.horizontalAlignment = eHorizontalTextAlignment::Left;
	style.verticalAlignment = eVerticalTextAlignment::Bottom;
	drawTextAtScreenPosition(
		style,
		glm::vec2(0.f, m_ownerWindow->getHeight() - 1),
		L"Camera %.1ffps", m_videoBufferView->getFPS());
}

void AppStage_VideoSourceSettings::onReturnEvent()
{
	getOwnerWindow()->popAppState();
}