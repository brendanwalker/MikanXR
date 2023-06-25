// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "AnchorTriangulation/RmlModel_AnchorTriangulation.h"
#include "App.h"
#include "Colors.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "GlViewport.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "AnchorTriangulator.h"
#include "InputManager.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "StringUtils.h"
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
const char* AppStage_AnchorTriangulation::APP_STAGE_NAME = "AnchorTriangulation";

//-- public methods -----
AppStage_AnchorTriangulation::AppStage_AnchorTriangulation(App* app)
	: AppStage(app, AppStage_AnchorTriangulation::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_AnchorTriangulation)
	, m_videoSourceView()
	, m_anchorTriangulator(nullptr)
	, m_monoDistortionView(nullptr)
	, m_camera(nullptr)
{
	m_targetAnchor.anchorId = INVALID_MIKAN_ID;
	m_targetAnchor.anchorName= "";
	m_targetAnchor.relativeTransform= GlmTransform();
}

AppStage_AnchorTriangulation::~AppStage_AnchorTriangulation()
{
	delete m_calibrationModel;
}

void AppStage_AnchorTriangulation::setBypassCalibrationFlag(bool flag)
{
	m_calibrationModel->setBypassCalibrationFlag(flag);
}

void AppStage_AnchorTriangulation::enter()
{
	AppStage::enter();

	// Disable depth testing on the line renderer while in this app stage
	Renderer::getInstance()->getLineRenderer()->setDisable3dDepth(true);

	// Get the current video source based on the config
	ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
	m_cameraTrackingPuckView =
		VRDeviceManager::getInstance()->getVRDeviceViewByPath(profileConfig->cameraVRDevicePath);

	// Create a new camera to view the scene
	m_camera = getFirstViewport()->getCurrentCamera();
	m_camera->setCameraMovementMode(eCameraMovementMode::stationary);

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Fire up the video scene in the background + pose calibrator
	eAnchorTriangulationMenuState newState;
	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_videoSourceView, 
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Create a calibrator to do the actual triangulation
		m_anchorTriangulator =
			new AnchorTriangulator(
				m_cameraTrackingPuckView,
				m_monoDistortionView);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			newState = eAnchorTriangulationMenuState::testCalibration;
		}
		else
		{
			newState = eAnchorTriangulationMenuState::verifyInitialCameraSetup;
		}
	}
	else
	{
		newState = eAnchorTriangulationMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnOkEvent = MakeDelegate(this, &AppStage_AnchorTriangulation::onOkEvent);
		m_calibrationModel->OnRedoEvent = MakeDelegate(this, &AppStage_AnchorTriangulation::onRedoEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_AnchorTriangulation::onCancelEvent);

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("anchor_triangulation.rml");
	}

	// Bind to space bar to capture frames
	// (Auto cleared on AppStage exit)
	{
		EventBindingSet* bindingSet = InputManager::getInstance()->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonReleasedEvent += MakeDelegate(this, &AppStage_AnchorTriangulation::onMouseButtonUp);
	}

	setMenuState(newState);
}

void AppStage_AnchorTriangulation::exit()
{
	setMenuState(eAnchorTriangulationMenuState::inactive);

	// Re-Enable depth testing on the line renderer while in this app stage
	Renderer::getInstance()->getLineRenderer()->setDisable3dDepth(false);

	m_camera= nullptr;

	if (m_videoSourceView)
	{
		// Turn back off the video feed
		m_videoSourceView->stopVideoStream();
		m_videoSourceView = nullptr;
	}

	// Free the calibrator
	if (m_anchorTriangulator != nullptr)
	{
		delete m_anchorTriangulator;
		m_anchorTriangulator = nullptr;
	}

	// Free the distortion view buffers
	if (m_monoDistortionView != nullptr)
	{
		delete m_monoDistortionView;
		m_monoDistortionView = nullptr;
	}

	AppStage::exit();
}

void AppStage_AnchorTriangulation::updateCamera()
{
	// Update the transform of the camera so that vr models align over the tracking puck
	const glm::mat4 cameraPose = m_videoSourceView->getCameraPose(m_cameraTrackingPuckView);
	
	m_camera->setCameraTransform(cameraPose);
}

void AppStage_AnchorTriangulation::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	updateCamera();

	// Update the video frame buffers to preview the calibration mat
	m_monoDistortionView->readAndProcessVideoFrame();

	// Update triangulation during triangulation states
	eAnchorTriangulationMenuState calibrationState= m_calibrationModel->getMenuState();
	if (calibrationState == eAnchorTriangulationMenuState::captureOrigin2 || 
		calibrationState == eAnchorTriangulationMenuState::captureXAxis2 ||
		calibrationState == eAnchorTriangulationMenuState::captureYAxis2)
	{
		m_anchorTriangulator->computeCurrentTriangulation();
	}
}

void AppStage_AnchorTriangulation::render()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eAnchorTriangulationMenuState::verifyInitialCameraSetup:
		case eAnchorTriangulationMenuState::captureOrigin1:
		case eAnchorTriangulationMenuState::captureXAxis1:
		case eAnchorTriangulationMenuState::captureYAxis1:
		case eAnchorTriangulationMenuState::verifyInitialPointCapture:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_anchorTriangulator->renderInitialPoint2dSegements();
			}
			break;
		case eAnchorTriangulationMenuState::moveCamera:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_anchorTriangulator->renderInitialPoint3dRays();
			}
			break;
		case eAnchorTriangulationMenuState::captureOrigin2:
		case eAnchorTriangulationMenuState::captureXAxis2:
		case eAnchorTriangulationMenuState::captureYAxis2:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_anchorTriangulator->renderCurrentPointTriangulation();
			}
			break;
		case eAnchorTriangulationMenuState::verifyTriangulatedPoints:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_anchorTriangulator->renderAllTriangulatedPoints(false);
			}
			break;
		case eAnchorTriangulationMenuState::testCalibration:
			{
				m_monoDistortionView->renderSelectedVideoBuffers();
				m_anchorTriangulator->renderAllTriangulatedPoints(false);
				m_anchorTriangulator->renderAnchorTransform();
			}
			break;
	}
}

void AppStage_AnchorTriangulation::setMenuState(eAnchorTriangulationMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
	}
}

// Input Events
void AppStage_AnchorTriangulation::onMouseButtonUp(int button)
{
	eAnchorTriangulationMenuState menuState= m_calibrationModel->getMenuState();

	if (menuState == eAnchorTriangulationMenuState::captureOrigin1 ||
		menuState == eAnchorTriangulationMenuState::captureXAxis1 ||
		menuState == eAnchorTriangulationMenuState::captureYAxis1 ||
		menuState == eAnchorTriangulationMenuState::captureOrigin2 ||
		menuState == eAnchorTriangulationMenuState::captureXAxis2 ||
		menuState == eAnchorTriangulationMenuState::captureYAxis2)
	{
		if (button == SDL_BUTTON_LEFT)
		{
			m_anchorTriangulator->sampleMouseScreenPosition();
		}

		switch (menuState)
		{
			case eAnchorTriangulationMenuState::captureOrigin1:
				setMenuState(eAnchorTriangulationMenuState::captureXAxis1);
				break;
			case eAnchorTriangulationMenuState::captureXAxis1:
				setMenuState(eAnchorTriangulationMenuState::captureYAxis1);
				break;
			case eAnchorTriangulationMenuState::captureYAxis1:
				setMenuState(eAnchorTriangulationMenuState::verifyInitialPointCapture);
				break;
			case eAnchorTriangulationMenuState::captureOrigin2:
				setMenuState(eAnchorTriangulationMenuState::captureXAxis2);
				break;
			case eAnchorTriangulationMenuState::captureXAxis2:
				setMenuState(eAnchorTriangulationMenuState::captureYAxis2);
				break;
			case eAnchorTriangulationMenuState::captureYAxis2:
				setMenuState(eAnchorTriangulationMenuState::verifyTriangulatedPoints);
				break;
		}
	}
}

// Calibration Model UI Events
void AppStage_AnchorTriangulation::onOkEvent()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eAnchorTriangulationMenuState::verifyInitialCameraSetup:
			{
				// Clear out all of the calibration data we recorded
				m_anchorTriangulator->resetCalibrationState();

				// Record the initial camera post
				m_anchorTriangulator->sampleCameraPose();

				// Reset the capture point count on the UI model
				m_calibrationModel->setCapturedPointCount(0);

				setMenuState(eAnchorTriangulationMenuState::captureOrigin1);
			} break;
		case eAnchorTriangulationMenuState::verifyInitialPointCapture:
			{
				setMenuState(eAnchorTriangulationMenuState::moveCamera);
			} break;
		case eAnchorTriangulationMenuState::moveCamera:
			{
				// Reset all calibration state on the calibration UI model
				m_calibrationModel->setCapturedPointCount(0);

				setMenuState(eAnchorTriangulationMenuState::captureOrigin2);
			} break;
		case eAnchorTriangulationMenuState::verifyTriangulatedPoints:
			{
				m_anchorTriangulator->computeAnchorTransform(m_targetAnchor);

				if (m_targetAnchor.anchorId == INVALID_MIKAN_ID)
				{
					AnchorObjectSystem::getSystem()->addNewAnchor(
						m_targetAnchor.anchorName, 
						m_targetAnchor.relativeTransform);
				}
				else
				{
					AnchorComponentPtr anchorComponent=
						AnchorObjectSystem::getSystem()->getSpatialAnchorById(
							m_targetAnchor.anchorId);

					anchorComponent->setRelativeTransform(m_targetAnchor.relativeTransform);
				}

				setMenuState(eAnchorTriangulationMenuState::testCalibration);
			} break;
		case eAnchorTriangulationMenuState::testCalibration:
		case eAnchorTriangulationMenuState::failedVideoStartStreamRequest:
			{
				m_app->popAppState();
			} break;
	}
}

void AppStage_AnchorTriangulation::onRedoEvent()
{
	// Clear out all of the calibration data we recorded
	m_anchorTriangulator->resetCalibrationState();

	// Reset the capture point count on the UI model
	m_calibrationModel->setCapturedPointCount(0);

	// Return to the capture state
	switch (m_calibrationModel->getMenuState())
	{
		case eAnchorTriangulationMenuState::captureOrigin1:
		case eAnchorTriangulationMenuState::captureXAxis1:
		case eAnchorTriangulationMenuState::captureYAxis1:
		case eAnchorTriangulationMenuState::verifyInitialPointCapture:
		case eAnchorTriangulationMenuState::testCalibration:
			setMenuState(eAnchorTriangulationMenuState::verifyInitialCameraSetup);
			break;
		case eAnchorTriangulationMenuState::captureOrigin2:
		case eAnchorTriangulationMenuState::captureXAxis2:
		case eAnchorTriangulationMenuState::captureYAxis2:
		case eAnchorTriangulationMenuState::verifyTriangulatedPoints:
			setMenuState(eAnchorTriangulationMenuState::captureOrigin2);
			break;
	}
}

void AppStage_AnchorTriangulation::onCancelEvent()
{
	m_app->popAppState();
}