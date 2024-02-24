// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "DepthMeshCapture/AppStage_DepthMeshCapture.h"
#include "DepthMeshCapture/RmlModel_DepthMeshCapture.h"
#include "DepthMeshCapture/RmlModel_DepthMeshCameraSettings.h"
#include "App.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlScene.h"
#include "GlTextRenderer.h"
#include "GlViewport.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MonoLensDepthMeshCapture.h"
#include "CalibrationPatternFinder.h"
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
const char* AppStage_DepthMeshCapture::APP_STAGE_NAME = "DepthMeshCapture";

//-- public methods -----
AppStage_DepthMeshCapture::AppStage_DepthMeshCapture(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_DepthMeshCapture::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_DepthMeshCapture)
	, m_cameraSettingsModel(new RmlModel_DepthMeshCameraSettings)
	, m_videoSourceView()
	, m_depthMeshCapture(nullptr)
	, m_monoDistortionView(nullptr)
	, m_scene(std::make_shared<GlScene>())
	, m_camera(nullptr)
{
}

AppStage_DepthMeshCapture::~AppStage_DepthMeshCapture()
{
	delete m_calibrationModel;
	delete m_cameraSettingsModel;
}

void AppStage_DepthMeshCapture::setBypassCalibrationFlag(bool flag)
{
	m_calibrationModel->setBypassCalibrationFlag(flag);
}

void AppStage_DepthMeshCapture::enter()
{
	AppStage::enter();

	// Get the current video source based on the config
	const ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
	m_cameraTrackingPuckView= 
		VRDeviceManager::getInstance()->getVRDeviceViewByPath(profileConfig->cameraVRDevicePath);

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
	eDepthMeshCaptureMenuState newState;
	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				App::getInstance()->getMainWindow()->getOpenCVManager(),
				m_videoSourceView, 
				VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG | 
				VIDEO_FRAME_HAS_GL_TEXTURE_FLAG |
				VIDEO_FRAME_HAS_DEPTH_FLAG);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Create a calibrator to do the actual pattern recording and calibration
		m_depthMeshCapture =
			new MonoLensDepthMeshCapture(
				profileConfig,
				m_cameraTrackingPuckView,
				m_monoDistortionView,
				DESIRED_CAPTURE_BOARD_COUNT);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			newState = eDepthMeshCaptureMenuState::testCapture;
		}
		else
		{
			newState = eDepthMeshCaptureMenuState::verifySetup;
		}
	}
	else
	{
		newState = eDepthMeshCaptureMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnBeginEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onBeginEvent);
		m_calibrationModel->OnRestartEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onRestartEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onCancelEvent);
		m_calibrationModel->OnReturnEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onReturnEvent);

		// Init camera settings model
		m_cameraSettingsModel->init(context, m_videoSourceView, profileConfig);
		m_cameraSettingsModel->OnViewpointModeChanged = MakeDelegate(this, &AppStage_DepthMeshCapture::onViewportModeChanged);
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::mixedRealityViewpoint);
		}
		else
		{
			m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::cameraViewpoint);
		}

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("depth_capture.rml");

		// Init camera settings view now that the dependent model has been created
		m_cameraSettingsView = addRmlDocument("depth_capture_camera_settings.rml");
	}

	setMenuState(newState);
}

void AppStage_DepthMeshCapture::exit()
{
	setMenuState(eDepthMeshCaptureMenuState::inactive);

	m_camera= nullptr;

	VRDeviceList vrDeviceList = VRDeviceManager::getInstance()->getVRDeviceList();
	for (auto it : vrDeviceList)
	{
		it->getVRDeviceInterface()->removeFromBoundScene();
	}

	if (m_videoSourceView)
	{
		// Turn back off the video feed
		m_videoSourceView->stopVideoStream();
		m_videoSourceView = nullptr;
	}

	// Free the calibrator
	if (m_depthMeshCapture != nullptr)
	{
		delete m_depthMeshCapture;
		m_depthMeshCapture = nullptr;
	}

	// Free the distortion view buffers
	if (m_monoDistortionView != nullptr)
	{
		delete m_monoDistortionView;
		m_monoDistortionView = nullptr;
	}

	AppStage::exit();
}

void AppStage_DepthMeshCapture::updateCamera()
{
	switch (m_cameraSettingsModel->getViewpointMode())
	{
	case eDepthMeshCaptureViewpointMode::cameraViewpoint:
	case eDepthMeshCaptureViewpointMode::vrViewpoint:
		{
			// Nothing to do
		}
		break;
	case eDepthMeshCaptureViewpointMode::mixedRealityViewpoint:
		{
			// Update the transform of the camera so that vr models align over the tracking puck
			glm::mat4 cameraPose;
			if (m_calibrationModel->getMenuState() == eDepthMeshCaptureMenuState::testCapture)
			{
				// Use the calibrated offset on the video source to get the camera pose
				cameraPose= m_videoSourceView->getCameraPose(m_cameraTrackingPuckView);
			}
			else
			{
				// Use the last computed preview camera alignment
				//cameraPose = m_depthMeshCapture->getLastCameraPose(m_cameraTrackingPuckView);
			}

			m_camera->setCameraTransform(cameraPose);
		}
		break;
	}
}

void AppStage_DepthMeshCapture::update(float deltaSeconds)
{
	updateCamera();

	switch(m_calibrationModel->getMenuState())
	{
		case eDepthMeshCaptureMenuState::verifySetup:
			{
				// Update the video frame buffers to preview the calibration mat
				m_monoDistortionView->readAndProcessVideoFrame();

				// Look for a calibration pattern so that we can preview if it's in frame
				m_depthMeshCapture->captureMesh();
			}
			break;
		case eDepthMeshCaptureMenuState::capture:
			{
				// Update the video frame buffers
				m_monoDistortionView->readAndProcessVideoFrame();

			#if 0
				// Update the chess board capture state
				if (m_depthMeshCapture->captureMesh())
				{
					m_depthMeshCapture->sampleLastCameraToPuckXform();

					// Update the calibration fraction on the UI Model
					m_calibrationModel->setCalibrationFraction(m_depthMeshCapture->getCalibrationProgress());
				}

				// See if we have gotten all the samples we require
				if (m_depthMeshCapture->hasFinishedSampling())
				{
					MikanQuatd rotationOffset;
					MikanVector3d translationOffset;
					if (m_depthMeshCapture->computeCalibratedCameraTrackerOffset(
						rotationOffset,
						translationOffset))
					{
						// Store the calibrated camera offset on the video source settings
						m_videoSourceView->setCameraPoseOffset(rotationOffset, translationOffset);

						// Go to the test calibration state
						m_monoDistortionView->setGrayscaleUndistortDisabled(true);
						m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::mixedRealityViewpoint);
						setMenuState(eDepthMeshCaptureMenuState::testCapture);
					}
				}
			#endif
			}
			break;
		case eDepthMeshCaptureMenuState::testCapture:
			{
				// Update the video frame buffers using the existing distortion calibration
				m_monoDistortionView->readAndProcessVideoFrame();
			}
			break;
	}
}

void AppStage_DepthMeshCapture::render()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eDepthMeshCaptureMenuState::verifySetup:
			{
				switch (m_cameraSettingsModel->getViewpointMode())
				{
					case eDepthMeshCaptureViewpointMode::cameraViewpoint:
						m_monoDistortionView->renderSelectedVideoBuffers();
						m_depthMeshCapture->renderCameraSpaceCalibrationState();
						break;
					case eDepthMeshCaptureViewpointMode::vrViewpoint:
						m_depthMeshCapture->renderVRSpaceCalibrationState();
						renderVRScene();
						break;
					case eDepthMeshCaptureViewpointMode::mixedRealityViewpoint:
						m_monoDistortionView->renderSelectedVideoBuffers();
						renderVRScene();
						break;
				}
			}
			break;
		case eDepthMeshCaptureMenuState::capture:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_depthMeshCapture->renderCameraSpaceCalibrationState();
			}
			break;
		case eDepthMeshCaptureMenuState::testCapture:
			{
				if (m_cameraSettingsModel->getViewpointMode() == eDepthMeshCaptureViewpointMode::mixedRealityViewpoint)
				{
					m_monoDistortionView->renderSelectedVideoBuffers();
				}

				renderVRScene();
			}
			break;
	}
}

void AppStage_DepthMeshCapture::renderVRScene()
{
	m_scene->render(m_camera);

	drawTransformedAxes(glm::mat4(1.f), 1.0f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(style, glm::vec3(1.f, 0.f, 0.f), L"X");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 1.f, 0.f), L"Y");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 1.f), L"Z");
}

void AppStage_DepthMeshCapture::setMenuState(eDepthMeshCaptureMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
		m_cameraSettingsModel->setMenuState(newState);

		// Show or hide the camera controls based on menu state
		const bool bIsCameraSettingsVisible = m_cameraSettingsView->IsVisible();
		const bool bWantCameraSettingsVisibility =
			(newState == eDepthMeshCaptureMenuState::capture) ||
			(newState == eDepthMeshCaptureMenuState::testCapture);
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
void AppStage_DepthMeshCapture::onBeginEvent()
{
	// Clear out all of the calibration data we recorded
	m_depthMeshCapture->resetCalibrationState();

	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::cameraViewpoint);

	// Advance to the capture state
	setMenuState(eDepthMeshCaptureMenuState::capture);
}

void AppStage_DepthMeshCapture::onRestartEvent()
{
	// Clear out all of the calibration data we recorded
	m_depthMeshCapture->resetCalibrationState();

	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::cameraViewpoint);

	// Return to the capture state
	setMenuState(eDepthMeshCaptureMenuState::verifySetup);
}

void AppStage_DepthMeshCapture::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_DepthMeshCapture::onReturnEvent()
{
	m_ownerWindow->popAppState();
}

// Camera Settings Model UI Events
void AppStage_DepthMeshCapture::onViewportModeChanged(eDepthMeshCaptureViewpointMode newViewMode)
{
	switch (newViewMode)
	{
		case eDepthMeshCaptureViewpointMode::cameraViewpoint:
			{
				m_camera->setCameraMovementMode(eCameraMovementMode::stationary);
				m_camera->setCameraTransform(glm::mat4(1.f));
			} break;
		case eDepthMeshCaptureViewpointMode::vrViewpoint:
			{
				m_camera->setCameraMovementMode(eCameraMovementMode::fly);
			} break;
		case eDepthMeshCaptureViewpointMode::mixedRealityViewpoint:
			{
				m_camera->setCameraMovementMode(eCameraMovementMode::stationary);
			} break;
		default:
			break;
	}
}