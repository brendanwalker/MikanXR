//-- inludes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "CameraSettings/RmlModel_CameraSettings.h"
#include "Shared/RmlDataBinding_CameraBrightness.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "ProjectConfig.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceSystem.h"

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
	assert(m_videoSourceComponent == nullptr);
}

void AppStage_CameraSettings::enter()
{
	AppStage::enter();

	ProjectConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	auto videoSourceSystem= VideoSourceSystem::getSystem();

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		// Init the camera settings model
		Rml::Context* context = getRmlContext();

		m_cameraSettingsModel->init(context, videoSourceSystem);

		// Init the camera settings view now that the model is ready
		m_cameraSettingsView = addRmlDocument("camera_settings.rml");
	}
}

void AppStage_CameraSettings::exit()
{
	// Forget about the video source
	m_videoSourceComponent = nullptr;

	// Clean up the data model
	getRmlContext()->RemoveDataModel("camera_settings");

	AppStage::exit();
}

void AppStage_CameraSettings::pause()
{
	AppStage::pause();
}

void AppStage_CameraSettings::resume()
{
	AppStage::resume();
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