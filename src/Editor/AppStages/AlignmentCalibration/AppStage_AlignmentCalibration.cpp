// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "AlignmentCalibration/GuiPanel_AlignmentCalibration.h"
#include "AlignmentCalibration/GuiPanel_AlignmentCameraSettings.h"
#include "ModalMessageBox/ModalDialog_MessageBox.h"
#include "MkGuiScopedWindow.h"
#include "CalibrationPatternFinder.h"
#include "CameraComponent.h"
#include "Colors.h"
#include "MkScene.h"
#include "IMkGraphicsContext.h"
#include "IMkFrameBuffer.h"
#include "IMkTriangulatedMesh.h"
#include "IMkLineRenderer.h"
#include "IMkTextRenderer.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanViewport.h"
#include "MainWindow.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkScopedObjectBinding.h"
#include "MkStateStack.h"
#include "MonoLensTrackerPoseCalibrator.h"
#include "StageComponent.h"
#include "TextStyle.h"
#include "TrackingMountObjectSystem.h"
#include "VideoSourceComponent.h"
#include "VideoFrameDistortionView.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"
#include "VRTrackingVolumeComponent.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/ext/vector_float4.hpp"

#include "imgui.h"

//-- statics ----
const char* AppStage_AlignmentCalibration::APP_STAGE_NAME = "AlignmentCalibration";

//-- constants -----
static const char* k_calibration_pattern_names[] = {
	"Chessboard",
	"Circle Grid",
};

//-- public methods -----
AppStage_AlignmentCalibration::AppStage_AlignmentCalibration(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_AlignmentCalibration::APP_STAGE_NAME)
	, m_targetCameraComponent()
	, m_videoSourceComponent()
	, m_trackerPoseCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
	, m_scene(std::make_shared<MkScene>())
	, m_mkCamera(nullptr)
	, m_frameBuffer(createMkFrameBuffer())
	, m_fullscreenRGBQuad(createFullscreenQuadMesh(ownerWindow->getGraphicsContext().get(), false))
{
}

AppStage_AlignmentCalibration::~AppStage_AlignmentCalibration()
{
}

bool AppStage_AlignmentCalibration::tryEnterAlignmentCalibration(
	AppStage* fromAppStage,
	CameraComponentPtr forCameraComponent)
{
	IEditorWindow* ownerWindow= fromAppStage->getOwnerWindow();

	VideoSourceComponentPtr videoSourceComponent = forCameraComponent->getVideoSourceComponent(); 
	if (!videoSourceComponent)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Camera is not associated with a video source. Please set a video source for the camera before aligning.");
		return false;
	}

	if (!videoSourceComponent->areCameraIntrinsicsValid())
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Camera does not have valid aperture intrinsics. Please calibrate the camera's intrinsics before using it for alignment.");
		return false;
	}

	VRDevicePoseViewPtr cameraPuckPose_VRSystemSpace=
		forCameraComponent->makeTrackingMountPoseView(eVRDevicePoseSpace::VRTrackingSystemPose);
	if (!cameraPuckPose_VRSystemSpace)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Camera is missing a tracking mount. Please ensure the camera tracking mount puck is on before aligning.");
		return false;
	}

	VRTrackingVolumeComponentConstPtr trackingVolumeComponent = 
		forCameraComponent->getVRTrackingVolumeComponent();
	if (!trackingVolumeComponent)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Camera is not associated with a VR tracking volume. Please assign a VR tracking volume to the stage this camera is attached to before aligning.");
		return false;
	}

	VRDevicePoseViewPtr matPuckPose_VRSystemSpace=
		trackingVolumeComponent->makeChArUcoTrackingMountPoseView(
			eVRDevicePoseSpace::VRTrackingSystemPose);
	if (!matPuckPose_VRSystemSpace)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"VRTracking volume missing ChArUco Mount. Please ensure the ChArUco tracking puck is on before aligning.");
		return false;
	}

	auto* alignmentCalibration = ownerWindow->pushAppStageOfType<AppStage_AlignmentCalibration>();
	alignmentCalibration->setTargetCameraComponent(forCameraComponent);
	alignmentCalibration->setVideoSourceComponent(videoSourceComponent);
	alignmentCalibration->setCameraPuckPose(cameraPuckPose_VRSystemSpace);
	alignmentCalibration->setMatPuckPose(matPuckPose_VRSystemSpace);

	return true;
}

void AppStage_AlignmentCalibration::setBypassCalibrationFlag(bool flag)
{
	m_bypassCalibrationFlag = flag;
	if (m_calibrationPanel != nullptr)
		m_calibrationPanel->setBypassCalibrationFlag(flag);
}

void AppStage_AlignmentCalibration::setTargetCameraComponent(CameraComponentPtr cameraComponent)
{
	m_targetCameraComponent = cameraComponent;
}

void AppStage_AlignmentCalibration::setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent)
{
	m_videoSourceComponent = videoSourceComponent;
}

void AppStage_AlignmentCalibration::setCameraPuckPose(VRDevicePoseViewPtr cameraPuckPose)
{
	m_cameraPuckPose_VRSystemSpace = cameraPuckPose;
}

void AppStage_AlignmentCalibration::setMatPuckPose(VRDevicePoseViewPtr matPuckPose)
{
	m_matPuckPose_VRSystemSpace = matPuckPose;
}

void AppStage_AlignmentCalibration::enter()
{
	AppStage::enter();
	assert(m_targetCameraComponent != nullptr);
	assert(m_videoSourceComponent);
	assert(m_videoSourceComponent->areCameraIntrinsicsValid());
	assert(m_cameraPuckPose_VRSystemSpace);
	assert(m_matPuckPose_VRSystemSpace);

	// Fetch the new mk camera associated with the viewport
	m_mkCamera= getFirstViewport()->getCurrentMikanCamera();

	// Make sure the mk camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceComponent->getCameraIntrinsics(cameraIntrinsics);
	m_mkCamera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Create a frame buffer to render the scene into using the resolution and fov from the camera intrinsics
	const MikanMonoIntrinsics& monoIntrinsics= cameraIntrinsics.getMonoIntrinsics();
	m_frameBuffer->setName("AlignmentCalibration");
	m_frameBuffer->setSize(monoIntrinsics.pixel_width, monoIntrinsics.pixel_height);
	m_frameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);
	m_frameBuffer->createResources();
	m_frameBuffer->setClearColor(glm::vec4(Colors::CornflowerBlue, 1.f));

	// Fire up the video scene in the background + pose calibrator
	eAlignmentCalibrationMenuState newState;
	switch (m_videoSourceComponent->startVideoStream())
	{
	case eVideoStreamingStatus::pendingStart:
		// Wait for the video stream to start in the update loop
		newState = eAlignmentCalibrationMenuState::pendingVideoStart;
		break;
	case eVideoStreamingStatus::started:
		{
			// Immediately setup the calibrator
			setupTrackerPoseCalibrator();

			// If bypassing the calibration, then jump straight to the test calibration state
			newState = (m_bypassCalibrationFlag) 
				? eAlignmentCalibrationMenuState::testCalibration
				: eAlignmentCalibrationMenuState::verifySetup;
		}
		break;
	default:
		newState = eAlignmentCalibrationMenuState::failedVideoStartStreamRequest;
		break;
	}

	// Create GUI panels
	// (Auto cleaned up on app state exit)
	{
		m_calibrationPanel = addGuiPanel<GuiPanel_AlignmentCalibration>();
		m_calibrationPanel->setBypassCalibrationFlag(m_bypassCalibrationFlag);
		m_calibrationPanel->OnBeginEvent = [this]() { onBeginEvent(); };
		m_calibrationPanel->OnRestartEvent = [this]() { onRestartEvent(); };
		m_calibrationPanel->OnCancelEvent = [this]() { onCancelEvent(); };
		m_calibrationPanel->OnReturnEvent = [this]() { onReturnEvent(); };
		m_calibrationPanel->OnChessboardStabilityChangedEvent =
			[this](bool bIsStable) { onChessboardStabilityChangedEvent(bIsStable); };

		m_cameraSettingsPanel = addGuiPanel<GuiPanel_AlignmentCameraSettings>();
		m_cameraSettingsPanel->setCameraDefinition(m_targetCameraComponent->getCameraDefinition());
		m_cameraSettingsPanel->OnViewpointModeChanged =
			[this](eAlignmentCalibrationViewpointMode mode) { onViewportModeChanged(mode); };
		m_cameraSettingsPanel->OnVRFrameDelayChanged =
			[this](int delay) { onVRFrameDelayChanged(delay); };
		m_cameraSettingsPanel->setViewpointMode(
			m_bypassCalibrationFlag
				? eAlignmentCalibrationViewpointMode::compositor
				: eAlignmentCalibrationViewpointMode::cameraViewpoint);
	}

	setMenuState(newState);
}

void AppStage_AlignmentCalibration::setupTrackerPoseCalibrator()
{
	// Allocate all distortion and video buffers
	m_monoDistortionView =
		new VideoFrameDistortionView(
			m_videoSourceComponent,
			VIDEO_FRAME_HAS_ALL);
	m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

	// Create a calibrator to do the actual pattern recording and calibration
	m_trackerPoseCalibrator =
		new MonoLensTrackerPoseCalibrator(
			m_targetCameraComponent,
			m_cameraPuckPose_VRSystemSpace,
			m_matPuckPose_VRSystemSpace,
			m_monoDistortionView,
			DESIRED_CAPTURE_BOARD_COUNT);
}

void AppStage_AlignmentCalibration::exit()
{
	setMenuState(eAlignmentCalibrationMenuState::inactive);

	m_mkCamera= nullptr;

	if (m_videoSourceComponent)
	{
		// Turn back off the video feed
		m_videoSourceComponent->stopVideoStream();
		m_videoSourceComponent = nullptr;
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
	switch (m_cameraSettingsPanel->getViewpointMode())
	{
	case eAlignmentCalibrationViewpointMode::cameraViewpoint:
	case eAlignmentCalibrationViewpointMode::scene:
		{
			// Nothing to do
		}
		break;
	case eAlignmentCalibrationViewpointMode::compositor:
		{
			bool bValidPose= false;

			// Update the transform of the camera so that vr models align over the tracking puck
			glm::mat4 cameraPose;
			if (m_calibrationPanel->getMenuState() == eAlignmentCalibrationMenuState::testCalibration)
			{
				// Use the calibrated aperture offset on the video source to get the camera pose
				bValidPose = m_targetCameraComponent->getSceneSpaceAperturePose(cameraPose);
			}
			else
			{
				// Use the last computed preview camera alignment
				bValidPose = 
					m_trackerPoseCalibrator->getLastSceneSpaceAperturePose(
						m_cameraPuckPose_VRSystemSpace, cameraPose);
			}

			if (bValidPose)
			{
				m_mkCamera->setCameraTransform(cameraPose);
			}
		}
		break;
	}
}

void AppStage_AlignmentCalibration::update(float deltaSeconds)
{
	updateCamera();

	switch(m_calibrationPanel->getMenuState())
	{
	case eAlignmentCalibrationMenuState::pendingVideoStart:
		{
			// Check if the video stream has started yet
			eVideoStreamingStatus status = m_videoSourceComponent->getVideoStreamingStatus();
			if (status == eVideoStreamingStatus::started)
			{
				// If it has, then setup the calibrator and move to the next state
				setupTrackerPoseCalibrator();
				setMenuState(
					m_bypassCalibrationFlag
						? eAlignmentCalibrationMenuState::testCalibration
						: eAlignmentCalibrationMenuState::verifySetup);
			}
			else if (status == eVideoStreamingStatus::stopped)
			{
				// If stopped, try to restart the video stream
				m_videoSourceComponent->startVideoStream();
			}
			else if (status == eVideoStreamingStatus::failed)
			{
				// If it failed to start, then move to the failed state
				setMenuState(eAlignmentCalibrationMenuState::failedVideoStartStreamRequest);
			}
		}
		break;
	case eAlignmentCalibrationMenuState::verifySetup:
		{
			// Update the video frame buffers to preview the calibration mat
			m_monoDistortionView->readAndProcessVideoFrame();

			// Look for a calibration pattern so that we can preview if it's in frame
			m_trackerPoseCalibrator->computeCameraToPuckXform();

			// See if we can compute a camera to puck transform this frame
			m_calibrationPanel->setCurrentChessboardValid(m_trackerPoseCalibrator->hasValidCameraToPuckXform());

			// Update the time that the chessboard has been stable for
			m_calibrationPanel->updateChessboardStabilityTimer(deltaSeconds);
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
				m_calibrationPanel->setCalibrationFraction(m_trackerPoseCalibrator->getCalibrationProgress());
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
					m_targetCameraComponent->getCameraDefinition()->setAperturePoseOffset(
						rotationOffset, translationOffset);

					// Go to the test calibration state
					m_cameraSettingsPanel->setViewpointMode(
						eAlignmentCalibrationViewpointMode::compositor);
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

void AppStage_AlignmentCalibration::render(IMkViewportPtr targetViewport)
{
	// Render the scene into the frame buffer
	if (m_frameBuffer->isValid())
	{
		MkScopedObjectBinding colorFramebufferBinding(
			m_ownerWindow->getGraphicsContext()->getMkStateStack().getCurrentState(),
			"Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
			switch (m_calibrationPanel->getMenuState())
			{
				case eAlignmentCalibrationMenuState::verifySetup:
					{
						switch (m_cameraSettingsPanel->getViewpointMode())
						{
							case eAlignmentCalibrationViewpointMode::cameraViewpoint:
								m_monoDistortionView->renderSelectedVideoBuffers();
								m_trackerPoseCalibrator->renderCameraSpaceCalibrationState();
								break;
							case eAlignmentCalibrationViewpointMode::scene:
								m_trackerPoseCalibrator->renderVRSpaceCalibrationState();
								renderVRScene();
								break;
							case eAlignmentCalibrationViewpointMode::compositor:
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
						if (m_cameraSettingsPanel->getViewpointMode() == eAlignmentCalibrationViewpointMode::compositor)
						{
							m_monoDistortionView->renderSelectedVideoBuffers();
						}

						renderVRScene();
					}
					break;
			}
		}

		// Render any lines and text that were added to the scene by the calibrator in the frame buffer's viewport
		m_ownerWindow->getGraphicsContext()->getLineRenderer()->render();
		m_ownerWindow->getGraphicsContext()->getTextRenderer()->render();
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

void AppStage_AlignmentCalibration::renderVRScene()
{
	IMkGraphicsContext* graphicsContext = getGraphicsContext();
	MkScene* scene= m_scene.get();

	// Rebuild list of renderables
	scene->removeAllInstances();

	// Add all renderable VR objects
	addAllVRDevicesToMkScene(getObjectSystemOfType<VRObjectSystem>(), m_scene);

	// Render the scene
	scene->render(m_mkCamera, graphicsContext->getMkStateStack());

	drawTransformedAxes(graphicsContext, glm::mat4(1.f), 1.0f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(graphicsContext, style, glm::vec3(1.f, 0.f, 0.f), L"X");
	drawTextAtWorldPosition(graphicsContext, style, glm::vec3(0.f, 1.f, 0.f), L"Y");
	drawTextAtWorldPosition(graphicsContext, style, glm::vec3(0.f, 0.f, 1.f), L"Z");
}

void AppStage_AlignmentCalibration::setMenuState(eAlignmentCalibrationMenuState newState)
{
	if (m_calibrationPanel->getMenuState() != newState)
	{
		eAlignmentCalibrationMenuState oldState= m_calibrationPanel->getMenuState();

		// Update menu state on the panels
		m_calibrationPanel->setMenuState(newState);
		m_cameraSettingsPanel->setMenuState(newState);

		// Broadcast the menu state change to the remote control manager
		{
			std::vector<std::string> parameters;
			parameters.push_back(k_alignmentCalibrationMenuStateStrings[(int)oldState]);
			parameters.push_back(k_alignmentCalibrationMenuStateStrings[(int)newState]);

			sendRemoteControlEvent("menu_state_changed", parameters);
		}
	}
}

// Calibration Model UI Events
void AppStage_AlignmentCalibration::onBeginEvent()
{
	tryBeginCapture();
}

bool AppStage_AlignmentCalibration::tryBeginCapture()
{
	if (m_calibrationPanel->getMenuState() == eAlignmentCalibrationMenuState::verifySetup)
	{
		// Clear out all of the calibration data we recorded
		m_trackerPoseCalibrator->resetCalibrationState();

		// Reset all calibration state on the calibration UI model
		m_calibrationPanel->setCalibrationFraction(0.f);

		// Go back to the camera viewpoint (in case we are in VR view)
		m_cameraSettingsPanel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);

		// Advance to the capture state
		setMenuState(eAlignmentCalibrationMenuState::capture);

		return true;
	}

	return false;
}

void AppStage_AlignmentCalibration::onRestartEvent()
{
	tryRestartCapture();
}

bool AppStage_AlignmentCalibration::tryRestartCapture()
{
	if (m_calibrationPanel->getMenuState() != eAlignmentCalibrationMenuState::verifySetup)
	{
		// Clear out all of the calibration data we recorded
		m_trackerPoseCalibrator->resetCalibrationState();

		// Reset all calibration state on the calibration UI model
		m_calibrationPanel->setCalibrationFraction(0.f);

		// Go back to the camera viewpoint (in case we are in VR view)
		m_cameraSettingsPanel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);

		// Return to the capture state
		setMenuState(eAlignmentCalibrationMenuState::verifySetup);

		return true;
	}

	return false;
}

void AppStage_AlignmentCalibration::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_AlignmentCalibration::onReturnEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_AlignmentCalibration::onChessboardStabilityChangedEvent(bool bIsStable)
{
	std::vector<std::string> parameters;
	parameters.push_back(bIsStable ? "true" : "false");

	sendRemoteControlEvent("chessboard_stability_changed", parameters);
}

// Camera Settings Model UI Events
void AppStage_AlignmentCalibration::onViewportModeChanged(eAlignmentCalibrationViewpointMode newViewMode)
{
	switch (newViewMode)
	{
		case eAlignmentCalibrationViewpointMode::cameraViewpoint:
			{
				m_mkCamera->setCameraMovementMode(eCameraMovementMode::stationary);
				m_mkCamera->setCameraTransform(glm::mat4(1.f));
			} break;
		case eAlignmentCalibrationViewpointMode::scene:
			{
				m_mkCamera->setCameraMovementMode(eCameraMovementMode::fly);
			} break;
		case eAlignmentCalibrationViewpointMode::compositor:
			{
				m_mkCamera->setCameraMovementMode(eCameraMovementMode::stationary);
			} break;
		default:
			break;
	}
}

void AppStage_AlignmentCalibration::onVRFrameDelayChanged(int newVRFrameDelay)
{
	m_targetCameraComponent->getCameraDefinition()->setTrackingFrameDelay(newVRFrameDelay);
}

// Remote Control
bool AppStage_AlignmentCalibration::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	if (command == "get_state")
	{
		return handleGetStateCommand(outResults);
	}
	else if (command == "get_chessboard_stability")
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

	return AppStage::handleRemoteControlCommand(command, parameters, outResults);
}

bool AppStage_AlignmentCalibration::handleGetStateCommand(
	std::vector<std::string>& outResults)
{
	const eAlignmentCalibrationMenuState menuState = m_calibrationPanel->getMenuState();
	const std::string& stateName = k_alignmentCalibrationMenuStateStrings[(int)menuState];

	outResults.push_back(stateName);

	return true;
}

bool AppStage_AlignmentCalibration::handleGetChessboardStabilityCommand(
	std::vector<std::string>& outResults)
{
	const bool bIsStable = m_calibrationPanel->getCurrentChessboardStable();
	outResults.push_back(bIsStable ? IRemoteControllable::k_true : IRemoteControllable::k_false);

	return true;
}

bool AppStage_AlignmentCalibration::handleBeginCommand(std::vector<std::string>& outResults)
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

bool AppStage_AlignmentCalibration::handleRestartCommand(std::vector<std::string>& outResults)
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

void AppStage_AlignmentCalibration::onGui()
{
	AppStage::onGui();

	constexpr float k_panelWidth = 415.f;
	const float displayWidth = m_ownerWindow->getWidth();
	const float displayHeight = m_ownerWindow->getHeight();

	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);
	constexpr ImGuiWindowFlags k_flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	MkGuiScopedWindow panel("##AlignmentCalibration", nullptr, k_flags);
	if (!panel) return;

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}