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

//-- statics ----__
const char* AppStage_CameraSettings::APP_STAGE_NAME = "CameraSettings";

//-- public methods -----
AppStage_CameraSettings::AppStage_CameraSettings(App* app)
	: AppStage(app, AppStage_CameraSettings::APP_STAGE_NAME)
	, m_videoSourceIterator(nullptr)
	, m_videoBufferView(nullptr)
{ }

AppStage_CameraSettings::~AppStage_CameraSettings()
{
	assert(m_videoSourceIterator == nullptr);
}

void AppStage_CameraSettings::enter()
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	m_videoSourceIterator= new VideoSourceListIterator(profileConfig->videoSourcePath);

	if (m_videoSourceIterator->hasVideoSources())
	{
		VideoSourceViewPtr videoSource = m_videoSourceIterator->getCurrent();

		// Update the current video source in the profile
		profileConfig->videoSourcePath = videoSource->getUSBDevicePath();
		profileConfig->save();

		// Fire up the video stream
		startVideoSource(videoSource);
	}
	else
	{
		m_brightness= 0;
	}
}

void AppStage_CameraSettings::exit()
{
	if (m_videoSourceIterator != nullptr)
	{
		// Stop any running video source
		stopVideoSource(m_videoSourceIterator->getCurrent());

		// Clean up the video source iterator
		delete m_videoSourceIterator;
		m_videoSourceIterator= nullptr;
	}
}

void AppStage_CameraSettings::pause()
{
	stopVideoSource(m_videoSourceIterator->getCurrent());
}

void AppStage_CameraSettings::resume()
{
	startVideoSource(m_videoSourceIterator->getCurrent());
}

void AppStage_CameraSettings::startVideoSource(VideoSourceViewPtr videoSource)
{
	assert(m_videoBufferView == nullptr);

	if (videoSource && videoSource->startVideoStream())
	{
		// Create a texture to hold the video frame
		m_videoBufferView = new VideoFrameDistortionView(videoSource, VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);

		// Fetch video properties we wamt to update in the UI
		m_brightness = videoSource->getVideoProperty(VideoPropertyType::Brightness);
		m_brightnessMin = videoSource->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
		m_brightnessMax = videoSource->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
		m_brightnessStep = videoSource->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);
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