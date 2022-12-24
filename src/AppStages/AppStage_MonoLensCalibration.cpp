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

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

struct MonoLensCalibrationDataModel
{
	Rml::DataModelHandle model_handle;

	// Menu state
	int menu_state = 0;
	bool are_current_image_points_valid= false;
	float calibration_progress= 0.f;
	float reprojection_error= 0.f;

	// Tracker Settings state
	Rml::Vector<Rml::String> video_display_modes = {"BGR", "Undistorted", "Grayscale"};
	int video_display_mode= (int)eVideoDisplayMode::mode_bgr;
	int brightness = 0;
	int brightness_min = 0;
	int brightness_max = 0;
	int brightness_step = 0;
	bool bypass_calibration_flag = false;

	void resetCalibrationState()
	{
		are_current_image_points_valid = false;
		model_handle.DirtyVariable("are_current_image_points_valid");

		calibration_progress = 0.f;
		model_handle.DirtyVariable("calibration_progress");

		reprojection_error = 0.f;
		model_handle.DirtyVariable("reprojection_error");
	}
};


//-- statics ----
const char* AppStage_MonoLensCalibration::APP_STAGE_NAME = "MonoCalibration";

//-- constants -----
#define DESIRED_CAPTURE_BOARD_COUNT 12
#define BOARD_NEW_LOCATION_PIXEL_DIST 100 

//-- public methods -----
AppStage_MonoLensCalibration::AppStage_MonoLensCalibration(App* app)
	: AppStage(app, AppStage_MonoLensCalibration::APP_STAGE_NAME)
	, m_dataModel(new MonoLensCalibrationDataModel)
	, m_videoSourceView()
	, m_monoLensCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
{
}

AppStage_MonoLensCalibration::~AppStage_MonoLensCalibration()
{
	delete m_dataModel;
}

void AppStage_MonoLensCalibration::setBypassCalibrationFlag(bool flag)
{ 
	m_dataModel->bypass_calibration_flag = flag; 
}

void AppStage_MonoLensCalibration::setMenuState(eMonoLensCalibrationMenuState newState)
{
	if (m_dataModel->menu_state != (int)newState)
	{
		// Update menu state on the data model
		m_dataModel->menu_state = (int)newState;
		m_dataModel->model_handle.DirtyVariable("menu_state");

		// Show or hide the camera controls based on menu state
		const bool bIsCameraSettingsVisible = m_cameraSettingsDoc->IsVisible();
		const bool bWantCameraSettingsVisibility= 
			(newState == eMonoLensCalibrationMenuState::capture) ||
			(newState == eMonoLensCalibrationMenuState::testCalibration);
		if (bWantCameraSettingsVisibility != bIsCameraSettingsVisible)
		{
			if (bWantCameraSettingsVisibility)
			{
				m_cameraSettingsDoc->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);
			}
			else
			{
				m_cameraSettingsDoc->Hide();
			}
		}
	}
}

void AppStage_MonoLensCalibration::enter()
{
	AppStage::enter();

	// Get the current video source based on the config
	const ProfileConfig* profileConfig= App::getInstance()->getProfileConfig();
	m_videoSourceView= VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();

	// Bind to space bar to capture frames
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_SPACE)->OnKeyPressed+= 
		MakeDelegate(this, &AppStage_MonoLensCalibration::captureRequested);

	// Create Datamodel
	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("mono_lens_calibration");
	if (!constructor)
		return;

	// Register Data Model Fields
	constructor.Bind("menu_state", &m_dataModel->menu_state);
	constructor.Bind("are_current_image_points_valid", &m_dataModel->are_current_image_points_valid);
	constructor.Bind("calibration_progress", &m_dataModel->calibration_progress);
	constructor.Bind("reprojection_error", &m_dataModel->reprojection_error);
	constructor.Bind("video_display_modes", &m_dataModel->video_display_modes);
	constructor.Bind("video_display_mode", &m_dataModel->video_display_mode);
	constructor.Bind("brightness", &m_dataModel->brightness);
	constructor.Bind("brightness_min", &m_dataModel->brightness_min);
	constructor.Bind("brightness_max", &m_dataModel->brightness_max);
	constructor.Bind("brightness_step", &m_dataModel->brightness_step);
	constructor.Bind("bypass_calibration_flag", &m_dataModel->bypass_calibration_flag);
	constructor.BindEventCallback(
		"restart",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			// Clear out all of the calibration data we recorded
			m_monoLensCalibrator->resetCalibrationState();
	
			// Reset the distortion map back to the camera intrinsics we started with
			m_monoLensCalibrator->resetDistortionView();
			
			// Clear out data model calibration state
			m_dataModel->resetCalibrationState();

			// Go back to the capture state
			setMenuState(eMonoLensCalibrationMenuState::capture);
		});
	constructor.BindEventCallback(
		"goto_main_menu",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			m_app->popAppState();
		});
	m_dataModel->model_handle = constructor.GetModelHandle();


	// Reset the model calibration vars
	m_dataModel->resetCalibrationState();

	eMonoLensCalibrationMenuState newState;
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

		// Fetch the initial video display mode
		m_dataModel->video_display_mode = m_monoDistortionView->getVideoDisplayMode();

		// Cache off the brightness properties
		m_dataModel->brightness_min = m_videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
		m_dataModel->brightness_max = m_videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
		m_dataModel->brightness_step = m_videoSourceView->getVideoPropertyConstraintStep(VideoPropertyType::Brightness);
		m_dataModel->brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_dataModel->bypass_calibration_flag)
		{
			newState= eMonoLensCalibrationMenuState::testCalibration;
		}
		else
		{
			newState= eMonoLensCalibrationMenuState::capture;
		}
	}
	else
	{
		newState= eMonoLensCalibrationMenuState::failedVideoStartStreamRequest;
	}

	addRmlDocument("rml\\mono_lens_calibration.rml");
	m_cameraSettingsDoc= addRmlDocument("rml\\mono_lens_camera_settings.rml");

	setMenuState(newState);
}

void AppStage_MonoLensCalibration::exit()
{
	setMenuState(eMonoLensCalibrationMenuState::inactive);

	// Save all modified camera config values 
	// (i.e. camera intrinsics and distortion coefficients)
	if (!m_dataModel->bypass_calibration_flag)
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

	// Clean up the data model
	getRmlContext()->RemoveDataModel("mono_lens_calibration");

	AppStage::exit();
}

void AppStage_MonoLensCalibration::update()
{
	switch ((eMonoLensCalibrationMenuState)m_dataModel->menu_state)
	{
		case eMonoLensCalibrationMenuState::inactive:
			{

			} break;

		case eMonoLensCalibrationMenuState::capture:
			{
				assert(m_monoLensCalibrator != nullptr);

				// Update video display property data bindings
				updateVideoDisplayProperties();

				// Update calibration progress data binding
				const float newProgress= m_monoLensCalibrator->computeCalibrationProgress() * 100.f;
				if (m_dataModel->calibration_progress != newProgress)
				{
					m_dataModel->calibration_progress = newProgress;
					m_dataModel->model_handle.DirtyVariable("calibration_progress");
				}

				// Update image points valid flag data binding
				const bool bNewImagePointsValid= m_monoLensCalibrator->areCurrentImagePointsValid();
				if (m_dataModel->are_current_image_points_valid != bNewImagePointsValid)
				{
					m_dataModel->are_current_image_points_valid = bNewImagePointsValid;
					m_dataModel->model_handle.DirtyVariable("are_current_image_points_valid");
				}

				// Update the video frame buffers
				m_monoDistortionView->readAndProcessVideoFrame();

				// Update the chess board capture state
				m_monoLensCalibrator->findNewCalibrationPattern(BOARD_NEW_LOCATION_PIXEL_DIST);

				// See if we have gotten all the samples we require
				if (m_monoLensCalibrator->hasSampledAllCalibrationPatterns())
				{
					// Kick off the async task (very expensive)
					m_monoLensCalibrator->computeCameraCalibration();
					setMenuState(eMonoLensCalibrationMenuState::processingCalibration);
				}
			} break;

		case eMonoLensCalibrationMenuState::processingCalibration:
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
						setMenuState(eMonoLensCalibrationMenuState::testCalibration);
					}
					else
					{
						setMenuState(eMonoLensCalibrationMenuState::failedCalibration);
					}
				}
			} break;

		case eMonoLensCalibrationMenuState::testCalibration:
			{
				updateVideoDisplayProperties();

				// Update reprojection error
				const float newReprojectionError= m_monoLensCalibrator->getReprojectionError();
				if (newReprojectionError != m_dataModel->reprojection_error)
				{
					m_dataModel->reprojection_error = newReprojectionError;
					m_dataModel->model_handle.DirtyVariable("reprojection_error");
				}

				// Update the video frame buffers using the existing distortion calibration
				m_monoDistortionView->readAndProcessVideoFrame();
			} break;

		case eMonoLensCalibrationMenuState::failedCalibration:
			{

			} break;

		case eMonoLensCalibrationMenuState::failedVideoStartStreamRequest:
			{

			} break;

		default:
			assert(0 && "unreachable");
	}
}

void AppStage_MonoLensCalibration::updateVideoDisplayProperties()
{
	if (m_dataModel->model_handle.IsVariableDirty("video_display_mode"))
	{
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode(m_dataModel->video_display_mode));
	}
	if (m_dataModel->model_handle.IsVariableDirty("brightness"))
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_dataModel->brightness, true);
		m_dataModel->brightness = m_videoSourceView->getVideoProperty(VideoPropertyType::Brightness);
	}
}

void AppStage_MonoLensCalibration::render()
{
	eMonoLensCalibrationMenuState menuState = (eMonoLensCalibrationMenuState)m_dataModel->menu_state;

	if (menuState == eMonoLensCalibrationMenuState::capture)
	{
		m_monoDistortionView->renderSelectedVideoBuffers();
		m_monoLensCalibrator->renderCalibrationState();
	}
	else if (menuState == eMonoLensCalibrationMenuState::testCalibration)
	{
		m_monoDistortionView->renderSelectedVideoBuffers();
	}
}

void AppStage_MonoLensCalibration::captureRequested()
{
	eMonoLensCalibrationMenuState menuState = (eMonoLensCalibrationMenuState)m_dataModel->menu_state;

	if (menuState == eMonoLensCalibrationMenuState::capture)
	{
		// Update the chess board capture state
		if (m_monoLensCalibrator != nullptr)
		{
			m_monoLensCalibrator->captureLastFoundCalibrationPattern();
		}
	}
}