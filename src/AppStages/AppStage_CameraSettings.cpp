//-- inludes -----
#include "AppStage_CameraSettings.h"
#include "AppStage_MonoLensCalibration.h"
#include "AppStage_MainMenu.h"
#include "App.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VideoFrameDistortionView.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

struct CameraSettingsDataModel
{
	Rml::DataModelHandle model_handle;

	int brightness= 128;
	int brightness_min = 0;
	int brightness_max = 255;
	int brightness_step = 2;
	Rml::Vector<Rml::String> video_sources;
	int selected_video_source = 0;
};

//-- statics ----__
const char* AppStage_CameraSettings::APP_STAGE_NAME = "CameraSettings";

//-- public methods -----
AppStage_CameraSettings::AppStage_CameraSettings(App* app)
	: AppStage(app, AppStage_CameraSettings::APP_STAGE_NAME)
	, m_dataModel(new CameraSettingsDataModel)
	, m_videoSourceIterator(nullptr)
	, m_videoBufferView(nullptr)
{ }

AppStage_CameraSettings::~AppStage_CameraSettings()
{
	delete m_dataModel;
	assert(m_videoSourceIterator == nullptr);
}

void AppStage_CameraSettings::enter()
{
	AppStage::enter();
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("camera_settings");
	if (!constructor)
		return;

	constructor.RegisterArray<Rml::Vector<Rml::String>>();
	constructor.Bind("brightness", &m_dataModel->brightness);
	constructor.Bind("video_sources", &m_dataModel->video_sources);
	constructor.Bind("selected_video_source", &m_dataModel->selected_video_source);
	m_dataModel->model_handle = constructor.GetModelHandle();
	
	m_dataModel->video_sources.push_back("Select Video Source");
	VideoSourceList videoSourceList= VideoSourceManager::getInstance()->getVideoSourceList();
	for (VideoSourceViewPtr view : videoSourceList)
	{
		m_dataModel->video_sources.push_back(view->getFriendlyName());
	}

	m_videoSourceIterator= new VideoSourceListIterator(profileConfig->videoSourcePath);
	if (m_videoSourceIterator->hasVideoSources())
	{
		VideoSourceViewPtr videoSource = m_videoSourceIterator->getCurrent();

		// Set the currently selected video source 
		m_dataModel->selected_video_source= m_videoSourceIterator->getCurrentIndex() + 1;

		// Update the current video source in the profile
		profileConfig->videoSourcePath = videoSource->getUSBDevicePath();
		profileConfig->save();

		// Fire up the video stream
		startVideoSource(videoSource);
	}
	else
	{
		m_dataModel->selected_video_source= 0;
	}

	pushRmlDocument("rml\\camera_settings.rml");
}

void AppStage_CameraSettings::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("camera_settings");

	if (m_videoSourceIterator != nullptr)
	{
		// Stop any running video source
		stopVideoSource(m_videoSourceIterator->getCurrent());

		// Clean up the video source iterator
		delete m_videoSourceIterator;
		m_videoSourceIterator= nullptr;
	}

	AppStage::exit();
}

void AppStage_CameraSettings::pause()
{
	stopVideoSource(m_videoSourceIterator->getCurrent());

	AppStage::pause();
}

void AppStage_CameraSettings::resume()
{
	startVideoSource(m_videoSourceIterator->getCurrent());

	AppStage::resume();
}

void AppStage_CameraSettings::startVideoSource(VideoSourceViewPtr videoSource)
{
	assert(m_videoBufferView == nullptr);

	if (videoSource && videoSource->startVideoStream())
	{
		// Create a texture to hold the video frame
		m_videoBufferView = new VideoFrameDistortionView(videoSource, VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);

		// Fetch video properties we wamt to update in the UI
		m_dataModel->brightness = videoSource->getVideoProperty(VideoPropertyType::Brightness);
		m_dataModel->brightness_min = videoSource->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
		m_dataModel->brightness_max = videoSource->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
		m_dataModel->brightness_step = videoSource->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);
	}
}

void AppStage_CameraSettings::stopVideoSource(VideoSourceViewPtr videoSource)
{
	// Free the distortion view buffers
	if (m_videoBufferView != nullptr)
	{
		delete m_videoBufferView;
		m_videoBufferView = nullptr;
	}

	// Turn back off the video feed
	if (videoSource)
	{
		videoSource->saveSettings();
		videoSource->stopVideoStream();
	}
}

void AppStage_CameraSettings::update()
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();
	VideoSourceViewPtr videoSource= m_videoSourceIterator->getCurrent();

	if (m_dataModel->model_handle.IsVariableDirty("brightness"))
	{
		videoSource->setVideoProperty(VideoPropertyType::Brightness, m_dataModel->brightness, true);
		m_dataModel->brightness = videoSource->getVideoProperty(VideoPropertyType::Brightness);
	}

	if (m_dataModel->model_handle.IsVariableDirty("selected_video_source"))
	{
		VideoSourceViewPtr oldVideoSource = m_videoSourceIterator->getCurrent();

		if (m_videoSourceIterator->goToIndex(m_dataModel->selected_video_source - 1))
		{
			// Update the config and video stream if we changed video sources
			VideoSourceViewPtr newVideoSource = m_videoSourceIterator->getCurrent();
			const std::string friendlyName = newVideoSource->getFriendlyName();
			const std::string usbPath = newVideoSource->getUSBDevicePath();

			if (newVideoSource != oldVideoSource)
			{
				stopVideoSource(oldVideoSource);
				startVideoSource(newVideoSource);

				// Save out to the profile
				profileConfig->videoSourcePath = newVideoSource->getUSBDevicePath();
				profileConfig->save();
			}
		}
	}

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
		m_videoBufferView->renderSelectedVideoBuffers();
	}
}

void AppStage_CameraSettings::onRmlClickEvent(const std::string& value)
{
	if (value == "goto_mono_tracker_calibration")
	{
		m_app->pushAppStage<AppStage_MonoLensCalibration>();
	}
	else if (value == "goto_mono_tracker_test")
	{
		m_app->pushAppStage<AppStage_MonoLensCalibration>()->setBypassCalibrationFlag(true);
	}
	else if (value == "goto_main_menu")
	{
		m_app->popAppState();
	}
}

#if 0
void AppStage_CameraSettings::renderUI()
{
	ProfileConfig* profileConfig= App::getInstance()->getProfileConfig();

	const char* k_window_title = "Video Source Settings";
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;
	const ImVec2 displaySize = ImGui::GetMainViewport()->Size;
	const ImVec2 panelSize = ImVec2(350, 230);

	ImGui::SetNextWindowPos(ImVec2(10, displaySize.y - 10), 0, ImVec2(0.0f, 1.0f));
	ImGui::SetNextWindowSize(panelSize);
	ImGui::Begin(k_window_title, nullptr, window_flags);

	if (m_videoSourceIterator->hasVideoSources())
	{
		// Video source selection
		{
			VideoSourceViewPtr oldVideoSource= m_videoSourceIterator->getCurrent();

			if (ImGui::Button("<##VideoSourceIndex"))
			{
				m_videoSourceIterator->goPrevious();
			}
			ImGui::SameLine();
			ImGui::Text("Video Source: %d", m_videoSourceIterator->getCurrentIndex());
			ImGui::SameLine();
			if (ImGui::Button(">##VideoSourceIndex"))
			{
				m_videoSourceIterator->goNext();
			}

			// Update the config and video stream if we changed video sources
			VideoSourceViewPtr newVideoSource = m_videoSourceIterator->getCurrent();
			const std::string friendlyName = newVideoSource->getFriendlyName();
			const std::string usbPath = newVideoSource->getUSBDevicePath();
			if (newVideoSource != oldVideoSource)
			{
				stopVideoSource(oldVideoSource);
				startVideoSource(newVideoSource);

				// Save out to the profile
				profileConfig->videoSourcePath = newVideoSource->getUSBDevicePath();
				profileConfig->save();
			}

			ImGui::TextWrapped("%s", friendlyName.c_str());
		}

		ImGui::Spacing();

		// Brightness
		{
			VideoSourceViewPtr videoSource= m_videoSourceIterator->getCurrent();

			if (ImGui::Button("-##Brightness"))
			{
				videoSource->setVideoProperty(VideoPropertyType::Brightness, m_brightness - m_brightnessStep, true);
				m_brightness = videoSource->getVideoProperty(VideoPropertyType::Brightness);
			}
			ImGui::SameLine();
			if (ImGui::Button("+##Brightness"))
			{
				videoSource->setVideoProperty(VideoPropertyType::Brightness, m_brightness + m_brightnessStep, true);
				m_brightness = videoSource->getVideoProperty(VideoPropertyType::Brightness);
			}
			ImGui::SameLine();
			ImGui::Text("Brightness: %d", m_brightness);
		}
		
		ImGui::Separator();

		{
			if (ImGui::Button("Calibrate Mono Tracker"))
			{
				m_app->pushAppStage<AppStage_MonoLensCalibration>();
			}
			ImGui::SameLine();
			if (ImGui::Button("Test Mono Calibration"))
			{
				m_app->pushAppStage<AppStage_MonoLensCalibration>()->setBypassCalibrationFlag(true);
			}
		}
	}
	else
	{
		ImGui::Text("No video sources");
	}

	ImGui::Separator();

	const ImVec2 buttonSize = ImVec2(150, 25);
	ImGui::NewLine();
	ImGui::SameLine((panelSize.x / 2) - (buttonSize.x / 2));
	if (ImGui::Button(locTextUTF8("", "return_to_main_menu"), buttonSize))
	{
		m_app->popAppState();
	}

	ImGui::End();
}
#endif