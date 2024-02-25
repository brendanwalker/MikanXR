//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "DepthMeshCapture/AppStage_DepthMeshCapture.h"
#include "DepthMeshCapture/RmlModel_DepthMeshCapture.h"
#include "DepthMeshCapture/RmlModel_DepthMeshCameraSettings.h"
#include "App.h"
#include "DepthMeshGenerator.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlFrameCompositor.h"
#include "GlScene.h"
#include "GlTextRenderer.h"
#include "GlViewport.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "CalibrationPatternFinder.h"
#include "SyntheticDepthEstimator.h"
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

void AppStage_DepthMeshCapture::setBypassCaptureFlag(bool flag)
{
	m_calibrationModel->setBypassCaptureFlag(flag);
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

	// Fire up the video scene in the background + depth estimator + mesh capture
	bool depthCaptureReady= false;
	if (m_videoSourceView->startVideoStream())
	{
		auto mainWindow = App::getInstance()->getMainWindow();
		auto glFrameCompositor = mainWindow->getFrameCompositor();

		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			std::make_shared<VideoFrameDistortionView>(
				m_videoSourceView, 
				VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG | 
				VIDEO_FRAME_HAS_GL_TEXTURE_FLAG);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Get the depth estimator from the frame compositor
		// (might have made one for depth visualization)
		m_syntheticDepthEstimator = glFrameCompositor->getSyntheticDepthEstimator();

		// If we don't have one, then make a new depth estimator ourselves
		if (!m_syntheticDepthEstimator)
		{
			m_syntheticDepthEstimator =
				std::make_shared<SyntheticDepthEstimator>(
					mainWindow->getOpenCVManager(), 
					DEPTH_OPTION_HAS_GL_TEXTURE_FLAG);

			if (m_syntheticDepthEstimator->initialize())
			{
				MIKAN_LOG_ERROR("GlFrameCompositor::openVideoSource") << "Failed to create depth estimator";
				m_syntheticDepthEstimator = nullptr;
			}
		}

		if (m_syntheticDepthEstimator)
		{
			// Create a depth mesh generator
			m_depthMeshCapture =
				std::make_shared<DepthMeshGenerator>(
					profileConfig,
					m_monoDistortionView,
					m_syntheticDepthEstimator);

			depthCaptureReady= true;
		}
	}

	eDepthMeshCaptureMenuState newState;
	if (depthCaptureReady)
	{
		// If bypassing the capture, then jump straight to the test capture state
		if (m_calibrationModel->getBypassCaptureFlag())
		{
			// Load the mesh and texture from the given paths
			if (m_depthMeshCapture->loadMeshFromObjFile(m_meshSavePath) &&
				m_depthMeshCapture->loadTextureFromPNG(m_textureSavePath))
			{
				newState = eDepthMeshCaptureMenuState::testCapture;
			}
			else
			{
				newState = eDepthMeshCaptureMenuState::failedToStart;
			}
		}
		else
		{
			newState = eDepthMeshCaptureMenuState::verifySetup;
		}
	}
	else
	{
		newState = eDepthMeshCaptureMenuState::failedToStart;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnContinueEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onContinueEvent);
		m_calibrationModel->OnRestartEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onRestartEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_DepthMeshCapture::onCancelEvent);

		// Init camera settings model
		m_cameraSettingsModel->init(context, m_videoSourceView, profileConfig);
		m_cameraSettingsModel->OnViewpointModeChanged = MakeDelegate(this, &AppStage_DepthMeshCapture::onViewportModeChanged);
		if (m_calibrationModel->getBypassCaptureFlag())
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
	m_depthMeshCapture = nullptr;

	// Free the distortion view buffers
	m_monoDistortionView = nullptr;

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
	if (m_calibrationModel->getMenuState() != eDepthMeshCaptureMenuState::failedToStart)
	{
		updateCamera();
		m_monoDistortionView->readAndProcessVideoFrame();
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

		if (newState == eDepthMeshCaptureMenuState::testCapture)
		{
			// Go to the mixed reality viewpoint by default when we are in the test capture state
			m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::mixedRealityViewpoint);
		}
	}
}

// Calibration Model UI Events
void AppStage_DepthMeshCapture::onContinueEvent()
{
	switch (m_calibrationModel->getMenuState())
	{
	case eDepthMeshCaptureMenuState::verifySetup:
	case eDepthMeshCaptureMenuState::captureFailed:
		{
			if (m_depthMeshCapture->captureMesh())
			{
				setMenuState(eDepthMeshCaptureMenuState::testCapture);
			}
			else
			{
				setMenuState(eDepthMeshCaptureMenuState::failedToStart);
			}
		}
		break;
	case eDepthMeshCaptureMenuState::testCapture:
		{
			// Write out the mesh to a file
			if (!m_calibrationModel->getBypassCaptureFlag())
			{
				m_depthMeshCapture->saveMeshToObjFile(m_meshSavePath);
				m_depthMeshCapture->saveTextureToPNG(m_textureSavePath);
			}

			m_ownerWindow->popAppState();
		}
		break;
	}
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