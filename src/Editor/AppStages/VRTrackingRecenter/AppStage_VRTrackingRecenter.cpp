//-- includes -----
#include "VRTrackingRecenter/AppStage_VRTrackingRecenter.h"
#include "VRTrackingRecenter/GuiPanel_VRTrackingRecenter.h"
#include "ArucoMarkerPoseSampler.h"
#include "App.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "MikanCamera.h"
#include "IMkFrameBuffer.h"
#include "IMkGraphicsContext.h"
#include "MikanLineRenderer.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkScene.h"
#include "MkScopedObjectBinding.h"
#include "MkStateStack.h"
#include "IMkTriangulatedMesh.h"
#include "IMkLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanViewport.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "CalibrationPatternFinder.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VRObjectSystem.h"
#include "VRTrackingVolumeSystem.h"
#include "VideoSourceComponent.h"

#include "glm/gtc/quaternion.hpp"

#include "MkGuiScopedWindow.h"

#include "imgui.h"

//-- statics ----
const char* AppStage_VRTrackingRecenter::APP_STAGE_NAME = "VRTrackingRecenter";

//-- public methods -----
AppStage_VRTrackingRecenter::AppStage_VRTrackingRecenter(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_VRTrackingRecenter::APP_STAGE_NAME)
	, m_videoSourceComponent()
	, m_markerPoseSampler(nullptr)
	, m_monoDistortionView(nullptr)
	, m_mkCamera(nullptr)
	, m_frameBuffer(createMkFrameBuffer())
	, m_fullscreenRGBQuad(createFullscreenQuadMesh(ownerWindow->getGraphicsContext().get(), false))
{
}

AppStage_VRTrackingRecenter::~AppStage_VRTrackingRecenter()
{
}

void AppStage_VRTrackingRecenter::setSourceCamera(CameraComponentPtr cameraComponent)
{
	// Get the current video source based on the config
	m_cameraComponent = cameraComponent;
	m_videoSourceComponent = m_cameraComponent->getVideoSourceComponent();
}

void AppStage_VRTrackingRecenter::enter()
{
	AppStage::enter();

	// Fetch the new camera associated with the viewport
	m_mkCamera= getFirstViewport()->getCurrentMikanCamera();
	m_mkCamera->setCameraMovementMode(eCameraMovementMode::stationary);

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_cameraComponent->getApertureIntrinsics(cameraIntrinsics);
	m_mkCamera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Create a frame buffer to render the scene into using the resolution and fov from the camera intrinsics
	const MikanMonoIntrinsics& monoIntrinsics= cameraIntrinsics.getMonoIntrinsics();
	m_frameBuffer->setName("VRTrackingRecenter");
	m_frameBuffer->setSize(monoIntrinsics.pixel_width, monoIntrinsics.pixel_height);
	m_frameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);
	m_frameBuffer->createResources();

	// Fire up the video scene in the background + pose calibrator
	eVRTrackingRecenterMenuState newState= eVRTrackingRecenterMenuState::verifySetup;
	//TODO: Handle pendingStart
	if ((int)m_videoSourceComponent->startVideoStream() > 0)
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_videoSourceComponent, 
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Create a sampler to do the actual marker pose recording
		m_markerPoseSampler =
			new ArucoMarkerPoseSampler(
				m_cameraComponent,
				m_monoDistortionView,
				DESIRED_MARKER_SAMPLE_COUNT);
	}
	else
	{
		newState = eVRTrackingRecenterMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage GUI panels
	// (Auto cleaned up on app state exit)
	{
		m_calibrationPanel = addGuiPanel<GuiPanel_VRTrackingRecenter>();
		m_calibrationPanel->OnBeginEvent = [this]() { onBeginEvent(); };
		m_calibrationPanel->OnRestartEvent = [this]() { onRestartEvent(); };
		m_calibrationPanel->OnCancelEvent = [this]() { onCancelEvent(); };
		m_calibrationPanel->OnReturnEvent = [this]() { onReturnEvent(); };
		m_calibrationPanel->OnMarkerStabilityChangedEvent = [this](bool b) { onMarkerStabilityChangedEvent(b); };
	}

	setMenuState(newState);
}

void AppStage_VRTrackingRecenter::exit()
{
	setMenuState(eVRTrackingRecenterMenuState::inactive);

	m_mkCamera= nullptr;

	if (m_videoSourceComponent)
	{
		// Turn back off the video feed
		m_videoSourceComponent->stopVideoStream();
		m_videoSourceComponent = nullptr;
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
	switch (m_calibrationPanel->getMenuState())
	{
		case eVRTrackingRecenterMenuState::verifySetup:
		case eVRTrackingRecenterMenuState::capture:
			{
				// All debug rendering during calibration is camera relative
				// so zero out the camera transform
				m_mkCamera->setCameraTransform(glm::mat4(1.f));
			}
			break;
		case eVRTrackingRecenterMenuState::testCalibration:
			{
				// Use the re-centered scene space for the camera
				glm::mat4 cameraPose;
				if (m_cameraComponent->getSceneSpaceAperturePose(cameraPose))
				{
					m_mkCamera->setCameraTransform(cameraPose);
				}
			}
			break;
	}
}

void AppStage_VRTrackingRecenter::update(float deltaSeconds)
{
	updateCameraPose();

	switch(m_calibrationPanel->getMenuState())
	{
		case eVRTrackingRecenterMenuState::verifySetup:
			{
				// Update the video frame buffers to preview the calibration mat
				m_monoDistortionView->readAndProcessVideoFrame();

				// Look for a marker pose so that we can preview if it's in frame
				m_markerPoseSampler->computeVRSpaceMarkerXform();

				// See if we can compute a valid marker pose
				m_calibrationPanel->setCurrentMarkerValid(m_markerPoseSampler->hasValidVRSpaceMarkerXform());

				// Update the time that the chessboard has been stable for
				m_calibrationPanel->updateMarkerStabilityTimer(deltaSeconds);
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
					m_calibrationPanel->setCalibrationFraction(m_markerPoseSampler->getCalibrationProgress());
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

						// Publish the new VR device pose offset to the target tracking volume
						if (m_targetVolumeId != INVALID_MIKAN_ID)
						{
							auto vrTrackingVolumeSystem = getSystemOfType<VRTrackingVolumeSystem>();
							VRTrackingVolumeComponentPtr trackingVolume =
								vrTrackingVolumeSystem->getVRTrackingVolumeById(m_targetVolumeId);

							trackingVolume->setVRDevicePoseOffset(glmVRDevicePoseOffset);
						}

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

void AppStage_VRTrackingRecenter::render(IMkViewportPtr targetViewport)
{
	IMkGraphicsContext* graphicsContext = getGraphicsContext();

	// Render the scene into the frame buffer
	if (m_frameBuffer->isValid())
	{
		MkScopedObjectBinding colorFramebufferBinding(
			graphicsContext->getMkStateStack().getCurrentState(),
			"Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
			// Draw the camera view no matter the calibration state
			m_monoDistortionView->renderSelectedVideoBuffers();

			switch (m_calibrationPanel->getMenuState())
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
						drawTransformedAxes(graphicsContext, origin, 0.1f);

						TextStyle style = getDefaultTextStyle();
						drawTextAtWorldPosition(graphicsContext, style, glm_mat4_get_position(origin), L"Origin");
					}
					break;
			}
		}

		// Render any lines and text that were added to the scene by the calibrator in the frame buffer's viewport
		graphicsContext->getLineRenderer()->render();
		graphicsContext->getTextRenderer()->render();
	}

	// Render the frame buffer to the screen
	if (m_frameBuffer->isValid())
	{
		MkMaterialInstancePtr materialInstance = m_fullscreenRGBQuad->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			auto colorTexture= m_frameBuffer->getColorTexture();

			// Bind the color texture
			materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, colorTexture);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				m_fullscreenRGBQuad->drawElements();
			}
		}
	}
}

void AppStage_VRTrackingRecenter::onGui()
{
	AppStage::onGui();

	constexpr float k_panelWidth = 415.f;
	const float displayWidth = m_ownerWindow->getWidth();
	const float displayHeight = m_ownerWindow->getHeight();
	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);

	constexpr ImGuiWindowFlags k_flags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar;

	MkGuiScopedWindow panel("##VRTrackingRecenter", nullptr, k_flags);
	if (!panel) return;

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}

void AppStage_VRTrackingRecenter::setMenuState(eVRTrackingRecenterMenuState newState)
{
	if (m_calibrationPanel->getMenuState() != newState)
	{
		eVRTrackingRecenterMenuState oldState= m_calibrationPanel->getMenuState();

		// Update menu state on the data models
		m_calibrationPanel->setMenuState(newState);

		// Broadcast the menu state change to the remote control manager
		{
			std::vector<std::string> parameters;
			parameters.push_back(k_VRTrackingRecenterMenuStateStrings[(int)oldState]);
			parameters.push_back(k_VRTrackingRecenterMenuStateStrings[(int)newState]);

			sendRemoteControlEvent("menu_state_changed", parameters);
		}
	}
}

// Calibration Model UI Events
void AppStage_VRTrackingRecenter::onBeginEvent()
{
	tryBeginCapture();
}

bool AppStage_VRTrackingRecenter::tryBeginCapture()
{
	if (m_calibrationPanel->getMenuState() == eVRTrackingRecenterMenuState::verifySetup)
	{
		// Clear out all of the calibration data we recorded
		m_markerPoseSampler->resetCalibrationState();

		// Reset all calibration state on the calibration UI model
		m_calibrationPanel->setCalibrationFraction(0.f);

		// Advance to the capture state
		setMenuState(eVRTrackingRecenterMenuState::capture);

		return true;
	}

	return false;
}

void AppStage_VRTrackingRecenter::onRestartEvent()
{
	tryRestartCapture();
}

bool AppStage_VRTrackingRecenter::tryRestartCapture()
{
	if (m_calibrationPanel->getMenuState() != eVRTrackingRecenterMenuState::verifySetup)
	{
		// Clear out all of the calibration data we recorded
		m_markerPoseSampler->resetCalibrationState();

		// Reset all calibration state on the calibration UI model
		m_calibrationPanel->setCalibrationFraction(0.f);

		// Return to the capture state
		setMenuState(eVRTrackingRecenterMenuState::verifySetup);

		return true;
	}

	return false;
}

void AppStage_VRTrackingRecenter::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_VRTrackingRecenter::onReturnEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_VRTrackingRecenter::onMarkerStabilityChangedEvent(bool bIsStable)
{
	std::vector<std::string> parameters;
	parameters.push_back(bIsStable ? IRemoteControllable::k_true : IRemoteControllable::k_false);

	sendRemoteControlEvent("marker_stability_changed", parameters);
}

// Remote Control
bool AppStage_VRTrackingRecenter::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	if (!IRemoteControllable::handleRemoteControlCommand(command, parameters, outResults))
	{
		if (command == "get_state")
		{
			return handleGetStateCommand(outResults);
		}
		else if (command == "get_marker_stability")
		{
			return handleGetChessboardStabilityCommand(outResults);
		}
		else if (command == "begin")
		{
			return handleBeginCommand(outResults);
		}
		else if (command == "restart")
		{
			return handleRestartCommand(outResults);
		}
	}

	return false;
}

bool AppStage_VRTrackingRecenter::handleGetStateCommand(
	std::vector<std::string>& outResults)
{
	const eVRTrackingRecenterMenuState menuState = m_calibrationPanel->getMenuState();
	const std::string& stateName = k_VRTrackingRecenterMenuStateStrings[(int)menuState];

	outResults.push_back(stateName);

	return true;
}

bool AppStage_VRTrackingRecenter::handleGetChessboardStabilityCommand(
	std::vector<std::string>& outResults)
{
	const bool bIsStable = m_calibrationPanel->getCurrentMarkerStable();
	outResults.push_back(bIsStable ? IRemoteControllable::k_true : IRemoteControllable::k_false);

	return true;
}

bool AppStage_VRTrackingRecenter::handleBeginCommand(std::vector<std::string>& outResults)
{
	if (tryBeginCapture())
	{
		outResults.push_back(IRemoteControllable::k_success);
	}
	else
	{
		outResults.push_back(IRemoteControllable::k_failure);
	}

	return true;
}

bool AppStage_VRTrackingRecenter::handleRestartCommand(std::vector<std::string>& outResults)
{
	if (tryRestartCapture())
	{
		outResults.push_back(IRemoteControllable::k_success);
	}
	else
	{
		outResults.push_back(IRemoteControllable::k_failure);
	}

	return true;
}