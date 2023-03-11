// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "FastenerCalibration/AppStage_FastenerCalibration.h"
#include "FastenerCalibration/RmlModel_FastenerCalibration.h"
#include "FastenerCalibration/RmlModel_FastenerCameraSettings.h"
#include "App.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlScene.h"
#include "GlTextRenderer.h"
#include "InputManager.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "FastenerCalibrator.h"
#include "ProfileConfig.h"
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
const char* AppStage_FastenerCalibration::APP_STAGE_NAME = "FastenerCalibration";

//-- public methods -----
AppStage_FastenerCalibration::AppStage_FastenerCalibration(App* app)
	: AppStage(app, AppStage_FastenerCalibration::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_FastenerCalibration)
	, m_cameraSettingsModel(new RmlModel_FastenerCameraSettings)
	, m_videoSourceView()
	, m_fastenerCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
	, m_targetFastenerId(INVALID_MIKAN_ID)
	, m_scene(new GlScene)
	, m_camera(nullptr)
{
}

AppStage_FastenerCalibration::~AppStage_FastenerCalibration()
{
	delete m_calibrationModel;
	delete m_cameraSettingsModel;
	delete m_scene;
}

void AppStage_FastenerCalibration::setBypassCalibrationFlag(bool flag)
{
	m_calibrationModel->setBypassCalibrationFlag(flag);
}

void AppStage_FastenerCalibration::enter()
{
	AppStage::enter();

	// Get the current video source based on the config
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
	m_cameraTrackingPuckView =
		VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->cameraVRDevicePath).getCurrent();

	// Create a new camera to view the scene
	m_camera = App::getInstance()->getRenderer()->pushCamera();
	m_camera->bindInput();

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Fire up the video scene in the background + pose calibrator
	eFastenerCalibrationMenuState newState;
	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_videoSourceView, 
				VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);		

		// Create a calibrator to do the actual pattern recording and calibration
		m_fastenerCalibrator =
			new FastenerCalibrator(
				profileConfig,
				m_cameraTrackingPuckView,
				m_monoDistortionView);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			newState = eFastenerCalibrationMenuState::testCalibration;
		}
		else
		{
			newState = eFastenerCalibrationMenuState::verifySetup1;
		}
	}
	else
	{
		newState = eFastenerCalibrationMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnContinueEvent = MakeDelegate(this, &AppStage_FastenerCalibration::onContinueEvent);
		m_calibrationModel->OnRestartEvent = MakeDelegate(this, &AppStage_FastenerCalibration::onRestartEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_FastenerCalibration::onCancelEvent);
		m_calibrationModel->OnReturnEvent = MakeDelegate(this, &AppStage_FastenerCalibration::onReturnEvent);

		// Init camera settings model
		m_cameraSettingsModel->init(context, m_videoSourceView, profileConfig);
		m_cameraSettingsModel->OnViewpointModeChanged = MakeDelegate(this, &AppStage_FastenerCalibration::onViewportModeChanged);
		m_cameraSettingsModel->OnBrightnessChanged = MakeDelegate(this, &AppStage_FastenerCalibration::onBrightnessChanged);
		m_cameraSettingsModel->setViewpointMode(eFastenerCalibrationViewpointMode::cameraViewpoint);

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("fastener_calibration.rml");

		// Init camera settings view now that the dependent model has been created
		m_cameraSettingsView = addRmlDocument("fastener_camera_settings.rml");
	}

	// Bind to space bar to capture frames
	// (Auto cleared on AppStage exit)
	{
		EventBindingSet* bindingSet = InputManager::getInstance()->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonReleasedEvent += MakeDelegate(this, &AppStage_FastenerCalibration::onMouseButtonUp);
	}

	setMenuState(newState);
}

void AppStage_FastenerCalibration::exit()
{
	setMenuState(eFastenerCalibrationMenuState::inactive);

	App::getInstance()->getRenderer()->popCamera();
	m_camera= nullptr;

	if (m_videoSourceView)
	{
		// Turn back off the video feed
		m_videoSourceView->stopVideoStream();
		m_videoSourceView = nullptr;
	}

	// Free the calibrator
	if (m_fastenerCalibrator != nullptr)
	{
		delete m_fastenerCalibrator;
		m_fastenerCalibrator = nullptr;
	}

	// Free the distortion view buffers
	if (m_monoDistortionView != nullptr)
	{
		delete m_monoDistortionView;
		m_monoDistortionView = nullptr;
	}

	AppStage::exit();
}

void AppStage_FastenerCalibration::updateCamera()
{
	switch (m_cameraSettingsModel->getViewpointMode())
	{
	case eFastenerCalibrationViewpointMode::cameraViewpoint:
		{
			m_camera->setModelViewMatrix(glm::mat4(1.f));
		}
		break;
	case eFastenerCalibrationViewpointMode::vrViewpoint:
		{
			m_camera->recomputeModelViewMatrix();
		}
		break;
	}
}

void AppStage_FastenerCalibration::update()
{
	updateCamera();

	const eFastenerCalibrationMenuState currentMenuState= m_calibrationModel->getMenuState();
	eFastenerCalibrationMenuState nextMenuState= currentMenuState;

	switch(m_calibrationModel->getMenuState())
	{
		case eFastenerCalibrationMenuState::verifySetup1:
		case eFastenerCalibrationMenuState::verifySetup2:
			{
				// Update the video frame buffers to preview the calibration mat
				m_monoDistortionView->readAndProcessVideoFrame();
			}
			break;
		case eFastenerCalibrationMenuState::capture1:
		case eFastenerCalibrationMenuState::capture2:
			{
				// Update the video frame buffers
				m_monoDistortionView->readAndProcessVideoFrame();

				// See if we have gotten all the samples we require
				if (m_fastenerCalibrator->hasFinishedSampling())
				{
					m_fastenerCalibrator->sampleCameraPose();

					if (currentMenuState == eFastenerCalibrationMenuState::capture1)
					{
						nextMenuState = eFastenerCalibrationMenuState::verifySetup2;
					}
					else
					{
						ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

						MikanSpatialFastenerInfo fastener;
						if (profileConfig->getSpatialFastenerInfo(m_targetFastenerId, fastener))
						{
							m_fastenerCalibrator->computeFastenerPoints(&fastener);
							profileConfig->updateFastener(fastener);
						}

						nextMenuState = eFastenerCalibrationMenuState::testCalibration;
					}
				}
			}
			break;
		case eFastenerCalibrationMenuState::testCalibration:
			{
				// Update the video frame buffers using the existing distortion calibration
				m_monoDistortionView->readAndProcessVideoFrame();
			}
			break;
	}

	if (nextMenuState != currentMenuState)
	{
		setMenuState(nextMenuState);
	}
}

void AppStage_FastenerCalibration::render()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	switch (m_calibrationModel->getMenuState())
	{
		case eFastenerCalibrationMenuState::verifySetup1:
		case eFastenerCalibrationMenuState::verifySetup2:
			{
				switch (m_cameraSettingsModel->getViewpointMode())
				{
					case eFastenerCalibrationViewpointMode::cameraViewpoint:
						m_monoDistortionView->renderSelectedVideoBuffers();
						m_fastenerCalibrator->renderCameraSpaceCalibrationState();
						break;
					case eFastenerCalibrationViewpointMode::vrViewpoint:
						m_fastenerCalibrator->renderVRSpaceCalibrationState();
						renderVRScene();
						break;
				}
			}
			break;
		case eFastenerCalibrationMenuState::capture1:
		case eFastenerCalibrationMenuState::capture2:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_fastenerCalibrator->renderCameraSpaceCalibrationState();
			}
			break;
		case eFastenerCalibrationMenuState::testCalibration:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				renderVRScene();
			}
			break;
	}
}

void AppStage_FastenerCalibration::renderVRScene()
{
	m_scene->render();

	drawTransformedAxes(glm::mat4(1.f), 1.0f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(style, glm::vec3(1.f, 0.f, 0.f), L"X");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 1.f, 0.f), L"Y");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 1.f), L"Z");
}

void AppStage_FastenerCalibration::setMenuState(eFastenerCalibrationMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
		m_cameraSettingsModel->setMenuState(newState);

		// Show or hide the camera controls based on menu state
		const bool bIsCameraSettingsVisible = m_cameraSettingsView->IsVisible();
		const bool bWantCameraSettingsVisibility =
			(newState == eFastenerCalibrationMenuState::capture1) ||
			(newState == eFastenerCalibrationMenuState::capture2) ||
			(newState == eFastenerCalibrationMenuState::testCalibration);
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

// Input Events
void AppStage_FastenerCalibration::onMouseButtonUp(int button)
{
	if (m_calibrationModel->getMenuState() == eFastenerCalibrationMenuState::capture1 ||
		m_calibrationModel->getMenuState() == eFastenerCalibrationMenuState::capture2)
	{
		if (button == SDL_BUTTON_LEFT)
		{
			m_fastenerCalibrator->sampleMouseScreenPosition();
		}
	}
}

// Calibration Model UI Events
void AppStage_FastenerCalibration::onContinueEvent()
{
	// Clear out all of the calibration data we recorded
	m_fastenerCalibrator->resetCalibrationState();

	// Reset all calibration state on the calibration UI model
	m_calibrationModel->setCapturedPointCount(0);

	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eFastenerCalibrationViewpointMode::cameraViewpoint);

	// Advance to the capture state
	if (m_calibrationModel->getMenuState() == eFastenerCalibrationMenuState::verifySetup1)
		setMenuState(eFastenerCalibrationMenuState::capture1);
	else if (m_calibrationModel->getMenuState() == eFastenerCalibrationMenuState::verifySetup2)
		setMenuState(eFastenerCalibrationMenuState::capture2);
}

void AppStage_FastenerCalibration::onRestartEvent()
{
	// Clear out all of the calibration data we recorded
	m_fastenerCalibrator->resetCalibrationState();

	// Reset all calibration state on the calibration UI model
	m_calibrationModel->setCapturedPointCount(0);

	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eFastenerCalibrationViewpointMode::cameraViewpoint);

	// Return to the capture state
	setMenuState(eFastenerCalibrationMenuState::verifySetup1);
}

void AppStage_FastenerCalibration::onCancelEvent()
{
	m_app->popAppState();
}

void AppStage_FastenerCalibration::onReturnEvent()
{
	m_app->popAppState();
}

// Camera Settings Model UI Events
void AppStage_FastenerCalibration::onViewportModeChanged(eFastenerCalibrationViewpointMode newViewMode)
{
	switch (newViewMode)
	{
		case eFastenerCalibrationViewpointMode::cameraViewpoint:
			m_camera->setIsLocked(true);
			break;
		case eFastenerCalibrationViewpointMode::vrViewpoint:
			m_camera->setIsLocked(false);
			break;
		default:
			break;
	}
}

void AppStage_FastenerCalibration::onBrightnessChanged(int newBrightness)
{
	m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, newBrightness, false);
}