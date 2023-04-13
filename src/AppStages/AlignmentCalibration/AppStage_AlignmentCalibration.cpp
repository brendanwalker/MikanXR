// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "AlignmentCalibration/RmlModel_AlignmentCalibration.h"
#include "AlignmentCalibration/RmlModel_AlignmentCameraSettings.h"
#include "App.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlScene.h"
#include "GlTextRenderer.h"
#include "GlViewport.h"
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

#include "glm/gtc/quaternion.hpp"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----
const char* AppStage_AlignmentCalibration::APP_STAGE_NAME = "AlignmentCalibration";

//-- constants -----
static const char* k_calibration_pattern_names[] = {
	"Chessboard",
	"Circle Grid",
};


//-- public methods -----
AppStage_AlignmentCalibration::AppStage_AlignmentCalibration(App* app)
	: AppStage(app, AppStage_AlignmentCalibration::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_AlignmentCalibration)
	, m_cameraSettingsModel(new RmlModel_AlignmentCameraSettings)
	, m_videoSourceView()
	, m_trackerPoseCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
	, m_scene(new GlScene)
	, m_camera(nullptr)
{
}

AppStage_AlignmentCalibration::~AppStage_AlignmentCalibration()
{
	delete m_calibrationModel;
	delete m_cameraSettingsModel;
	delete m_scene;
}

void AppStage_AlignmentCalibration::setBypassCalibrationFlag(bool flag)
{
	m_calibrationModel->setBypassCalibrationFlag(flag);
}

void AppStage_AlignmentCalibration::enter()
{
	AppStage::enter();

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

	// Fetch the new camera associated with the viewport
	m_camera= getFirstViewport()->getCurrentCamera();

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Fire up the video scene in the background + pose calibrator
	eAlignmentCalibrationMenuState newState;
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

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			newState = eAlignmentCalibrationMenuState::testCalibration;
			m_monoDistortionView->setGrayscaleUndistortDisabled(true);
		}
		else
		{
			newState = eAlignmentCalibrationMenuState::verifySetup;
			m_monoDistortionView->setGrayscaleUndistortDisabled(false);
		}
	}
	else
	{
		newState = eAlignmentCalibrationMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnBeginEvent = MakeDelegate(this, &AppStage_AlignmentCalibration::onBeginEvent);
		m_calibrationModel->OnRestartEvent = MakeDelegate(this, &AppStage_AlignmentCalibration::onRestartEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_AlignmentCalibration::onCancelEvent);
		m_calibrationModel->OnReturnEvent = MakeDelegate(this, &AppStage_AlignmentCalibration::onReturnEvent);

		// Init camera settings model
		m_cameraSettingsModel->init(context, m_videoSourceView, profileConfig);
		m_cameraSettingsModel->OnViewpointModeChanged = MakeDelegate(this, &AppStage_AlignmentCalibration::onViewportModeChanged);
		m_cameraSettingsModel->OnBrightnessChanged = MakeDelegate(this, &AppStage_AlignmentCalibration::onBrightnessChanged);
		m_cameraSettingsModel->OnVRFrameDelayChanged = MakeDelegate(this, &AppStage_AlignmentCalibration::onVRFrameDelayChanged);
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::mixedRealityViewpoint);
		}
		else
		{
			m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);
		}

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("alignment_calibration.rml");

		// Init camera settings view now that the dependent model has been created
		m_cameraSettingsView = addRmlDocument("alignment_camera_settings.rml");
	}

	setMenuState(newState);
}

void AppStage_AlignmentCalibration::exit()
{
	setMenuState(eAlignmentCalibrationMenuState::inactive);

	m_camera= nullptr;

	VRDeviceList vrDeviceList = VRDeviceManager::getInstance()->getVRDeviceList();
	for (auto it : vrDeviceList)
	{
		it->getVRDeviceInterface()->removeFromBoundScene();
	}

	if (m_videoSourceView)
	{
		if (m_bHasModifiedCameraSettings)
		{
			// Save updated camera settings:
			// * intrinsics and distortion coefficients if we completed calibration
			// * VR frame delay if it was modified in the UI
			m_videoSourceView->saveSettings();
		}
		else
		{
			// Restore video source settings back to what was saved 
			m_videoSourceView->loadSettings();
		}

		// Turn back off the video feed
		m_videoSourceView->stopVideoStream();
		m_videoSourceView = nullptr;
	}

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

	AppStage::exit();
}

void AppStage_AlignmentCalibration::updateCamera()
{
	switch (m_cameraSettingsModel->getViewpointMode())
	{
	case eAlignmentCalibrationViewpointMode::cameraViewpoint:
		{
			m_camera->setModelViewMatrix(glm::mat4(1.f));
		}
		break;
	case eAlignmentCalibrationViewpointMode::vrViewpoint:
		{
			m_camera->recomputeModelViewMatrix();
		}
		break;
	case eAlignmentCalibrationViewpointMode::mixedRealityViewpoint:
		{
			// Update the transform of the camera so that vr models align over the tracking puck
			glm::mat4 cameraPose;
			if (m_calibrationModel->getMenuState() == eAlignmentCalibrationMenuState::testCalibration)
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

	switch(m_calibrationModel->getMenuState())
	{
		case eAlignmentCalibrationMenuState::verifySetup:
			{
				// Update the video frame buffers to preview the calibration mat
				m_monoDistortionView->readAndProcessVideoFrame();

				// Look for a calibration pattern so that we can preview if it's in frame
				m_trackerPoseCalibrator->computeCameraToPuckXform();
			}
			break;
		case eAlignmentCalibrationMenuState::capture:
			{
				// Update the video frame buffers
				m_monoDistortionView->readAndProcessVideoFrame();

				// Update the chess board capture state
				if (m_trackerPoseCalibrator->computeCameraToPuckXform())
				{
					m_trackerPoseCalibrator->sampleLastCameraToPuckXform();

					// Update the calibration fraction on the UI Model
					m_calibrationModel->setCalibrationFraction(m_trackerPoseCalibrator->getCalibrationProgress());
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
						// Store the calibrated camera offset on the video source settings
						m_videoSourceView->setCameraPoseOffset(rotationOffset, translationOffset);

						// Flag that calibration has modified camera pose
						// (Used to decide if we should save settings on state exit)
						m_bHasModifiedCameraSettings = true;

						// Go to the test calibration state
						m_monoDistortionView->setGrayscaleUndistortDisabled(true);
						m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::mixedRealityViewpoint);
						setMenuState(eAlignmentCalibrationMenuState::testCalibration);
					}
				}
			}
			break;
		case eAlignmentCalibrationMenuState::testCalibration:
			{
				// Update the video frame buffers using the existing distortion calibration
				m_monoDistortionView->readAndProcessVideoFrame();
			}
			break;
	}
}

void AppStage_AlignmentCalibration::render()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	switch (m_calibrationModel->getMenuState())
	{
		case eAlignmentCalibrationMenuState::verifySetup:
			{
				switch (m_cameraSettingsModel->getViewpointMode())
				{
					case eAlignmentCalibrationViewpointMode::cameraViewpoint:
						m_monoDistortionView->renderSelectedVideoBuffers();
						m_trackerPoseCalibrator->renderCameraSpaceCalibrationState();
						break;
					case eAlignmentCalibrationViewpointMode::vrViewpoint:
						m_trackerPoseCalibrator->renderVRSpaceCalibrationState();
						renderVRScene();
						break;
					case eAlignmentCalibrationViewpointMode::mixedRealityViewpoint:
						m_monoDistortionView->renderSelectedVideoBuffers();
						renderVRScene();
						break;
				}
			}
			break;
		case eAlignmentCalibrationMenuState::capture:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_trackerPoseCalibrator->renderCameraSpaceCalibrationState();
			}
			break;
		case eAlignmentCalibrationMenuState::testCalibration:
			{
				if (m_cameraSettingsModel->getViewpointMode() == eAlignmentCalibrationViewpointMode::mixedRealityViewpoint)
				{
					m_monoDistortionView->renderSelectedVideoBuffers();
				}

				renderVRScene();
			}
			break;
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

void AppStage_AlignmentCalibration::setMenuState(eAlignmentCalibrationMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
		m_cameraSettingsModel->setMenuState(newState);

		// Show or hide the camera controls based on menu state
		const bool bIsCameraSettingsVisible = m_cameraSettingsView->IsVisible();
		const bool bWantCameraSettingsVisibility =
			(newState == eAlignmentCalibrationMenuState::capture) ||
			(newState == eAlignmentCalibrationMenuState::testCalibration);
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
	}
}

// Calibration Model UI Events
void AppStage_AlignmentCalibration::onBeginEvent()
{
	// Clear out all of the calibration data we recorded
	m_trackerPoseCalibrator->resetCalibrationState();

	// Reset all calibration state on the calibration UI model
	m_calibrationModel->setCalibrationFraction(0.f);

	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);

	// Advance to the capture state
	setMenuState(eAlignmentCalibrationMenuState::capture);
}

void AppStage_AlignmentCalibration::onRestartEvent()
{
	// Clear out all of the calibration data we recorded
	m_trackerPoseCalibrator->resetCalibrationState();

	// Reset all calibration state on the calibration UI model
	m_calibrationModel->setCalibrationFraction(0.f);

	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);

	// Re-enable gray scale undistort mode
	m_monoDistortionView->setGrayscaleUndistortDisabled(false);

	// Return to the capture state
	setMenuState(eAlignmentCalibrationMenuState::verifySetup);
}

void AppStage_AlignmentCalibration::onCancelEvent()
{
	m_app->popAppState();
}

void AppStage_AlignmentCalibration::onReturnEvent()
{
	m_app->popAppState();
}

// Camera Settings Model UI Events
void AppStage_AlignmentCalibration::onViewportModeChanged(eAlignmentCalibrationViewpointMode newViewMode)
{
	switch (newViewMode)
	{
		case eAlignmentCalibrationViewpointMode::cameraViewpoint:
			m_camera->setIsLocked(true);
			break;
		case eAlignmentCalibrationViewpointMode::vrViewpoint:
			m_camera->setIsLocked(false);
			break;
		case eAlignmentCalibrationViewpointMode::mixedRealityViewpoint:
			m_camera->setIsLocked(true);
			break;
		default:
			break;
	}
}

void AppStage_AlignmentCalibration::onBrightnessChanged(int newBrightness)
{
	m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, newBrightness, false);
}

void AppStage_AlignmentCalibration::onVRFrameDelayChanged(int newVRFrameDelay)
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	profileConfig->vrFrameDelay = newVRFrameDelay;
	m_bHasModifiedCameraSettings = true;
}