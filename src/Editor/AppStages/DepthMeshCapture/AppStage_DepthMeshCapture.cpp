//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "DepthMeshCapture/AppStage_DepthMeshCapture.h"
#include "DepthMeshCapture/RmlModel_DepthMeshCapture.h"
#include "DepthMeshCapture/RmlModel_DepthMeshCameraSettings.h"
#include "App.h"
#include "Colors.h"
#include "DepthMeshGenerator.h"
#include "EditorObjectSystem.h"
#include "MikanCamera.h"
#include "GlFrameCompositor.h"
#include "MkScene.h"
#include "IMkLineRenderer.h"
#include "IMkStaticMeshInstance.h"
#include "IMkTextRenderer.h"
#include "IMkTriangulatedMesh.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanRenderModelResource.h"
#include "MikanViewport.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanScene.h"
#include "ModelStencilComponent.h"
#include "ObjectSystemManager.h"
#include "ProfileConfig.h"
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
	, m_scene(std::make_shared<MikanScene>())
	, m_viewport(nullptr)
{
}

AppStage_DepthMeshCapture::~AppStage_DepthMeshCapture()
{
	delete m_calibrationModel;
	delete m_cameraSettingsModel;
}

void AppStage_DepthMeshCapture::setTargetModelStencil(ModelStencilDefinitionPtr definition)
{
	m_targetModelStencilDefinition= definition;
}

void AppStage_DepthMeshCapture::enter()
{
	AppStage::enter();

	// Cache object systems we'll be accessing
	ObjectSystemManagerPtr objectSystemManager = m_ownerWindow->getObjectSystemManager();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();

	// Get the current video source based on the config
	m_profile = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(m_profile->videoSourcePath).getCurrent();

	auto* vrDeviceManager = VRDeviceManager::getInstance();
	auto cameraTrackingPuckView= vrDeviceManager->getVRDeviceViewByPath(m_profile->cameraVRDevicePath);
	m_cameraTrackingPuckPoseView= cameraTrackingPuckView->makePoseView(eVRDevicePoseSpace::MikanScene);

	// Add all VR devices to the 3d scene
	VRDeviceList vrDeviceList= VRDeviceManager::getInstance()->getVRDeviceList();
	for (auto it : vrDeviceList)
	{
		it->getVRDeviceInterface()->bindToScene(m_scene->getMkScene());
	}

	// Setup viewport
	m_viewport = getFirstViewport();

	// Create and bind cameras
	setupCameras();

	// Register the scene with the primary viewport
	m_editorSystem->bindViewport(getFirstViewport());

	// Fire up the video scene in the background + depth estimator + mesh capture
	bool depthCaptureReady= false;
	//TODO: Handle pendingStart
	if ((int)m_videoSourceView->startVideoStream() > 0)
	{
		auto glFrameCompositor = m_ownerWindow->getFrameCompositor();

		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			std::make_shared<VideoFrameDistortionView>(
				m_ownerWindow,
				m_videoSourceView, 
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

#if REALTIME_DEPTH_ESTIMATION_ENABLED
		// Get the depth estimator from the frame compositor
		// (might have made one for depth visualization)
		m_syntheticDepthEstimator = glFrameCompositor->getSyntheticDepthEstimator();
#endif

		// If we don't have one, then make a new depth estimator ourselves
		if (!m_syntheticDepthEstimator)
		{
			m_syntheticDepthEstimator =
				std::make_shared<SyntheticDepthEstimator>(
					m_ownerWindow->getOpenCVManager(), 
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
					m_ownerWindow,
					m_profile,
					m_monoDistortionView,
					m_syntheticDepthEstimator);

			depthCaptureReady= true;
		}
	}

	eDepthMeshCaptureMenuState newState;
	if (depthCaptureReady)
	{
		// If bypassing the capture, then jump straight to the test capture state
		if (m_depthMeshCapture->loadMeshFromStencilDefinition(m_targetModelStencilDefinition))
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
		m_cameraSettingsModel->init(context, m_videoSourceView, m_profile);
		m_cameraSettingsModel->OnViewpointModeChanged = MakeDelegate(this, &AppStage_DepthMeshCapture::onViewportModeChanged);

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

	// Unregister all viewports from the editor
	m_editorSystem->clearViewports();
	m_editorSystem= nullptr;

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

	// Free the depth estimator
	m_syntheticDepthEstimator = nullptr;

	// Free the distortion view buffers
	m_monoDistortionView = nullptr;

	// Clean up the data model
	getRmlContext()->RemoveDataModel("depth_mesh_capture");
	getRmlContext()->RemoveDataModel("depth_mesh_camera_settings");

	AppStage::exit();
}

// Camera
void AppStage_DepthMeshCapture::setupCameras()
{
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);

	for (int cameraIndex = 0; cameraIndex < (int)eDepthMeshCaptureViewpointMode::COUNT; ++cameraIndex)
	{
		// Create a camera for the corresponding display mode if it doesn't exist
		if (cameraIndex == m_viewport->getCameraCount())
		{
			m_viewport->addCamera();
		}

		// Use fly-cam input control for every camera except for the first one
		MikanCameraPtr camera = m_viewport->getMikanCameraByIndex(cameraIndex);
		if (cameraIndex == 0)
			camera->setCameraMovementMode(eCameraMovementMode::stationary);
		else
			camera->setCameraMovementMode(eCameraMovementMode::fly);

		// Make sure all the cameras intrinsics are using the same fov as the video source
		camera->applyMonoCameraIntrinsics(&cameraIntrinsics);
	}

	// Default to the XR Camera view
	onViewportModeChanged(eDepthMeshCaptureViewpointMode::videoSourceViewpoint);
}

MikanCameraPtr AppStage_DepthMeshCapture::getViewpointCamera(eDepthMeshCaptureViewpointMode viewportMode) const
{
	return m_viewport->getMikanCameraByIndex((int)viewportMode);
}

void AppStage_DepthMeshCapture::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	if (m_calibrationModel->getMenuState() != eDepthMeshCaptureMenuState::failedToStart)
	{
		// Get the transform of the video source
		if (!m_cameraTrackingPuckPoseView ||
			!m_cameraTrackingPuckPoseView->getPose(m_videoSourceXform))
		{
			m_videoSourceXform= glm::mat4(1.f);
		}

		// Read the next video frame
		m_monoDistortionView->readAndProcessVideoFrame();
	}
}

void AppStage_DepthMeshCapture::render()
{
	AppStage::render();

	switch (m_cameraSettingsModel->getViewpointMode())
	{
		case eDepthMeshCaptureViewpointMode::videoSourceViewpoint:
			m_monoDistortionView->renderSelectedVideoBuffers();
			m_depthMeshCapture->renderCameraSpaceCalibrationState();
			break;
		case eDepthMeshCaptureViewpointMode::vrViewpoint:
			renderVRScene();
			break;
	}
}

void AppStage_DepthMeshCapture::renderVRScene()
{
	// Render the editor scene
	MikanCameraPtr vrCamera = getViewpointCamera(eDepthMeshCaptureViewpointMode::vrViewpoint);

	// Draw where the tracked camera is
	MikanCameraPtr videoSourceCamera = getViewpointCamera(eDepthMeshCaptureViewpointMode::videoSourceViewpoint);
	if (videoSourceCamera)
	{
		// Draw the frustum for the initial camera pose
		const float hfov_radians = degrees_to_radians(videoSourceCamera->getHorizontalFOVDegrees());
		const float vfov_radians = degrees_to_radians(videoSourceCamera->getVerticalFOVDegrees());
		const float zNear = fmaxf(videoSourceCamera->getZNear(), 0.1f);
		const float zFar = fminf(videoSourceCamera->getZFar(), 2.0f);

		drawTransformedFrustum(
			m_videoSourceXform,
			hfov_radians, vfov_radians,
			zNear, zFar,
			Colors::Yellow);
		drawTransformedAxes(m_videoSourceXform, 0.1f);
	}

	// Draw tracking space
	drawGrid(glm::mat4(1.f), 10.f, 10.f, 20, 20, Colors::GhostWhite);
	if (m_profile->getRenderOriginFlag())
	{
		TextStyle style = getDefaultTextStyle();

		drawTransformedAxes(glm::mat4(1.f), 1.f, 1.f, 1.f);
		drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
	}

	// Draw any meshes added to the scene (inlcuding the depth capture mesh)
	m_scene->render(vrCamera, m_ownerWindow->getMkStateStack());
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

		// Remove any previously captured meshes from the scene
		removeDepthMeshResourceFromScene();

		if (newState == eDepthMeshCaptureMenuState::testCapture)
		{
			// Go to the VR viewpoint by default when we are in the test capture state
			m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::vrViewpoint);

			// If entering the test capture state, then add the captured mesh to the scene
			addDepthMeshResourcesToScene();
		}
		else
		{
			m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::videoSourceViewpoint);
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
				setMenuState(eDepthMeshCaptureMenuState::captureFailed);
			}
		}
		break;
	case eDepthMeshCaptureMenuState::testCapture:
		{
			// Write out the mesh to a file
			m_depthMeshCapture->saveMeshToStencilDefinition(
				m_targetModelStencilDefinition,
				m_videoSourceXform);

			m_ownerWindow->popAppState();
		}
		break;
	}
}

void AppStage_DepthMeshCapture::onRestartEvent()
{
	// Go back to the camera viewpoint (in case we are in VR view)
	m_cameraSettingsModel->setViewpointMode(eDepthMeshCaptureViewpointMode::videoSourceViewpoint);

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
	m_viewport->setCurrentCamera((int)m_cameraSettingsModel->getViewpointMode());
}

// GlScene Helpers
void AppStage_DepthMeshCapture::addDepthMeshResourcesToScene()
{
	auto depthMeshResource= m_depthMeshCapture->getCapturedDepthMeshResource();
	if (depthMeshResource != nullptr)
	{
		for (int meshIndex = 0; meshIndex < depthMeshResource->getTriangulatedMeshCount(); meshIndex++)
		{
			auto mesh= depthMeshResource->getTriangulatedMesh(meshIndex);
			IMkStaticMeshInstancePtr meshInstance= createMkStaticMeshInstance(mesh->getName(), mesh);

			// Set the model matrix to the video source transform 
			// since that is what the depth data is relative to
			meshInstance->setModelMatrix(m_videoSourceXform);
			meshInstance->setVisible(true);

			// Register the mesh instance with the scene
			m_scene->getMkScene()->addInstance(meshInstance);

			// Need to keep track of mesh instances locally
			// since the scene uses weak pointers
			m_depthMeshInstances.push_back(meshInstance);
		}
	}
}

void AppStage_DepthMeshCapture::removeDepthMeshResourceFromScene()
{
	for (IMkStaticMeshInstancePtr depthMeshInstance : m_depthMeshInstances)
	{
		m_scene->getMkScene()->removeInstance(depthMeshInstance);
	}
	m_depthMeshInstances.clear();
}