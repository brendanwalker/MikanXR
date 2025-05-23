// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "MonoLensCalibration/RmlModel_MonoLensCalibration.h"
#include "MonoLensCalibration/RmlModel_MonoCameraSettings.h"
#include "App.h"
#include "InputManager.h"
#include "MainWindow.h"
#include "MonoLensDistortionCalibrator.h"
#include "CalibrationPatternFinder.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VideoFrameDistortionView.h"

#include "SDL_keycode.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>


//-- statics ----
const char* AppStage_MonoLensCalibration::APP_STAGE_NAME = "MonoCalibration";

const std::string g_monoLensCalibrationMenuStateStrings[(int)eMonoLensCalibrationMenuState::COUNT] = {
	"inactive",
	"capture",
	"processingCalibration",
	"testCalibration",
	"failedCalibration",
	"failedVideoStartStreamRequest",
};

//-- public methods -----
AppStage_MonoLensCalibration::AppStage_MonoLensCalibration(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_MonoLensCalibration::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_MonoLensCalibration)
	, m_cameraSettingsModel(new RmlModel_MonoCameraSettings)
	, m_videoSourceView()
	, m_monoLensCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
{
}

AppStage_MonoLensCalibration::~AppStage_MonoLensCalibration()
{
	delete m_calibrationModel;
	delete m_cameraSettingsModel;
}

void AppStage_MonoLensCalibration::setBypassCalibrationFlag(bool flag)
{ 
	m_calibrationModel->setBypassCalibrationFlag(true);
}

void AppStage_MonoLensCalibration::enter()
{
	AppStage::enter();

	// Bind to space bar to capture frames
	// (Auto cleared on AppStage exit)
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_SPACE)->OnKeyPressed +=
		MakeDelegate(this, &AppStage_MonoLensCalibration::onCaptureKeyPressed);

	// Get the current video source based on the config
	ProfileConfigConstPtr profileConfig= App::getInstance()->getProfileConfig();
	m_videoSourceView= VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();

	// Initialize video stream + lens calibrator
	eMonoLensCalibrationMenuState newState;
	//TODO: Handle pendingStart
	if ((int)m_videoSourceView->startVideoStream() > 0)
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = new VideoFrameDistortionView(
			m_ownerWindow,
			m_videoSourceView, 
			VIDEO_FRAME_HAS_ALL);

		// Create a calibrator to do the actual pattern recording and calibration
		m_monoLensCalibrator =
			new MonoLensDistortionCalibrator(
				profileConfig,
				m_monoDistortionView,
				DESIRED_CAPTURE_BOARD_COUNT);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			newState= eMonoLensCalibrationMenuState::testCalibration;
			m_monoDistortionView->setGrayscaleUndistortDisabled(false);
		}
		else
		{
			newState= eMonoLensCalibrationMenuState::capture;
			m_monoDistortionView->setGrayscaleUndistortDisabled(true);
		}
	}
	else
	{
		newState= eMonoLensCalibrationMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_MonoLensCalibration::onCancelEvent);
		m_calibrationModel->OnRestartEvent = MakeDelegate(this, &AppStage_MonoLensCalibration::onRestartEvent);
		m_calibrationModel->OnReturnEvent = MakeDelegate(this, &AppStage_MonoLensCalibration::onReturnEvent);
		m_calibrationModel->OnImagePointStabilityChangedEvent = 
			MakeDelegate(this, &AppStage_MonoLensCalibration::onImagePointStabilityChangedEvent);

		// Init camera settings model
		m_cameraSettingsModel->init(context);
		m_cameraSettingsModel->OnVideoDisplayModeChanged = MakeDelegate(this, &AppStage_MonoLensCalibration::onVideoDisplayModeChanged);

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("mono_lens_calibration.rml");

		// Init camera settings view now that the dependent model has been created
		m_cameraSettingsView = addRmlDocument("mono_camera_settings.rml");
	}

	setMenuState(newState);
}

void AppStage_MonoLensCalibration::exit()
{
	setMenuState(eMonoLensCalibrationMenuState::inactive);

	// Save all modified camera config values 
	// (i.e. camera intrinsics and distortion coefficients)
	if (!m_calibrationModel->getBypassCalibrationFlag())
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

	AppStage::exit();
}

void AppStage_MonoLensCalibration::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update data bindings on child models
	m_calibrationModel->update();
	m_cameraSettingsModel->update();

	// Update the calibration state machine
	switch (m_calibrationModel->getMenuState())
	{
		case eMonoLensCalibrationMenuState::inactive:
			{

			} break;

		case eMonoLensCalibrationMenuState::capture:
			{
				assert(m_monoLensCalibrator != nullptr);

				// Update calibration progress UI data binding
				m_calibrationModel->setCalibrationFraction(m_monoLensCalibrator->computeCalibrationProgress());

				// Update image points valid flag UI data binding
				m_calibrationModel->setCurrentImagePointsValid(m_monoLensCalibrator->areCurrentImagePointsValid());

				// Update the image point stability timer / flag
				m_calibrationModel->updateImagePointStabilityTimer(deltaSeconds);

				// Update the video frame buffers
				m_monoDistortionView->readAndProcessVideoFrame();

				// Update the chess board capture state
				assert(m_monoDistortionView->isGrayscaleUndistortDisabled());
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
						cameraIntrinsics.makeMonoIntrinsics()= new_mono_intrinsics;
						m_videoSourceView->setCameraIntrinsics(cameraIntrinsics);

						// Rebuild the distortion map to reflect the updated calibration
						m_monoDistortionView->applyMonoCameraIntrinsics(&new_mono_intrinsics);
						m_monoDistortionView->setGrayscaleUndistortDisabled(false);
						m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_bgr);

						// Switch back to the color video feed
						m_cameraSettingsModel->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

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
				// Update reprojection error in the UI data binding
				m_calibrationModel->setReprojectionError(m_monoLensCalibrator->getReprojectionError());

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

void AppStage_MonoLensCalibration::render()
{
	eMonoLensCalibrationMenuState menuState = m_calibrationModel->getMenuState();

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

void AppStage_MonoLensCalibration::onCaptureKeyPressed()
{
	tryCapture();
}

bool AppStage_MonoLensCalibration::tryCapture()
{
	eMonoLensCalibrationMenuState menuState = m_calibrationModel->getMenuState();

	if (menuState == eMonoLensCalibrationMenuState::capture)
	{
		// Update the chess board capture state
		if (m_monoLensCalibrator != nullptr)
		{
			return m_monoLensCalibrator->captureLastFoundCalibrationPattern();
		}
	}

	return false;
}

void AppStage_MonoLensCalibration::setMenuState(eMonoLensCalibrationMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		eMonoLensCalibrationMenuState oldState= m_calibrationModel->getMenuState();

		// Update menu state on the data model
		m_calibrationModel->setMenuState(newState);

		// Show or hide the camera controls based on menu state
		const bool bIsCameraSettingsVisible = m_cameraSettingsView->IsVisible();
		const bool bWantCameraSettingsVisibility =
			(newState == eMonoLensCalibrationMenuState::capture) ||
			(newState == eMonoLensCalibrationMenuState::testCalibration);
		if (bWantCameraSettingsVisibility != bIsCameraSettingsVisible)
		{
			if (bWantCameraSettingsVisibility)
			{
				m_cameraSettingsView->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);
			}
			else
			{
				m_cameraSettingsView->Hide();
			}
		}

		// Broadcast the menu state change to the remote control manager
		{
			std::vector<std::string> parameters;
			parameters.push_back(g_monoLensCalibrationMenuStateStrings[(int)oldState]);
			parameters.push_back(g_monoLensCalibrationMenuStateStrings[(int)newState]);

			sendRemoteControlEvent("menu_state_changed", parameters);
		}
	}
}

// Calibration Model UI Events
void AppStage_MonoLensCalibration::onRestartEvent()
{
	// Clear out data model calibration state
	m_calibrationModel->resetCalibrationState();

	// Clear out all of the calibration data we recorded
	m_monoLensCalibrator->resetCalibrationState();

	// Reset the distortion map back to the camera intrinsics we started with
	m_monoLensCalibrator->resetDistortionView();

	// Turn back off grayscale undistortion mode
	m_monoDistortionView->setGrayscaleUndistortDisabled(false);

	// Go back to the capture state
	setMenuState(eMonoLensCalibrationMenuState::capture);
}

void AppStage_MonoLensCalibration::onReturnEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_MonoLensCalibration::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_MonoLensCalibration::onImagePointStabilityChangedEvent(bool bIsStable)
{
	std::vector<std::string> parameters;
	parameters.push_back(bIsStable ? "true" : "false");

	sendRemoteControlEvent("image_point_stability_changed", parameters);
}

// Camera Settings Model UI Events
void AppStage_MonoLensCalibration::onVideoDisplayModeChanged(eVideoDisplayMode newDisplayMode)
{
	m_monoDistortionView->setVideoDisplayMode(newDisplayMode);
}

bool AppStage_MonoLensCalibration::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	if (!IRemoteControllableAppStage::handleRemoteControlCommand(command, parameters, outResults))
	{
		if (command == "get_state")
		{
			return handleGetStateCommand(outResults);
		}
		else if (command == "get_image_point_stability")
		{
			return handleGetImagePointStabilityCommand(outResults);
		}
		else if (command == "get_samples_needed")
		{
			return handleGetSamplesNeededCommand(outResults);
		}
		else if (command == "capture")
		{
			return handleCaptureCommand(outResults);
		}
	}

	return false;
}

bool AppStage_MonoLensCalibration::handleGetStateCommand(
	std::vector<std::string>& outResults)
{
	const eMonoLensCalibrationMenuState menuState = m_calibrationModel->getMenuState();
	const std::string& stateName= g_monoLensCalibrationMenuStateStrings[(int)menuState];

	outResults.push_back(stateName);

	return true;
}

bool AppStage_MonoLensCalibration::handleGetImagePointStabilityCommand(
	std::vector<std::string>& outResults)
{
	const bool bIsStable = m_monoLensCalibrator->getIsCameraCalibrationComplete();
	outResults.push_back(bIsStable ? "true" : "false");

	return true;
}

bool AppStage_MonoLensCalibration::handleGetSamplesNeededCommand(
	std::vector<std::string>& outResults)
{
	const int samplesNeeded = m_monoLensCalibrator->getDesiredPatternCount();
	outResults.push_back(std::to_string(samplesNeeded));

	return true;
}

bool AppStage_MonoLensCalibration::handleCaptureCommand(std::vector<std::string>& outResults)
{
	if (tryCapture())
	{
		outResults.push_back("success");
	}
	else
	{
		outResults.push_back("failure");
	}

	float calibrationFraction = m_calibrationModel->getCalibrationFraction();
	outResults.push_back(std::to_string(calibrationFraction));

	return true;
}