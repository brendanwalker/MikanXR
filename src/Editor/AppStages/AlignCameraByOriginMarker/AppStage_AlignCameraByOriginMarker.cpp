//-- includes -----
#include "AlignCameraByOriginMarker/AppStage_AlignCameraByOriginMarker.h"
#include "AlignCameraByOriginMarker/GuiPanel_AlignCameraByOriginMarker.h"
#include "ModalMessageBox/ModalDialog_MessageBox.h"
#include "ArucoMarkerPoseSampler.h"
#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "Colors.h"
#include "IMkFrameBuffer.h"
#include "IMkLineRenderer.h"
#include "IMkTriangulatedMesh.h"
#include "IMkTexture.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "MikanCamera.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanViewport.h"
#include "MkGuiScopedWindow.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkScopedObjectBinding.h"
#include "MkStateStack.h"
#include "StageComponent.h"
#include "TextStyle.h"
#include "VideoDisplayConstants.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"

#include "glm/gtc/quaternion.hpp"

#include "imgui.h"

//-- statics ----
const char* AppStage_AlignCameraByOriginMarker::APP_STAGE_NAME = "AlignCameraByOriginMarker";

//-- public methods -----
AppStage_AlignCameraByOriginMarker::AppStage_AlignCameraByOriginMarker(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_AlignCameraByOriginMarker::APP_STAGE_NAME)
	, m_frameBuffer(createMkFrameBuffer())
	, m_fullscreenQuad(createFullscreenQuadMesh(ownerWindow->getGraphicsContext().get(), false))
{
}

AppStage_AlignCameraByOriginMarker::~AppStage_AlignCameraByOriginMarker()
{
}

bool AppStage_AlignCameraByOriginMarker::tryEnterCalibration(
	AppStage* fromAppStage,
	CameraComponentPtr targetCameraComponent)
{
	IEditorWindow* ownerWindow = fromAppStage->getOwnerWindow();

	// 1. Target camera must have a valid video source with valid intrinsics
	VideoSourceComponentPtr videoSource = targetCameraComponent->getVideoSourceComponent();
	if (!videoSource)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera has no video source. Please assign a video source before aligning.");
		return false;
	}

	if (!videoSource->areCameraIntrinsicsValid())
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera does not have valid aperture intrinsics. Please calibrate the camera's intrinsics first.");
		return false;
	}

	// 2. Target camera must NOT have a tracking mount (this method is for static cameras)
	if (targetCameraComponent->hasValidTrackingMountComponent())
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera already has a tracking mount. Use the standard alignment calibration instead.");
		return false;
	}

	// 3. Owner stage must have a marker tracking volume
	StageComponentConstPtr ownerStage = targetCameraComponent->getOwnerStageComponent();
	if (!ownerStage)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera has no owner stage.");
		return false;
	}

	TrackingVolumeComponentConstPtr trackingVolume = ownerStage->getTrackingVolumeConst();
	if (!trackingVolume || trackingVolume->getTrackingVolumeType() != eTrackingVolumeType::marker)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera's stage has no marker tracking volume. Please assign a marker tracking volume to the stage.");
		return false;
	}

	// 4. Tracking volume must have a valid origin marker assigned
	const MikanMarkerID originMarkerId =
		trackingVolume->getTrackingVolumeDefinition()->getOriginMarkerId();
	if (originMarkerId == INVALID_MIKAN_ID)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"The stage's marker tracking volume has no Origin Marker assigned. Please assign an origin marker to the tracking volume.");
		return false;
	}

	auto* appStage = ownerWindow->pushAppStageOfType<AppStage_AlignCameraByOriginMarker>();
	appStage->setTargetCameraComponent(targetCameraComponent);

	return true;
}

void AppStage_AlignCameraByOriginMarker::setTargetCameraComponent(CameraComponentPtr cameraComponent)
{
	m_targetCameraComponent = cameraComponent;
}

// -- AppStage -- //
void AppStage_AlignCameraByOriginMarker::enter()
{
	AppStage::enter();
	assert(m_targetCameraComponent != nullptr);

	// Fetch the origin marker ID from the tracking volume
	StageComponentConstPtr ownerStage = m_targetCameraComponent->getOwnerStageComponent();
	assert(ownerStage != nullptr);
	TrackingVolumeComponentConstPtr trackingVolume = ownerStage->getTrackingVolumeConst();
	assert(trackingVolume != nullptr);
	m_originMarkerId = trackingVolume->getTrackingVolumeDefinition()->getOriginMarkerId();

	// Cache the target video source
	m_targetVideoSource = m_targetCameraComponent->getVideoSourceComponent();
	assert(m_targetVideoSource != nullptr);

	// Create GUI panel
	m_calibrationPanel = addGuiPanel<GuiPanel_AlignCameraByOriginMarker>();
	m_calibrationPanel->OnBeginEvent   = [this]() { onBeginEvent(); };
	m_calibrationPanel->OnRestartEvent = [this]() { onRestartEvent(); };
	m_calibrationPanel->OnCancelEvent  = [this]() { onCancelEvent(); };
	m_calibrationPanel->OnReturnEvent  = [this]() { onReturnEvent(); };

	// Start the video stream immediately (no source camera selection needed)
	startVideoStream();
}

void AppStage_AlignCameraByOriginMarker::exit()
{
	setMenuState(eAlignCameraByOriginMarkerMenuState::inactive);

	// Stop and release target video
	if (m_targetVideoSource)
	{
		m_targetVideoSource->stopVideoStream();
		m_targetVideoSource = nullptr;
	}

	// Free target calibration objects
	if (m_targetMarkerSampler != nullptr)
	{
		delete m_targetMarkerSampler;
		m_targetMarkerSampler = nullptr;
	}
	if (m_targetDistortionView != nullptr)
	{
		delete m_targetDistortionView;
		m_targetDistortionView = nullptr;
	}

	m_targetCameraComponent = nullptr;

	// Release frame buffer resources
	if (m_frameBuffer)
		m_frameBuffer->disposeResources();

	AppStage::exit();
}

void AppStage_AlignCameraByOriginMarker::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	switch (m_calibrationPanel->getMenuState())
	{
	case eAlignCameraByOriginMarkerMenuState::pendingVideoStart:
		{
			const eVideoStreamingStatus targetStatus = m_targetVideoSource->getVideoStreamingStatus();

			if (targetStatus == eVideoStreamingStatus::failed)
			{
				setMenuState(eAlignCameraByOriginMarkerMenuState::failedVideoStart);
			}
			else if (targetStatus == eVideoStreamingStatus::started)
			{
				setupCalibrator();
				setMenuState(eAlignCameraByOriginMarkerMenuState::verifySetup);
			}
		}
		break;

	case eAlignCameraByOriginMarkerMenuState::verifySetup:
		updateVerifySetup();
		break;

	case eAlignCameraByOriginMarkerMenuState::capturing:
		updateCapturing();
		break;

	case eAlignCameraByOriginMarkerMenuState::testCalibration:
		{
			// Keep video feed updated for preview
			if (m_targetDistortionView)
				m_targetDistortionView->readAndProcessVideoFrame();
		}
		break;

	default:
		break;
	}
}

void AppStage_AlignCameraByOriginMarker::onGui()
{
	AppStage::onGui();

	// Video preview during active states
	const eAlignCameraByOriginMarkerMenuState state = m_calibrationPanel->getMenuState();
	const bool bShowVideo =
		state == eAlignCameraByOriginMarkerMenuState::verifySetup ||
		state == eAlignCameraByOriginMarkerMenuState::capturing;

	if (bShowVideo)
	{
		const ImVec2 displaySize = ImGui::GetMainViewport()->Size;
		constexpr float k_panelWidth = 415.f;
		const float videoAreaWidth = displaySize.x - k_panelWidth;
		constexpr ImGuiWindowFlags k_bgFlags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
		ImGui::SetNextWindowSize(ImVec2(videoAreaWidth, displaySize.y));
		ImGui::SetNextWindowBgAlpha(0.0f);
		if (ImGui::Begin("##VideoTargetBg", nullptr, k_bgFlags))
		{
			IMkTexturePtr tgtTex = m_targetDistortionView
				? m_targetDistortionView->getVideoTexture()
				: nullptr;
			if (tgtTex && tgtTex->getGlTextureId() != 0)
			{
				ImGui::Image(
					(void*)(intptr_t)tgtTex->getGlTextureId(),
					ImVec2(videoAreaWidth, displaySize.y),
					ImVec2(0, 0), ImVec2(1, 1));
			}
		}
		ImGui::End();
	}

	// Side panel with calibration controls
	constexpr float k_panelWidth = 415.f;
	const float displayWidth = m_ownerWindow->getWidth();

	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, 0), ImGuiCond_Always);
	constexpr ImGuiWindowFlags k_flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	MkGuiScopedWindow panel("##AlignCameraByOriginMarker", nullptr, k_flags);
	if (!panel) return;

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}

void AppStage_AlignCameraByOriginMarker::render(IMkViewportPtr targetViewport)
{
	AppStage::render(targetViewport);

	// During testCalibration, render the target camera view with axes at the stage origin
	if (m_calibrationPanel->getMenuState() != eAlignCameraByOriginMarkerMenuState::testCalibration)
		return;
	if (!m_frameBuffer->isValid())
		return;

	{
		MkScopedObjectBinding colorFramebufferBinding(
			m_ownerWindow->getGraphicsContext()->getMkStateStack().getCurrentState(),
			"AlignCameraByOriginMarker Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
			// Render target video as background
			if (m_targetDistortionView)
				m_targetDistortionView->renderSelectedVideoBuffers();

			// Draw axes at the stage origin (identity = where the physical origin marker is)
			IMkGraphicsContext* graphicsContext = getGraphicsContext();
			const glm::mat4 stageOrigin = glm::mat4(1.f);
			drawTransformedAxes(graphicsContext, stageOrigin, 0.1f);

			TextStyle style = getDefaultTextStyle();
			drawTextAtWorldPosition(
				graphicsContext,
				style,
				glm_mat4_get_position(stageOrigin),
				L"Origin Marker");
		}

		// Flush line and text renderers
		m_ownerWindow->getGraphicsContext()->getLineRenderer()->render();
		m_ownerWindow->getGraphicsContext()->getTextRenderer()->render();
	}

	// Blit framebuffer to screen as fullscreen quad
	{
		MkMaterialInstancePtr materialInstance = m_fullscreenQuad->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			auto colorTexture = m_frameBuffer->getColorTexture();
			materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, colorTexture);

			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				m_fullscreenQuad->drawElements();
			}
		}
	}
}

// -- Private helpers --

void AppStage_AlignCameraByOriginMarker::startVideoStream()
{
	const eVideoStreamingStatus targetStatus = m_targetVideoSource->startVideoStream();

	if (targetStatus == eVideoStreamingStatus::failed)
	{
		setMenuState(eAlignCameraByOriginMarkerMenuState::failedVideoStart);
		return;
	}

	if (targetStatus == eVideoStreamingStatus::started)
	{
		setupCalibrator();
		setMenuState(eAlignCameraByOriginMarkerMenuState::verifySetup);
	}
	else
	{
		setMenuState(eAlignCameraByOriginMarkerMenuState::pendingVideoStart);
	}
}

void AppStage_AlignCameraByOriginMarker::setupCalibrator()
{
	// Get the origin marker definition
	MarkerObjectSystemPtr markerSystem = getObjectSystemOfType<MarkerObjectSystem>();
	assert(markerSystem != nullptr);

	MarkerComponentPtr originMarkerComponent = markerSystem->getMarkerById(m_originMarkerId);
	assert(originMarkerComponent != nullptr);

	MarkerDefinitionConstPtr originMarkerDef = originMarkerComponent->getMarkerDefinition();
	assert(originMarkerDef != nullptr);

	// Set up target camera distortion view and marker sampler
	m_targetDistortionView = new VideoFrameDistortionView(m_targetVideoSource, VIDEO_FRAME_HAS_ALL);
	m_targetDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);
	m_targetMarkerSampler = new ArucoMarkerPoseSampler(
		m_targetCameraComponent,
		m_targetDistortionView,
		ALIGN_CAMERA_BY_ORIGIN_MARKER_SAMPLE_COUNT,
		originMarkerDef);

	// Set up framebuffer for testCalibration view
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_targetVideoSource->getCameraIntrinsics(cameraIntrinsics);
	const MikanMonoIntrinsics& monoIntrinsics = cameraIntrinsics.getMonoIntrinsics();
	m_frameBuffer->setName("AlignCameraByOriginMarker");
	m_frameBuffer->setSize(monoIntrinsics.pixel_width, monoIntrinsics.pixel_height);
	m_frameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);
	m_frameBuffer->createResources();
	m_frameBuffer->setClearColor(glm::vec4(Colors::CornflowerBlue, 1.f));
}

void AppStage_AlignCameraByOriginMarker::updateVerifySetup()
{
	if (!m_targetDistortionView)
		return;

	// Process latest video frame
	m_targetDistortionView->readAndProcessVideoFrame();

	// Check if camera sees the origin marker
	const bool targetCanSeeMarker = m_targetMarkerSampler->computeApertureRelativeMarkerXform();
	m_calibrationPanel->setMarkerVisible(targetCanSeeMarker);
}

void AppStage_AlignCameraByOriginMarker::updateCapturing()
{
	if (!m_targetDistortionView)
		return;

	// Process latest video frame
	m_targetDistortionView->readAndProcessVideoFrame();

	// Sample aperture-to-marker transform
	if (m_targetMarkerSampler->computeApertureRelativeMarkerXform())
	{
		if (!m_targetMarkerSampler->hasFinishedSampling())
		{
			m_targetMarkerSampler->sampleLastApertureRelativeMarkerXform();
		}
	}

	// Update progress
	m_calibrationPanel->setCaptureFraction(m_targetMarkerSampler->getCalibrationProgress());

	// Check if sampling is complete
	if (m_targetMarkerSampler->hasFinishedSampling())
	{
		computeAndApplyTargetTransform();
	}
}

void AppStage_AlignCameraByOriginMarker::computeAndApplyTargetTransform()
{
	// 1. Get averaged aperture-to-marker transform
	MikanQuatd markerRot;
	MikanVector3d markerPos;
	if (!m_targetMarkerSampler->computeCalibratedMarkerPose(markerRot, markerPos))
		return;
	const glm::dmat4 avgApertureToMarker =
		glm_mat4_from_pose(
			MikanQuatd_to_glm_dquat(markerRot),
			MikanVector3d_to_glm_dvec3(markerPos));

	// 2. The origin marker IS the stage origin, so marker_StageSpace = identity.
	//    Therefore: cameraAperture_StageSpace = inverse(apertureToMarker) * identity
	//                                         = inverse(apertureToMarker)
	m_cameraApertureXform_StageSpace = glm::inverse(avgApertureToMarker);

	// 3. Apply final pose to the camera component
	m_targetCameraComponent->setRelativeTransform(glm::mat4(m_cameraApertureXform_StageSpace));

	setMenuState(eAlignCameraByOriginMarkerMenuState::testCalibration);
}

void AppStage_AlignCameraByOriginMarker::setMenuState(eAlignCameraByOriginMarkerMenuState newState)
{
	if (m_calibrationPanel)
	{
		m_calibrationPanel->setMenuState(newState);
	}

	if (newState == eAlignCameraByOriginMarkerMenuState::testCalibration)
	{
		if (m_targetCameraComponent)
		{
			MikanCameraPtr mkCamera = getFirstViewport()->getCurrentMikanCamera();
			mkCamera->setCameraTransform(m_targetCameraComponent->getRelativeTransform().getMat4());
		}
	}
}

void AppStage_AlignCameraByOriginMarker::onBeginEvent()
{
	if (m_targetMarkerSampler)
		m_targetMarkerSampler->resetCalibrationState();

	setMenuState(eAlignCameraByOriginMarkerMenuState::capturing);
}

void AppStage_AlignCameraByOriginMarker::onRestartEvent()
{
	if (m_targetMarkerSampler)
		m_targetMarkerSampler->resetCalibrationState();
	m_calibrationPanel->setCaptureFraction(0.f);

	setMenuState(eAlignCameraByOriginMarkerMenuState::verifySetup);
}

void AppStage_AlignCameraByOriginMarker::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_AlignCameraByOriginMarker::onReturnEvent()
{
	m_ownerWindow->popAppState();
}
