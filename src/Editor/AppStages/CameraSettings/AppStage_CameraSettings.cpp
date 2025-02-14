//-- inludes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "CameraSettings/RmlDataBinding_CameraBrightness.h"
#include "CameraSettings/RmlModel_CameraSettings.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "ProfileConfig.h"
#include "TextStyle.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VideoFrameDistortionView.h"

#include <glm/gtc/matrix_transform.hpp>

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----__
const char* AppStage_CameraSettings::APP_STAGE_NAME = "CameraSettings";

//-- public methods -----
AppStage_CameraSettings::AppStage_CameraSettings(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_CameraSettings::APP_STAGE_NAME)
	, m_cameraSettingsModel(std::make_shared<RmlModel_CameraSettings>())
{ }

AppStage_CameraSettings::~AppStage_CameraSettings()
{
	m_cameraSettingsModel= nullptr;
	assert(m_videoBufferView == nullptr);
	assert(m_videoSourceView == nullptr);
}

void AppStage_CameraSettings::enter()
{
	AppStage::enter();

	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	VideoSourceManager* videoSourceManager= m_ownerWindow->getVideoSourceManager();

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		// Init the camera settings model
		Rml::Context* context = getRmlContext();

		m_cameraSettingsModel->init(context, profileConfig, videoSourceManager);
		m_cameraSettingsModel->OnUpdateVideoSourcePath = 
			MakeDelegate(this, &AppStage_CameraSettings::onVideoSourceChanged);

		// Init the camera settings view now that the model is ready
		m_cameraSettingsView = addRmlDocument("camera_settings.rml");
	}

	// Get the current video source based on the config
	m_videoSourceView = VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
	if (m_videoSourceView != nullptr)
	{
		// Fire up the video stream
		startVideoSource();
	}
}

void AppStage_CameraSettings::exit()
{
	// Stop any running video source
	stopVideoSource();

	// Forget about the video source
	m_videoSourceView = nullptr;

	// Clean up the data model
	getRmlContext()->RemoveDataModel("camera_settings");

	AppStage::exit();
}

void AppStage_CameraSettings::pause()
{
	stopVideoSource();

	AppStage::pause();
}

void AppStage_CameraSettings::resume()
{
	startVideoSource();

	AppStage::resume();
}

void AppStage_CameraSettings::startVideoSource()
{
	assert(m_videoBufferView == nullptr);

	if (m_videoSourceView && m_videoSourceView->startVideoStream())
	{
		// Create a texture to hold the video frame
		m_videoBufferView = std::make_shared<VideoFrameDistortionView>(
			m_ownerWindow,
			m_videoSourceView, 
			VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);

		// Update the brightness data binding
		m_cameraSettingsModel->getBrightnessDataBinding()->setVideoSourceView(m_videoSourceView);
	}
}

void AppStage_CameraSettings::stopVideoSource()
{
	// Free the distortion view buffers
	m_videoBufferView = nullptr;

	// Turn back off the video feed
	if (m_videoSourceView)
	{
		m_videoSourceView->saveSettings();
		m_videoSourceView->stopVideoStream();
	}

	// Update the brightness data binding
	m_cameraSettingsModel->getBrightnessDataBinding()->setVideoSourceView(nullptr);
}

void AppStage_CameraSettings::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Get the latest video frame
	if (m_videoBufferView != nullptr)
	{
		m_videoBufferView->readAndProcessVideoFrame();
	}
}

void AppStage_CameraSettings::onVideoSourceChanged(const std::string& newVideoSourcePath)
{
	// Ignore event if this happens during RML view loading
	if (m_cameraSettingsView == nullptr)
		return;

	VideoSourceManager* videoSourceManager= m_ownerWindow->getVideoSourceManager();
	VideoSourceViewPtr newVideoSourceView= videoSourceManager->getVideoSourceViewByPath(newVideoSourcePath);
	
	if (newVideoSourceView != m_videoSourceView)
	{
		ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();

		stopVideoSource();
		m_videoSourceView= newVideoSourceView;
		startVideoSource();

		// Update the currently selected video source in the profile
		profileConfig->videoSourcePath = newVideoSourcePath;
		profileConfig->markDirty(
			ConfigPropertyChangeSet()
			.addPropertyName(ProfileConfig::k_cameraVRDevicePathPropertyId));
	}
}

void AppStage_CameraSettings::render()
{

	if (m_videoBufferView != nullptr)
	{
		const float windowHeight = m_ownerWindow->getHeight();

		m_videoBufferView->renderSelectedVideoBuffers();

		// Always draw the FPS in the lower right
		TextStyle style = getDefaultTextStyle();
		style.horizontalAlignment = eHorizontalTextAlignment::Left;
		style.verticalAlignment = eVerticalTextAlignment::Bottom;
		drawTextAtScreenPosition(
			style,
			glm::vec2(0.f, windowHeight - 1),
			L"Camera %.1ffps", m_videoBufferView->getFPS());
	}
}

void AppStage_CameraSettings::onRmlClickEvent(const std::string& value)
{
	if (value == "goto_mono_tracker_calibration")
	{
		m_ownerWindow->pushAppStage<AppStage_MonoLensCalibration>();
	}
	else if (value == "goto_mono_tracker_test")
	{
		m_ownerWindow->pushAppStage<AppStage_MonoLensCalibration>()->setBypassCalibrationFlag(true);
	}
	else if (value == "goto_main_menu")
	{
		m_ownerWindow->popAppState();
	}
}