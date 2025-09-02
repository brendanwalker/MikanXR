// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "AlignmentCalibration/RmlModel_AlignmentCalibration.h"
#include "AlignmentCalibration/RmlModel_AlignmentCameraSettings.h"
#include "CalibrationPatternFinder.h"
#include "CameraComponent.h"
#include "Colors.h"
#include "MkScene.h"
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
#include "VideoSourceComponent.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceSystem.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"
#include "VRTrackingSystemDefinition.h"

#include "SDL_keycode.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/ext/vector_float4.hpp"

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
AppStage_AlignmentCalibration::AppStage_AlignmentCalibration(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_AlignmentCalibration::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_AlignmentCalibration)
	, m_cameraSettingsModel(new RmlModel_AlignmentCameraSettings)
	, m_targetCameraComponent()
	, m_videoSourceComponent()
	, m_trackerPoseCalibrator(nullptr)
	, m_monoDistortionView(nullptr)
	, m_scene(std::make_shared<MkScene>())
	, m_mkCamera(nullptr)
	, m_frameBuffer(createMkFrameBuffer())
	, m_fullscreenQuad(createFullscreenQuadMesh(ownerWindow, false))
{
}

AppStage_AlignmentCalibration::~AppStage_AlignmentCalibration()
{
	delete m_calibrationModel;
	delete m_cameraSettingsModel;
}

void AppStage_AlignmentCalibration::setBypassCalibrationFlag(bool flag)
{
	m_calibrationModel->setBypassCalibrationFlag(flag);
}

void AppStage_AlignmentCalibration::setTargetCameraComponent(CameraComponentPtr cameraComponent)
{
	m_targetCameraComponent = cameraComponent;
}

void AppStage_AlignmentCalibration::enter()
{
	AppStage::enter();
	assert(m_targetCameraComponent != nullptr);

	// Bind components needed for calibration
	m_videoSourceComponent = m_targetCameraComponent->getVideoSourceComponent();
	m_matTrackingPuckPoseView = makeMatPoseViewFromCamera(m_targetCameraComponent);

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
	//TODO: Handle pendingStart
	if ((int)m_videoSourceComponent->startVideoStream() > 0)
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_ownerWindow,
				m_videoSourceComponent,
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Create a calibrator to do the actual pattern recording and calibration
		m_trackerPoseCalibrator =
			new MonoLensTrackerPoseCalibrator(
				m_targetCameraComponent,
				m_matTrackingPuckPoseView,
				m_monoDistortionView,
				DESIRED_CAPTURE_BOARD_COUNT);

		// If bypassing the calibration, then jump straight to the test calibration state
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			newState = eAlignmentCalibrationMenuState::testCalibration;
		}
		else
		{
			newState = eAlignmentCalibrationMenuState::verifySetup;
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
		m_calibrationModel->OnChessboardStabilityChangedEvent = 
			MakeDelegate(this, &AppStage_AlignmentCalibration::onChessboardStabilityChangedEvent);

		// Init camera settings model
		m_cameraSettingsModel->init(context, m_targetCameraComponent->getCameraDefinition());
		m_cameraSettingsModel->OnViewpointModeChanged = MakeDelegate(this, &AppStage_AlignmentCalibration::onViewportModeChanged);
		m_cameraSettingsModel->OnVRFrameDelayChanged = MakeDelegate(this, &AppStage_AlignmentCalibration::onVRFrameDelayChanged);
		if (m_calibrationModel->getBypassCalibrationFlag())
		{
			m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::compositor);
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
	switch (m_cameraSettingsModel->getViewpointMode())
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
			if (m_calibrationModel->getMenuState() == eAlignmentCalibrationMenuState::testCalibration)
			{
				// Use the calibrated offset on the video source to get the camera pose
				m_targetCameraComponent->getAperturePose(cameraPose);
			}
			else
			{
				// Use the last computed preview camera alignment
				bValidPose = 
					m_trackerPoseCalibrator->getLastCameraPose(
						m_cameraTrackingPuckPoseView, cameraPose);
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

	switch(m_calibrationModel->getMenuState())
	{
		case eAlignmentCalibrationMenuState::verifySetup:
			{
				// Update the video frame buffers to preview the calibration mat
				m_monoDistortionView->readAndProcessVideoFrame();

				// Look for a calibration pattern so that we can preview if it's in frame
				m_trackerPoseCalibrator->computeCameraToPuckXform();

				// See if we can compute a camera to puck transform this frame
				m_calibrationModel->setCurrentChessboardValid(m_trackerPoseCalibrator->hasValidCameraToPuckXform());

				// Update the time tha the chessboard has been stable for
				m_calibrationModel->updateChessboardStabilityTimer(deltaSeconds);
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
						m_targetCameraComponent->getCameraDefinition()->setAperturePoseOffset(
							rotationOffset, translationOffset);

						// Go to the test calibration state
						m_cameraSettingsModel->setViewpointMode(
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
			m_ownerWindow->getMkStateStack().getCurrentState(),
			"Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
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
						if (m_cameraSettingsModel->getViewpointMode() == eAlignmentCalibrationViewpointMode::compositor)
						{
							m_monoDistortionView->renderSelectedVideoBuffers();
						}

						renderVRScene();
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
		MkMaterialInstancePtr materialInstance = m_fullscreenQuad->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

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

void AppStage_AlignmentCalibration::renderVRScene()
{
	MkScene* scene= m_scene.get();

	// Rebuild list of renderables
	scene->removeAllInstances();

	// Add all renderable VR objects
	addAllVRDevicesToMkScene(m_scene);

	// Render the scene
	scene->render(m_mkCamera, m_ownerWindow->getMkStateStack());

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
		eAlignmentCalibrationMenuState oldState= m_calibrationModel->getMenuState();

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
	if (m_calibrationModel->getMenuState() == eAlignmentCalibrationMenuState::verifySetup)
	{
		// Clear out all of the calibration data we recorded
		m_trackerPoseCalibrator->resetCalibrationState();

		// Reset all calibration state on the calibration UI model
		m_calibrationModel->setCalibrationFraction(0.f);

		// Go back to the camera viewpoint (in case we are in VR view)
		m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);

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
	if (m_calibrationModel->getMenuState() != eAlignmentCalibrationMenuState::verifySetup)
	{
		// Clear out all of the calibration data we recorded
		m_trackerPoseCalibrator->resetCalibrationState();

		// Reset all calibration state on the calibration UI model
		m_calibrationModel->setCalibrationFraction(0.f);

		// Go back to the camera viewpoint (in case we are in VR view)
		m_cameraSettingsModel->setViewpointMode(eAlignmentCalibrationViewpointMode::cameraViewpoint);

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
	if (!IRemoteControllableAppStage::handleRemoteControlCommand(command, parameters, outResults))
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
	}

	return false;
}

bool AppStage_AlignmentCalibration::handleGetStateCommand(
	std::vector<std::string>& outResults)
{
	const eAlignmentCalibrationMenuState menuState = m_calibrationModel->getMenuState();
	const std::string& stateName = k_alignmentCalibrationMenuStateStrings[(int)menuState];

	outResults.push_back(stateName);

	return true;
}

bool AppStage_AlignmentCalibration::handleGetChessboardStabilityCommand(
	std::vector<std::string>& outResults)
{
	const bool bIsStable = m_calibrationModel->getCurrentChessboardStable();
	outResults.push_back(bIsStable ? "true" : "false");

	return true;
}

bool AppStage_AlignmentCalibration::handleBeginCommand(std::vector<std::string>& outResults)
{
	if (tryBeginCapture())
	{
		outResults.push_back("success");
	}
	else
	{
		outResults.push_back("failure");
	}

	return true;
}

bool AppStage_AlignmentCalibration::handleRestartCommand(std::vector<std::string>& outResults)
{
	if (tryRestartCapture())
	{
		outResults.push_back("success");
	}
	else
	{
		outResults.push_back("failure");
	}

	return true;
}

VRDevicePoseViewPtr AppStage_AlignmentCalibration::makeMatPoseViewFromCamera(CameraComponentPtr m_targetCameraComponent)
{
	VRTrackingSystemDefinitionConstPtr vrTrackingSystem =
		m_targetCameraComponent->getVRTrackingSystemDefinition();

	if (!vrTrackingSystem)
	{
		MikanTrackingMountID matMountId = vrTrackingSystem->getCharucoTrackingMountId();

		if (matMountId != INVALID_MIKAN_ID)
		{
			TrackingMountDefinitionConstPtr matTrackingMount =
				vrTrackingSystem->getTrackingMountDefinitionConst(matMountId);
			if (matTrackingMount)
			{
				auto vrSystem = getSystemOfType<VRObjectSystem>();
				VRDeviceComponentPtr matTrackingPuck =
					vrSystem->getVRDeviceByPath(matTrackingMount->getDevicePath());

				if (matTrackingPuck)
				{
					return matTrackingPuck->makePoseView(eVRDevicePoseSpace::VRTrackingSystem);
				}
			}
		}
	}

	// Return an empty pose view if we couldn't find the MAT device
	return std::make_shared<VRDevicePoseView>();
}