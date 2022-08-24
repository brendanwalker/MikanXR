// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "AppStage_CameraSettings.h"
#include "AppStage_AlignmentCalibration.h"
#include "App.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlScene.h"
#include "GlTextRenderer.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MonoLensTrackerPoseCalibrator.h"
#include "CalibrationPatternFinder.h"
#include "Renderer.h"
#include "TextStyle.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VideoFrameDistortionView.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include "SDL_keycode.h"

#include <imgui.h>

#include "glm/gtc/quaternion.hpp"

//-- statics ----
const char* AppStage_AlignmentCalibration::APP_STAGE_NAME = "AlignmentCalibration";

//-- constants -----
static const char* k_calibration_pattern_names[] = {
	"Chessboard",
	"Circle Grid",
};

#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 

//-- public methods -----
AppStage_AlignmentCalibration::AppStage_AlignmentCalibration(App* app)
	: AppStage(app, AppStage_AlignmentCalibration::APP_STAGE_NAME)
	, m_menuState(eMenuState::inactive)
	, m_uiBrightness(0)
	, m_bypassCalibrationFlag(false)
	, m_videoSourceView()
	, m_trackerPoseCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
	, m_scene(new GlScene)
	, m_camera(nullptr)
{
}

AppStage_AlignmentCalibration::~AppStage_AlignmentCalibration()
{
	delete m_scene;
}

void AppStage_AlignmentCalibration::enter()
{
	// Get the current video source based on the config
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
	m_cameraTrackingPuckView= 
		VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->cameraVRDevicePath).getCurrent();
	m_matTrackingPuckView =
		VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->matVRDevicePath).getCurrent();

	// Add all VR devices to the 3d scene
	VRDeviceList vrDeviceList= VRDeviceManager::getInstance()->getVRDeviceList();
	for (auto it : vrDeviceList)
	{
		it->getVRDeviceInterface()->bindToScene(m_scene);
	}

	// Create a new camera to view the scene
	m_camera = App::getInstance()->getRenderer()->pushCamera();
	m_camera->bindInput();

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Fire up the video scene in the background
	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_videoSourceView, 
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);		

		// Create a calibrator to do the actual pattern recording and calibration
		m_trackerPoseCalibrator =
			new MonoLensTrackerPoseCalibrator(
				profileConfig,
				m_cameraTrackingPuckView,
				m_matTrackingPuckView,
				m_monoDistortionView,
				DESIRED_CAPTURE_BOARD_COUNT);

		// Make sure we have a reasonable brightness value when previewing the camera.
		// This override will get rolled back once we exit this app stage
		const int newBrightnessValue =
			(m_videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness) +
				m_videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness)) / 2;
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, newBrightnessValue, false);

		// Cache off the current brightness setting into a value that ui can display
		// so that we aren't polling the video property every tick .
		m_uiBrightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_bypassCalibrationFlag)
		{
			m_menuState = eMenuState::testCalibration;
			m_monoDistortionView->setGrayscaleUndistortDisabled(true);
			setViewpointMode(eViewpointMode::mixedRealityViewpoint);
		}
		else
		{
			m_menuState = eMenuState::verifySetup;
			m_monoDistortionView->setGrayscaleUndistortDisabled(false);
			setViewpointMode(eViewpointMode::cameraViewpoint);
		}
	}
	else
	{
		m_menuState = eMenuState::failedVideoStartStreamRequest;
	}
}

void AppStage_AlignmentCalibration::exit()
{
	m_menuState = eMenuState::inactive;

	App::getInstance()->getRenderer()->popCamera();
	m_camera= nullptr;

	VRDeviceList vrDeviceList = VRDeviceManager::getInstance()->getVRDeviceList();
	for (auto it : vrDeviceList)
	{
		it->getVRDeviceInterface()->removeFromBoundScene();
	}

	// Save all modified camera config values 
	// (i.e. camera intrinsics and distortion coefficients)
	if (!m_bypassCalibrationFlag)
	{
		m_videoSourceView->saveSettings();
	}

	// Re-Load settings back from config 
	// This will reset an modified video properties not saved in the config
	// (i.e. the camera brightness settings we modified)
	m_videoSourceView->loadSettings();

	// Free the calibrator
	if (m_trackerPoseCalibrator != nullptr)
	{
		delete m_trackerPoseCalibrator;
		m_trackerPoseCalibrator = nullptr;
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

void AppStage_AlignmentCalibration::setViewpointMode(eViewpointMode newViewMode)
{
	if (newViewMode != m_viewMode)
	{
		switch (newViewMode)
		{
		case eViewpointMode::cameraViewpoint:
			m_camera->setIsLocked(true);
			break;
		case eViewpointMode::vrViewpoint:
			m_camera->setIsLocked(false);
			break;
		case eViewpointMode::mixedRealityViewpoint:
			m_camera->setIsLocked(true);
			break;
		default:
			break;
		}

		m_viewMode= newViewMode;
	}
}

void AppStage_AlignmentCalibration::updateCamera()
{
	switch (m_viewMode)
	{
	case eViewpointMode::cameraViewpoint:
		{
			m_camera->setModelViewMatrix(glm::mat4(1.f));
		}
		break;
	case eViewpointMode::vrViewpoint:
		{
			m_camera->recomputeModelViewMatrix();
		}
		break;
	case eViewpointMode::mixedRealityViewpoint:
		{
			// Update the transform of the camera so that vr models align over the tracking puck
			glm::mat4 cameraPose;
			if (m_menuState == eMenuState::testCalibration)
			{
				// Use the calibrated offset on the video source to get the camera pose
				cameraPose= m_videoSourceView->getCameraPose(m_cameraTrackingPuckView);
			}
			else
			{
				// Use the last computed preview camera alignment
				cameraPose = m_trackerPoseCalibrator->getLastCameraPose(m_cameraTrackingPuckView);
			}

			m_camera->setCameraPose(cameraPose);
		}
		break;
	}
}

void AppStage_AlignmentCalibration::update()
{
	updateCamera();

	if (m_menuState == eMenuState::verifySetup)
	{
		// Update the video frame buffers to preview the calibration mat
		m_monoDistortionView->readAndProcessVideoFrame();

		// Look for a calibration pattern so that we can preview if it's in frame
		m_trackerPoseCalibrator->computeCameraToPuckXform();
	}
	else if (m_menuState == eMenuState::capture)
	{
		// Update the video frame buffers
		m_monoDistortionView->readAndProcessVideoFrame();

		// Update the chess board capture state
		if (m_trackerPoseCalibrator->computeCameraToPuckXform())
		{
			m_trackerPoseCalibrator->sampleLastCameraToPuckXform();
		}

		// See if we have gotten all the samples we require
		if (m_trackerPoseCalibrator->hasFinishedSampling())
		{
			MikanQuatd rotationOffset;
			MikanVector3d translationOffset;
			if (m_trackerPoseCalibrator->computeCalibratedCameraTrackerOffset(
				rotationOffset,
				translationOffset))
			{
				m_videoSourceView->setCameraPoseOffset(rotationOffset, translationOffset);
				m_videoSourceView->saveSettings();

				// Go to the test calibration state
				m_menuState = eMenuState::testCalibration;
				m_monoDistortionView->setGrayscaleUndistortDisabled(true);
				setViewpointMode(eViewpointMode::mixedRealityViewpoint);
			}
		}
	}
	else if (m_menuState == eMenuState::testCalibration)
	{
		// Update the video frame buffers using the existing distortion calibration
		m_monoDistortionView->readAndProcessVideoFrame();
	}
}

void AppStage_AlignmentCalibration::render()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	if (m_menuState == eMenuState::verifySetup)
	{
		if (m_viewMode == eViewpointMode::cameraViewpoint)
		{
			m_monoDistortionView->renderSelectedVideoBuffers();
			m_trackerPoseCalibrator->renderCameraSpaceCalibrationState();
		}
		else if (m_viewMode == eViewpointMode::vrViewpoint)
		{
			m_trackerPoseCalibrator->renderVRSpaceCalibrationState();
			renderVRScene();
		}
		else if (m_viewMode == eViewpointMode::mixedRealityViewpoint)
		{
			m_monoDistortionView->renderSelectedVideoBuffers();
			renderVRScene();
		}
	}
	else if (m_menuState == eMenuState::capture)
	{
		m_monoDistortionView->renderSelectedVideoBuffers();
		m_trackerPoseCalibrator->renderCameraSpaceCalibrationState();
	}
	else if (m_menuState == eMenuState::testCalibration)
	{
		m_monoDistortionView->renderSelectedVideoBuffers();
		renderVRScene();
	}
}

void AppStage_AlignmentCalibration::renderVRScene()
{
	m_scene->render();

	drawTransformedAxes(glm::mat4(1.f), 1.0f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(style, glm::vec3(1.f, 0.f, 0.f), L"X");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 1.f, 0.f), L"Y");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 1.f), L"Z");
}

void AppStage_AlignmentCalibration::renderCameraSettingsUI()
{
	ProfileConfig* config= App::getInstance()->getProfileConfig();
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;

	const float kPanelHeight= (m_menuState == eMenuState::verifySetup) ? 150.f : 100.f;
	ImGui::SetNextWindowPos(ImVec2(10.f, ImGui::GetIO().DisplaySize.y - kPanelHeight - 10.f));
	ImGui::SetNextWindowSize(ImVec2(275, kPanelHeight));
	ImGui::Begin("Video Controls", nullptr, window_flags);

	const int brightnessStep = m_videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);
	if (ImGui::Button("-##Brightness"))
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_uiBrightness - brightnessStep, false);
		m_uiBrightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	}
	ImGui::SameLine();
	if (ImGui::Button("+##Brightness"))
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_uiBrightness + brightnessStep, false);
		m_uiBrightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	}
	ImGui::SameLine();
	ImGui::Text("Brightness: %d", m_uiBrightness);

	if (ImGui::Button("-##FrameDelay"))
	{
		config->vrFrameDelay= int_max(config->vrFrameDelay - 1, 0);
	}
	ImGui::SameLine();
	if (ImGui::Button("+##FrameDelay"))
	{
		config->vrFrameDelay = int_min(config->vrFrameDelay + 1, 100);
	}
	ImGui::SameLine();
	ImGui::Text("VR Frame Delay: %d", config->vrFrameDelay);

	if (m_menuState == eMenuState::verifySetup)
	{
		if (ImGui::RadioButton("Camera Viewpoint", m_viewMode == eViewpointMode::cameraViewpoint))
		{
			setViewpointMode(eViewpointMode::cameraViewpoint);
		}
		else if (ImGui::RadioButton("VR Viewpoint", m_viewMode == eViewpointMode::vrViewpoint))
		{
			setViewpointMode(eViewpointMode::vrViewpoint);
		}
		else if (ImGui::RadioButton("Mixed Reality", m_viewMode == eViewpointMode::mixedRealityViewpoint))
		{
			setViewpointMode(eViewpointMode::mixedRealityViewpoint);
		}
	}

	ImGui::End();
}

void AppStage_AlignmentCalibration::renderUI()
{
	const float k_panel_width = 200.f;
	const char* k_window_title = "Alignment Calibration";
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;

	switch (m_menuState)
	{
	case eMenuState::verifySetup:
	{
		renderCameraSettingsUI();

		{
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.f - k_panel_width / 2.f, 20.f));
			ImGui::SetNextWindowSize(ImVec2(k_panel_width, 110));
			ImGui::Begin(k_window_title, nullptr, window_flags);

			ImGui::Text("Place the mat tracking puck on the mat. Hit begin to start the calibration.");

			if (ImGui::Button("Begin"))
			{
				// Clear out all of the calibration data we recorded
				m_trackerPoseCalibrator->resetCalibrationState();

				// Move on to the capture state
				m_menuState= eMenuState::capture;

				// Go back to the camera viewpoint (in case we are in VR view)
				setViewpointMode(eViewpointMode::cameraViewpoint);
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_app->popAppState();
			}

			ImGui::End();
		}

	} break;

	case eMenuState::capture:
	{
		assert(m_trackerPoseCalibrator != nullptr);

		renderCameraSettingsUI();

		{
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.f - k_panel_width / 2.f, 20.f));
			ImGui::SetNextWindowSize(ImVec2(k_panel_width, 110));
			ImGui::Begin(k_window_title, nullptr, window_flags);

			ImGui::ProgressBar(m_trackerPoseCalibrator->getCalibrationProgress(), ImVec2(k_panel_width - 20, 20));

			if (ImGui::Button("Restart"))
			{
				// Clear out all of the calibration data we recorded
				m_trackerPoseCalibrator->resetCalibrationState();

				// Move back to the wait for ready state
				m_menuState = eMenuState::verifySetup;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_app->popAppState();
			}

			ImGui::End();
		}
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
		}

		if (!m_bypassCalibrationFlag)
		{
			if (ImGui::Button("Redo Calibration"))
			{
				// Clear out all of the calibration data we recorded
				m_trackerPoseCalibrator->resetCalibrationState();

				m_monoDistortionView->setGrayscaleUndistortDisabled(false);
				m_menuState = eMenuState::verifySetup;
			}
			ImGui::SameLine();
		}
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