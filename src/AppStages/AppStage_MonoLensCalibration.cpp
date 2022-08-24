// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "AppStage_MonoLensCalibration.h"
#include "App.h"
#include "InputManager.h"
#include "MonoLensDistortionCalibrator.h"
#include "MikanClientTypes.h"
#include "CalibrationPatternFinder.h"
#include "Renderer.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VideoFrameDistortionView.h"

#include "SDL_keycode.h"

#include <imgui.h>

//-- statics ----
const char* AppStage_MonoLensCalibration::APP_STAGE_NAME = "MonoCalibration";

//-- constants -----
static const char* k_video_display_mode_names[] = {
	"BGR",
	"Undistorted",
	"Grayscale"
};

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 

//-- public methods -----
AppStage_MonoLensCalibration::AppStage_MonoLensCalibration(App* app)
	: AppStage(app, AppStage_MonoLensCalibration::APP_STAGE_NAME)
	, m_menuState(AppStage_MonoLensCalibration::inactive)
	, m_brightness(0)
	, m_brightnessMin(0)
	, m_brightnessMax(0)
	, m_brightnessStep(0)
	, m_bypassCalibrationFlag(false)
	, m_videoSourceView()
	, m_monoLensCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
{
}

void AppStage_MonoLensCalibration::enter()
{
	// Get the current video source based on the config
	const ProfileConfig* profileConfig= App::getInstance()->getProfileConfig();
	m_videoSourceView= VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();

	// Bind to space bar to capture frames
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_SPACE)->OnKeyPressed+= 
		MakeDelegate(this, &AppStage_MonoLensCalibration::captureRequested);

	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = new VideoFrameDistortionView(m_videoSourceView, VIDEO_FRAME_HAS_ALL);

		// Create a calibrator to do the actual pattern recording and calibration
		m_monoLensCalibrator =
			new MonoLensDistortionCalibrator(
				profileConfig,
				m_monoDistortionView,
				DESIRED_CAPTURE_BOARD_COUNT);

		// Cache off the brightness properties
		m_brightnessMin = m_videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
		m_brightnessMax = m_videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
		m_brightnessStep = m_videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);
		m_brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_bypassCalibrationFlag)
		{
			m_menuState = AppStage_MonoLensCalibration::testCalibration;
		}
		else
		{
			m_menuState = AppStage_MonoLensCalibration::capture;
		}
	}
	else
	{
		m_menuState = AppStage_MonoLensCalibration::failedVideoStartStreamRequest;
	}

}

void AppStage_MonoLensCalibration::exit()
{
	m_menuState = AppStage_MonoLensCalibration::inactive;

	// Save all modified camera config values 
	// (i.e. camera intrinsics and distortion coefficients)
	if (!m_bypassCalibrationFlag)
	{
		m_videoSourceView->saveSettings();
	}

	// Free the calibrator
	if (m_monoLensCalibrator != nullptr)
	{
		delete m_monoLensCalibrator;
		m_monoLensCalibrator = nullptr;
	}

	// Free the distortion view buffers
	if (m_monoDistortionView != nullptr)
	{
		delete m_monoDistortionView;
		m_monoDistortionView = nullptr;
	}

	// Turn back off the video feed
	m_videoSourceView->stopVideoStream();
	m_videoSourceView = nullptr;
}

void AppStage_MonoLensCalibration::update()
{
	if (m_menuState == AppStage_MonoLensCalibration::testCalibration)	
	{
		// Update the video frame buffers using the existing distortion calibration
		m_monoDistortionView->readAndProcessVideoFrame();
	}
	else if (m_menuState == AppStage_MonoLensCalibration::capture)
	{
		// Update the video frame buffers
		m_monoDistortionView->readAndProcessVideoFrame();

		// Update the chess board capture state
		m_monoLensCalibrator->findNewCalibrationPattern(BOARD_NEW_LOCATION_PIXEL_DIST);

		// See if we have gotten all the samples we require
		if (m_monoLensCalibrator->hasSampledAllCalibrationPatterns())
		{
			// Kick off the async task (very expensive)
			m_monoLensCalibrator->computeCameraCalibration();
			m_menuState = AppStage_MonoLensCalibration::processingCalibration;
		}
	}
	else if (m_menuState == AppStage_MonoLensCalibration::processingCalibration)
	{
		MikanMonoIntrinsics new_mono_intrinsics;

		if (m_monoLensCalibrator->getIsCameraCalibrationComplete())
		{
			if (m_monoLensCalibrator->getCameraCalibration(&new_mono_intrinsics))
			{
				// Update the camera intrinsics for this camera
				MikanVideoSourceIntrinsics cameraIntrinsics;
				cameraIntrinsics.intrinsics.mono = new_mono_intrinsics;
				cameraIntrinsics.intrinsics_type = MONO_CAMERA_INTRINSICS;
				m_videoSourceView->setCameraIntrinsics(cameraIntrinsics);

				// Rebuild the distortion map to reflect the updated calibration
				m_monoDistortionView->rebuildDistortionMap(&new_mono_intrinsics);

				// Switch back to the color video feed
				m_monoDistortionView->setVideoDisplayMode(mode_undistored);

				// Go to the test calibration state
				m_menuState = AppStage_MonoLensCalibration::testCalibration;
			}
			else
			{
				m_menuState = AppStage_MonoLensCalibration::failedCalibration;
			}
		}
	}
}

void AppStage_MonoLensCalibration::render()
{
	if (m_menuState == AppStage_MonoLensCalibration::capture)
	{
		m_monoDistortionView->renderSelectedVideoBuffers();
		m_monoLensCalibrator->renderCalibrationState();
	}
	else if (m_menuState == AppStage_MonoLensCalibration::testCalibration)
	{
		m_monoDistortionView->renderSelectedVideoBuffers();
	}
}

void AppStage_MonoLensCalibration::renderCameraSettingsUI()
{
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowPos(ImVec2(10.f, ImGui::GetIO().DisplaySize.y - 100 - 10.f));
	ImGui::SetNextWindowSize(ImVec2(275, 100));
	ImGui::Begin("Video Controls", nullptr, window_flags);

	if (ImGui::Button("<##Filter"))
	{
		m_monoDistortionView->setVideoDisplayMode(
			static_cast<eVideoDisplayMode>(
				(m_monoDistortionView->getVideoDisplayMode() + eVideoDisplayMode::MAX_VIDEO_DISPLAY_MODES - 1)
				% eVideoDisplayMode::MAX_VIDEO_DISPLAY_MODES));
	}
	ImGui::SameLine();
	if (ImGui::Button(">##Filter"))
	{
		m_monoDistortionView->setVideoDisplayMode(
			static_cast<eVideoDisplayMode>(
				(m_monoDistortionView->getVideoDisplayMode() + 1) % eVideoDisplayMode::MAX_VIDEO_DISPLAY_MODES));
	}
	ImGui::SameLine();
	ImGui::Text("Video Filter Mode: %s", k_video_display_mode_names[m_monoDistortionView->getVideoDisplayMode()]);
	
	if (ImGui::Button("-##Brightness"))
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_brightness - m_brightnessStep, true);
		m_brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	}
	ImGui::SameLine();
	if (ImGui::Button("+##Brightness"))
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_brightness + m_brightnessStep, true);
		m_brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	}
	ImGui::SameLine();
	ImGui::Text("Brightness: %d", m_brightness);

	ImGui::End();
}

void AppStage_MonoLensCalibration::renderUI()
{
	ProfileConfig* profileConfig= App::getInstance()->getProfileConfig();

	const float k_panel_width = 200.f;
	const char* k_window_title = "Distortion Calibration";
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;

	switch (m_menuState)
	{
	case eMenuState::capture:
	{
		assert(m_monoLensCalibrator != nullptr);

		renderCameraSettingsUI();

		{
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.f - k_panel_width / 2.f, 20.f));
			ImGui::SetNextWindowSize(ImVec2(k_panel_width, 110));
			ImGui::Begin(k_window_title, nullptr, window_flags);

			ImGui::ProgressBar(m_monoLensCalibrator->computeCalibrationProgress(), ImVec2(k_panel_width - 20, 20));

			if (ImGui::Button("Restart"))
			{
				// Clear out all of the calibration data we recorded
				m_monoLensCalibrator->resetCalibrationState();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_app->popAppState();
			}
			else if (m_monoLensCalibrator->areCurrentImagePointsValid())
			{
				ImGui::Text("Press spacebar to capture");
			}

			ImGui::End();
		}
	} break;

	case eMenuState::processingCalibration:
	{
		const float k_wide_panel_width = 250.f;
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.f - k_wide_panel_width / 2.f, 20.f));
		ImGui::SetNextWindowSize(ImVec2(k_wide_panel_width, 100));

		ImGui::Begin("PROCESSING", nullptr, window_flags);

		ImGui::TextWrapped(
			"Computing distortion calibration.\n" \
			"This may take a few seconds...");

		ImGui::End();
	} break;

	case eMenuState::testCalibration:
	{
		renderCameraSettingsUI();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.f - k_panel_width / 2.f, 10.f));
		ImGui::SetNextWindowSize(ImVec2(k_panel_width, 110));
		ImGui::Begin(k_window_title, nullptr, window_flags);

		if (!m_bypassCalibrationFlag)
		{
			ImGui::Text("Calibration complete!");
			ImGui::Text("Avg Error: %f", m_monoLensCalibrator->getReprojectionError());
		}

		if (!m_bypassCalibrationFlag)
		{
			if (ImGui::Button("Redo Calibration"))
			{
				// Clear out all of the calibration data we recorded
				m_monoLensCalibrator->resetCalibrationState();

				// Reset the distortion map back to the camera intrinsics we started with
				m_monoLensCalibrator->resetDistortionView();

				m_menuState = eMenuState::capture;
			}
			ImGui::SameLine();
		}
		if (ImGui::Button("Ok"))
		{
			m_app->popAppState();
		}

		ImGui::End();
	} break;

	case eMenuState::failedCalibration:
	{
		ImGui::SetNextWindowSize(ImVec2(k_panel_width, 130));
		ImGui::Begin(k_window_title, nullptr, window_flags);

		ImGui::Text("Failed calibration! See log for details.");

		if (ImGui::Button("Redo Calibration"))
		{
			// Clear out all of the calibration data we recorded
			m_monoLensCalibrator->resetCalibrationState();

			// Reset the distortion map back to the camera intrinsics we started with
			m_monoLensCalibrator->resetDistortionView();

			m_menuState = eMenuState::capture;
		}
		ImGui::SameLine();
		if (ImGui::Button("Ok"))
		{
			m_app->popAppState();
		}

		ImGui::End();
	} break;

	case eMenuState::failedVideoStartStreamRequest:
	{
		ImGui::SetNextWindowSize(ImVec2(k_panel_width, 130));
		ImGui::Begin(k_window_title, nullptr, window_flags);

		if (m_menuState == eMenuState::failedVideoStartStreamRequest)
			ImGui::Text("Failed to start tracker stream!");
		else
			ImGui::Text("Failed to open tracker stream!");

		if (ImGui::Button("Ok"))
		{
			m_app->popAppState();
		}

		ImGui::End();
	} break;

	default:
		assert(0 && "unreachable");
	}
}

void AppStage_MonoLensCalibration::captureRequested()
{
	if (m_menuState == eMenuState::capture)
	{
		// Update the chess board capture state
		if (m_monoLensCalibrator != nullptr)
		{
			m_monoLensCalibrator->captureLastFoundCalibrationPattern();
		}
	}
}