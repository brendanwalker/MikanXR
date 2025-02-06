//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "StencilAlignment/AppStage_StencilAlignment.h"
#include "StencilAlignment/RmlModel_StencilAlignment.h"
#include "App.h"
#include "Colors.h"
#include "CalibrationRenderHelpers.h"
#include "GlCamera.h"
#include "GlFrameBuffer.h"
#include "GlLineRenderer.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlScopedObjectBinding.h"
#include "GlStaticMeshInstance.h"
#include "GlScene.h"
#include "GlStateStack.h"
#include "GlTextRenderer.h"
#include "GlTriangulatedMesh.h"
#include "GlViewport.h"
#include "IMkWireframeMesh.h"
#include "InputManager.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MeshColliderComponent.h"
#include "ModelStencilComponent.h"
#include "ProfileConfig.h"
#include "StencilObjectSystem.h"
#include "StencilComponent.h"
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
const char* AppStage_StencilAlignment::APP_STAGE_NAME = "StencilAlignment";

//-- public methods -----
AppStage_StencilAlignment::AppStage_StencilAlignment(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_StencilAlignment::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_StencilAlignment)
	, m_videoSourceView()
	, m_stencilAligner(nullptr)
	, m_monoDistortionView(nullptr)
	, m_scene(std::make_shared<GlScene>())
	, m_camera(nullptr)
	, m_frameBuffer(std::make_shared<GlFrameBuffer>())
	, m_fullscreenQuad(createFullscreenQuadMesh(ownerWindow, false))
{
}

AppStage_StencilAlignment::~AppStage_StencilAlignment()
{
	delete m_calibrationModel;
}

void AppStage_StencilAlignment::enter()
{
	AppStage::enter();

	// Get the current video source based on the config
	ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();
	m_videoSourceView = 
		VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();

	// Get the pose view for the camera tracking puck in Mikan Scene space
	auto* vrDeviceManager = VRDeviceManager::getInstance();
	auto cameraTrackingPuckView = vrDeviceManager->getVRDeviceViewByPath(profileConfig->cameraVRDevicePath);
	m_cameraTrackingPuckPoseView = cameraTrackingPuckView->makePoseView(eVRDevicePoseSpace::MikanScene);

	// Listen for mouse ray events
	GlViewportPtr viewport= getFirstViewport();
	viewport->OnMouseRayChanged+= MakeDelegate(this, &AppStage_StencilAlignment::onMouseRayChanged);
	viewport->OnMouseRayButtonUp+= MakeDelegate(this, &AppStage_StencilAlignment::onMouseRayButtonUp);

	// Create a new camera to view the scene
	m_camera = viewport->getCurrentCamera();
	m_camera->setCameraMovementMode(eCameraMovementMode::stationary);

	// Center the orbit camera on the stencil model
	updateVRCamera();

	// Make sure the camera doing the 3d rendering has the same
	// fov and aspect ration as the real camera
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_videoSourceView->getCameraIntrinsics(cameraIntrinsics);
	m_camera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// Create a frame buffer to render the scene into using the resolution and fov from the camera intrinsics
	const MikanMonoIntrinsics& monoIntrinsics = cameraIntrinsics.getMonoIntrinsics();
	m_frameBuffer->setName("StencilAlignment");
	m_frameBuffer->setSize(monoIntrinsics.pixel_width, monoIntrinsics.pixel_height);
	m_frameBuffer->setFrameBufferType(GlFrameBuffer::eFrameBufferType::COLOR);
	m_frameBuffer->createResources();

	// Add the stencil's wireframe meshes to the scene
	if (m_targetStencilComponent)
	{
		for (GlStaticMeshInstancePtr meshInstance : m_targetStencilComponent->getWireframeMeshes())
		{
			m_scene->addInstance(meshInstance);
		}
	}

	// Fire up the video scene in the background + pose calibrator
	eStencilAlignmentMenuState newState;
	if (m_videoSourceView->startVideoStream())
	{
		// Allocate all distortion and video buffers
		m_monoDistortionView = 
			new VideoFrameDistortionView(
				m_ownerWindow,
				m_videoSourceView, 
				VIDEO_FRAME_HAS_ALL);
		m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);

		// Create a aligner to calibrate the stencil
		m_stencilAligner =
			new StencilAligner(
				m_cameraTrackingPuckPoseView,
				m_monoDistortionView,
				m_targetStencilComponent);

		newState = eStencilAlignmentMenuState::verifyInitialCameraSetup;
	}
	else
	{
		newState = eStencilAlignmentMenuState::failedVideoStartStreamRequest;
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnOkEvent = MakeDelegate(this, &AppStage_StencilAlignment::onOkEvent);
		m_calibrationModel->OnRedoEvent = MakeDelegate(this, &AppStage_StencilAlignment::onRedoEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_StencilAlignment::onCancelEvent);

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("stencil_alignment.rml");
	}

	setMenuState(newState);
}

void AppStage_StencilAlignment::exit()
{
	setMenuState(eStencilAlignmentMenuState::inactive);

	// Stop listening to mouse ray events
	GlViewportPtr viewport = getFirstViewport();
	viewport->OnMouseRayChanged -= MakeDelegate(this, &AppStage_StencilAlignment::onMouseRayChanged);
	viewport->OnMouseRayButtonUp -= MakeDelegate(this, &AppStage_StencilAlignment::onMouseRayButtonUp);

	// Forget about the stencil we were aligning
	m_targetStencilComponent= nullptr;

	// Forget about the camera
	m_camera= nullptr;

	// Forget about the stencil model we added
	m_scene->removeAllInstances();

	if (m_videoSourceView)
	{
		// Turn back off the video feed
		m_videoSourceView->stopVideoStream();
		m_videoSourceView = nullptr;
	}

	// Free the aligner
	if (m_stencilAligner != nullptr)
	{
		delete m_stencilAligner;
		m_stencilAligner = nullptr;
	}

	// Free the distortion view buffers
	if (m_monoDistortionView != nullptr)
	{
		delete m_monoDistortionView;
		m_monoDistortionView = nullptr;
	}

	AppStage::exit();
}

void AppStage_StencilAlignment::updateXRCamera()
{
	// Update the transform of the camera so that vr models align over the tracking puck
	glm::mat4 cameraPose;	
	if (m_videoSourceView->getCameraPose(m_cameraTrackingPuckPoseView, cameraPose))
	{
		m_camera->setCameraTransform(cameraPose);
	}
}

void AppStage_StencilAlignment::updateVRCamera()
{
	if (!m_targetStencilComponent)
		return;

	bool bValidBoundingSphere = false;

	m_boundingSphereCenter= glm::vec3();
	m_boundingSphereRadius= 1.f;

	for (auto colliderComponent : m_targetStencilComponent->getColliderComponents())
	{
		glm::vec3 colliderCenter;
		float colliderRadius;
		if (colliderComponent->getBoundingSphere(colliderCenter, colliderRadius))
		{
			if (bValidBoundingSphere)
			{
				glm_sphere_union(
					m_boundingSphereCenter, m_boundingSphereRadius,
					colliderCenter, colliderRadius,
					m_boundingSphereCenter, m_boundingSphereRadius);

			}
			else
			{
				m_boundingSphereCenter = colliderCenter;
				m_boundingSphereRadius = colliderRadius;
				bValidBoundingSphere = true;
			}
		}
	}

	if (bValidBoundingSphere)
	{
		m_camera->setOrbitTargetPosition(m_boundingSphereCenter);
		m_camera->setOrbitLocation(0.f, 0.f, m_boundingSphereRadius * 5.0f);
	}
}

void AppStage_StencilAlignment::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	switch (m_calibrationModel->getMenuState())
	{
		case eStencilAlignmentMenuState::verifyInitialCameraSetup:
		case eStencilAlignmentMenuState::verifyPointsCapture:
		case eStencilAlignmentMenuState::captureOriginPixel:
		case eStencilAlignmentMenuState::captureXAxisPixel:
		case eStencilAlignmentMenuState::captureYAxisPixel:
		case eStencilAlignmentMenuState::captureZAxisPixel:
			{
				m_monoDistortionView->readAndProcessVideoFrame();
				updateXRCamera();
			}
			break;
		case eStencilAlignmentMenuState::captureOriginVertex:
		case eStencilAlignmentMenuState::captureXAxisVertex:
		case eStencilAlignmentMenuState::captureYAxisVertex:
		case eStencilAlignmentMenuState::captureZAxisVertex:
			{
			}
			break;
		case eStencilAlignmentMenuState::testCalibration:
			{
				m_monoDistortionView->readAndProcessVideoFrame();
			}
			break;
	}
}

void AppStage_StencilAlignment::render()
{
	// Render the scene into the frame buffer
	if (m_frameBuffer->isValid())
	{
		GlScopedObjectBinding colorFramebufferBinding(
			*m_ownerWindow->getGlStateStack().getCurrentState(),
			"Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
			switch (m_calibrationModel->getMenuState())
			{
				case eStencilAlignmentMenuState::verifyInitialCameraSetup:
				case eStencilAlignmentMenuState::verifyPointsCapture:
				case eStencilAlignmentMenuState::captureOriginPixel:
				case eStencilAlignmentMenuState::captureXAxisPixel:
				case eStencilAlignmentMenuState::captureYAxisPixel:
				case eStencilAlignmentMenuState::captureZAxisPixel:
					{
						m_monoDistortionView->renderSelectedVideoBuffers();
						m_stencilAligner->renderPixelSamples();
					}
					break;
				case eStencilAlignmentMenuState::captureOriginVertex:
				case eStencilAlignmentMenuState::captureXAxisVertex:
				case eStencilAlignmentMenuState::captureYAxisVertex:
				case eStencilAlignmentMenuState::captureZAxisVertex:
					{
						renderStencilScene();
						m_stencilAligner->renderVertexSamples();
					}
					break;
				case eStencilAlignmentMenuState::testCalibration:
					{
						m_monoDistortionView->renderSelectedVideoBuffers();
						m_stencilAligner->renderVertexSamples();
						renderStencilScene();
					}
					break;
			}

			// Render any lines and text that were added to the scene by the calibrator in the frame buffer's viewport
			m_ownerWindow->getLineRenderer()->render();
			m_ownerWindow->getTextRenderer()->render();
		}
	}

	// Render the frame buffer to the screen
	if (m_frameBuffer->isValid())
	{
		GlMaterialInstancePtr materialInstance = m_fullscreenQuad->getMaterialInstance();
		GlMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			auto colorTexture = m_frameBuffer->getColorTexture();

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

void AppStage_StencilAlignment::renderStencilScene()
{
	m_scene->render(m_camera, m_ownerWindow->getGlStateStack());

	if (m_targetStencilComponent)
	{
		// Draw the stencil's local axes
		glm::mat4 stencilXform= m_targetStencilComponent->getWorldTransform();
		drawTransformedAxes(stencilXform, m_boundingSphereRadius * 1.1f, true);

		if (m_hoverResult.hitValid)
		{
			// Draw collision normal
			drawSegment(
				glm::mat4(1.f),
				m_hoverResult.hitLocation,
				m_hoverResult.hitLocation + m_hoverResult.hitNormal*0.01f,
				Colors::Green);

			// Draw the closest vertex to the collision point
			drawSegment(
				glm::mat4(1.f),
				m_hoverResult.closestVertexWorld,
				m_hoverResult.closestVertexWorld + m_hoverResult.hitNormal * 0.01f,
				Colors::Yellow);
		}
	}
}

void AppStage_StencilAlignment::setMenuState(eStencilAlignmentMenuState newState)
{
	eStencilAlignmentMenuState oldState= m_calibrationModel->getMenuState();

	if (oldState != newState)
	{
		switch (newState)
		{
			case eStencilAlignmentMenuState::captureOriginVertex:
			case eStencilAlignmentMenuState::captureXAxisVertex:
			case eStencilAlignmentMenuState::captureYAxisVertex:
			case eStencilAlignmentMenuState::captureZAxisVertex:
				m_camera->setCameraMovementMode(eCameraMovementMode::orbit);
				break;
			default:
				m_camera->setCameraMovementMode(eCameraMovementMode::stationary);
		}

		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
	}
}

// Viewpoint Events
void AppStage_StencilAlignment::onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	eStencilAlignmentMenuState menuState = m_calibrationModel->getMenuState();
	if (menuState == eStencilAlignmentMenuState::captureOriginVertex ||
		menuState == eStencilAlignmentMenuState::captureXAxisVertex ||
		menuState == eStencilAlignmentMenuState::captureYAxisVertex ||
		menuState == eStencilAlignmentMenuState::captureZAxisVertex)
	{
		m_hoverResult = ColliderRaycastHitResult();

		if (m_targetStencilComponent)
		{
			glm::vec3 closestWorldspacePoint;
			float closestDistance = std::numeric_limits<float>::max();

			for (auto colliderComponent : m_targetStencilComponent->getColliderComponents())
			{
				ColliderRaycastHitRequest request = {rayOrigin, rayDir};
				ColliderRaycastHitResult result;

				if (colliderComponent->computeRayIntersection(request, result) &&
					result.hitDistance < closestDistance)
				{
					m_hoverResult = result;
					closestDistance = result.hitDistance;
				}
			}

			// Update the color of the wireframe meshes based on the hover result
			glm::vec3 newColor = m_hoverResult.hitValid ? Colors::LightGray : Colors::DarkGray;
			for (GlStaticMeshInstancePtr meshInstance : m_targetStencilComponent->getWireframeMeshes())
			{
				meshInstance->getMaterialInstance()->setVec4BySemantic(
					eUniformSemantic::diffuseColorRGBA,
					glm::vec4(newColor, 1.f));
			}
		}
	}
}

void AppStage_StencilAlignment::onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	if (button != SDL_BUTTON_LEFT)
		return;

	eStencilAlignmentMenuState menuState= m_calibrationModel->getMenuState();
	if (menuState >= eStencilAlignmentMenuState::captureOriginPixel &&
		menuState <= eStencilAlignmentMenuState::captureZAxisVertex)
	{
		bool bValidSample = false;

		switch (menuState)
		{
			case eStencilAlignmentMenuState::captureOriginPixel:
			case eStencilAlignmentMenuState::captureXAxisPixel:
			case eStencilAlignmentMenuState::captureYAxisPixel:
			case eStencilAlignmentMenuState::captureZAxisPixel:
				{
					GlViewportPtr viewport= getFirstViewport();

					// Get the cursor position in the window viewport
					glm::vec2 viewportPixel;
					if (viewport->getCursorViewportPixelPos(viewportPixel))
					{
						// Remap from window viewport size to the frame buffer size
						glm::i32vec2 windowViewportSize= viewport->getViewportSize();
						glm::vec2 frameBufferPixel= remapPointIntoTarget(
							(float)windowViewportSize.x, (float)windowViewportSize.y,
							0.f, 0.f,
							m_frameBuffer->getWidth(), m_frameBuffer->getHeight(),
							viewportPixel);

						// Record the pixel location sample
						m_stencilAligner->samplePixel(frameBufferPixel);
						bValidSample = true;
					}
				}
				break;
			case eStencilAlignmentMenuState::captureOriginVertex:
			case eStencilAlignmentMenuState::captureXAxisVertex:
			case eStencilAlignmentMenuState::captureYAxisVertex:
			case eStencilAlignmentMenuState::captureZAxisVertex:
				if (m_hoverResult.closestVertexValid)
				{
					m_stencilAligner->sampleVertex(m_hoverResult.closestVertexLocal);
					bValidSample = true;
				}
				break;
		}

		if (bValidSample)
		{
			eStencilAlignmentMenuState newMenuState = (eStencilAlignmentMenuState)((int)menuState + 1);

			setMenuState(newMenuState);
		}
	}
}

// Calibration Model UI Events
void AppStage_StencilAlignment::onOkEvent()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eStencilAlignmentMenuState::verifyInitialCameraSetup:
			{
				// Clear out all of the calibration data we recorded
				m_stencilAligner->resetCalibrationState();

				setMenuState(eStencilAlignmentMenuState::captureOriginPixel);
			} break;
		case eStencilAlignmentMenuState::verifyPointsCapture:
			{

				glm::mat4 newStencilTransform;
				if (m_stencilAligner->computeStencilTransform(newStencilTransform))
				{
					m_targetStencilComponent->setWorldTransform(newStencilTransform);
				}

				setMenuState(eStencilAlignmentMenuState::testCalibration);
			} break;
		case eStencilAlignmentMenuState::testCalibration:
		case eStencilAlignmentMenuState::failedVideoStartStreamRequest:
			{
				m_ownerWindow->popAppState();
			} break;
	}
}

void AppStage_StencilAlignment::onRedoEvent()
{
	// Clear out all of the calibration data we recorded
	m_stencilAligner->resetCalibrationState();

	// Return to the capture state
	setMenuState(eStencilAlignmentMenuState::verifyInitialCameraSetup);
}

void AppStage_StencilAlignment::onCancelEvent()
{
	m_ownerWindow->popAppState();
}