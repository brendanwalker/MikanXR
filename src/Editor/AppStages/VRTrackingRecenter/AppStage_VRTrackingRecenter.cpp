//-- includes -----
#include "VRTrackingRecenter/AppStage_VRTrackingRecenter.h"
#include "VRTrackingRecenter/RmlModel_VRTrackingRecenter.h"
#include "ArucoMarkerPoseSampler.h"
#include "App.h"
#include "MikanCamera.h"
#include "GlFrameBuffer.h"
#include "GlLineRenderer.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlScene.h"
#include "MkScopedObjectBinding.h"
#include "GlStateStack.h"
#include "IMkTriangulatedMesh.h"
#include "GlTextRenderer.h"
#include "MikanViewport.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MathGLM.h"
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
const char* AppStage_VRTrackingRecenter::APP_STAGE_NAME = "VRTrackingRecenter";

//-- public methods -----
AppStage_VRTrackingRecenter::AppStage_VRTrackingRecenter(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_VRTrackingRecenter::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_VRTrackingRecenter)
	, m_videoSourceView()
	, m_markerPoseSampler(nullptr)
	, m_monoDistortionView(nullptr)
	, m_camera(nullptr)
	, m_frameBuffer(std::make_shared<GlFrameBuffer>())
	, m_fullscreenQuad(createFullscreenQuadMesh(ownerWindow, false))
{
}

AppStage_VRTrackingRecenter::~AppStage_VRTrackingRecenter()
{
	delete m_calibrationModel;
}


void AppStage_VRTrackingRecenter::enter()
{
	AppStage::enter();

	// Get the current video source based on the config
	const ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();

	// Get the camera tracking puck pose view
	auto vrDeviceManager = VRDeviceManager::getInstance();
	auto cameraTrackingPuckView= vrDeviceManager->getVRDeviceViewByPath(profileConfig->cameraVRDevicePath);
	m_cameraTrackingPuckRawPoseView= cameraTrackingPuckView->makePoseView(eVRDevicePoseSpace::VRTrackingSystem);
	m_cameraTrackingPuckScenePoseView= cameraTrackingPuckView->makePoseView(eVRDevicePoseSpace::MikanScene);

	// Fetch the new camera associated with the viewport
	m_camera= getFirstViewport()->getCurrentCamera();
	m_camera->setCameraMovementMode(eCameraMovementMode::stationary);

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Create a frame buffer to render the scene into using the resolution and fov from the camera intrinsics
	const MikanMonoIntrinsics& monoIntrinsics= cameraIntrinsics.getMonoIntrinsics();
	m_frameBuffer->setName("VRTrackingRecenter");
	m_frameBuffer->setSize(monoIntrinsics.pixel_width, monoIntrinsics.pixel_height);
	m_frameBuffer->setFrameBufferType(GlFrameBuffer::eFrameBufferType::COLOR);
	m_frameBuffer->createResources();

	// Fire up the video scene in the background + pose calibrator
	eVRTrackingRecenterMenuState newState= eVRTrackingRecenterMenuState::verifySetup;
	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_ownerWindow,
				m_videoSourceView, 
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Create a sampler to do the actual marker pose recording
		m_markerPoseSampler =
			new ArucoMarkerPoseSampler(
				profileConfig,
				m_cameraTrackingPuckRawPoseView,
				m_monoDistortionView,
				DESIRED_MARKER_SAMPLE_COUNT);
	}
	else
	{
		newState = eVRTrackingRecenterMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnBeginEvent = MakeDelegate(this, &AppStage_VRTrackingRecenter::onBeginEvent);
		m_calibrationModel->OnRestartEvent = MakeDelegate(this, &AppStage_VRTrackingRecenter::onRestartEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_VRTrackingRecenter::onCancelEvent);
		m_calibrationModel->OnReturnEvent = MakeDelegate(this, &AppStage_VRTrackingRecenter::onReturnEvent);

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("vr_tracking_recenter.rml");
	}

	setMenuState(newState);
}

void AppStage_VRTrackingRecenter::exit()
{
	setMenuState(eVRTrackingRecenterMenuState::inactive);

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
	if (m_markerPoseSampler != nullptr)
	{
		delete m_markerPoseSampler;
		m_markerPoseSampler = nullptr;
	}

	// Free the distortion view buffers
	if (m_monoDistortionView != nullptr)
	{
		delete m_monoDistortionView;
		m_monoDistortionView = nullptr;
	}

	AppStage::exit();
}

void AppStage_VRTrackingRecenter::updateCameraPose()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eVRTrackingRecenterMenuState::verifySetup:
		case eVRTrackingRecenterMenuState::capture:
			{
				// All debug rendering during calibration is camera relative
				// so zero out the camera transform
				m_camera->setCameraTransform(glm::mat4(1.f));
			}
			break;
		case eVRTrackingRecenterMenuState::testCalibration:
			{
				// Use the re-centered scene space for the camera
				glm::mat4 cameraPose;
				if (m_videoSourceView->getCameraPose(m_cameraTrackingPuckScenePoseView, cameraPose))
				{
					m_camera->setCameraTransform(cameraPose);
				}
			}
			break;
	}
}

void AppStage_VRTrackingRecenter::update(float deltaSeconds)
{
	updateCameraPose();

	switch(m_calibrationModel->getMenuState())
	{
		case eVRTrackingRecenterMenuState::verifySetup:
			{
				// Update the video frame buffers to preview the calibration mat
				m_monoDistortionView->readAndProcessVideoFrame();

				// Look for a marker pose so that we can preview if it's in frame
				m_markerPoseSampler->computeVRSpaceMarkerXform();
			}
			break;
		case eVRTrackingRecenterMenuState::capture:
			{
				// Update the video frame buffers
				m_monoDistortionView->readAndProcessVideoFrame();

				// Update the chess board capture state
				if (m_markerPoseSampler->computeVRSpaceMarkerXform())
				{
					m_markerPoseSampler->sampleLastVRSpaceMarkerXform();

					// Update the calibration fraction on the UI Model
					m_calibrationModel->setCalibrationFraction(m_markerPoseSampler->getCalibrationProgress());
				}

				// See if we have gotten all the samples we require
				if (m_markerPoseSampler->hasFinishedSampling())
				{
					MikanQuatd rotation;
					MikanVector3d translation;
					if (m_markerPoseSampler->computeCalibratedMarkerPose(rotation, translation))
					{
						// The VR device pose offset is the inverse of the marker pose
						// (This makes the marker pose the tracking origin)
						glm::vec3 glmPosition = MikanVector3d_to_glm_dvec3(translation);
						glm::quat glmOrientation = MikanQuatd_to_glm_dquat(rotation);
						glm::mat4 glmXform= glm_mat4_from_pose(glmOrientation, glmPosition);
						glm::mat4 glmVRDevicePoseOffset= glm::inverse(glmXform);

						// Publish the new VR device pose offset to the profile config
						const ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
						profileConfig->vrDevicePoseOffset = glm_mat4_to_MikanMatrix4f(glmVRDevicePoseOffset);
						profileConfig->markDirty(
							ConfigPropertyChangeSet()
							.addPropertyName(ProfileConfig::k_vrDevicePoseOffsetPropertyId));

						setMenuState(eVRTrackingRecenterMenuState::testCalibration);
					}
				}
			}
			break;
		case eVRTrackingRecenterMenuState::testCalibration:
			{
				// Update the video frame buffers using the existing distortion calibration
				m_monoDistortionView->readAndProcessVideoFrame();
			}
			break;
	}
}

void AppStage_VRTrackingRecenter::render()
{
	// Render the scene into the frame buffer
	if (m_frameBuffer->isValid())
	{
		MkScopedObjectBinding colorFramebufferBinding(
			*m_ownerWindow->getGlStateStack().getCurrentState(),
			"Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
			// Draw the camera view no matter the calibration state
			m_monoDistortionView->renderSelectedVideoBuffers();

			switch (m_calibrationModel->getMenuState())
			{
				case eVRTrackingRecenterMenuState::verifySetup:
				case eVRTrackingRecenterMenuState::capture:
					{
						// draw the camera relative calibration state when calibrating
						m_markerPoseSampler->renderCameraSpaceCalibrationState();
					}
					break;
				case eVRTrackingRecenterMenuState::testCalibration:
					{
						// Draw the origin after calibrating
						glm::mat4 origin(1.f);
						drawTransformedAxes(origin, 0.1f);

						TextStyle style = getDefaultTextStyle();
						drawTextAtWorldPosition(style, glm_mat4_get_position(origin), L"Origin");
					}
					break;
			}
		}

		// Render any lines and text that were added to the scene by the calibrator in the frame buffer's viewport
		m_ownerWindow->getLineRenderer()->render();
		m_ownerWindow->getTextRenderer()->render();
	}

	// Render the frame buffer to the screen
	if (m_frameBuffer->isValid())
	{
		GlMaterialInstancePtr materialInstance = m_fullscreenQuad->getMaterialInstance();
		GlMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			auto colorTexture= m_frameBuffer->getColorTexture();

			// Bind the color texture
			materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, colorTexture);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				m_fullscreenQuad->drawElements();
			}
		}
	}
}

void AppStage_VRTrackingRecenter::setMenuState(eVRTrackingRecenterMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
	}
}

// Calibration Model UI Events
void AppStage_VRTrackingRecenter::onBeginEvent()
{
	// Clear out all of the calibration data we recorded
	m_markerPoseSampler->resetCalibrationState();

	// Reset all calibration state on the calibration UI model
	m_calibrationModel->setCalibrationFraction(0.f);

	// Advance to the capture state
	setMenuState(eVRTrackingRecenterMenuState::capture);
}

void AppStage_VRTrackingRecenter::onRestartEvent()
{
	// Clear out all of the calibration data we recorded
	m_markerPoseSampler->resetCalibrationState();

	// Reset all calibration state on the calibration UI model
	m_calibrationModel->setCalibrationFraction(0.f);

	// Return to the capture state
	setMenuState(eVRTrackingRecenterMenuState::verifySetup);
}

void AppStage_VRTrackingRecenter::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_VRTrackingRecenter::onReturnEvent()
{
	m_ownerWindow->popAppState();
}