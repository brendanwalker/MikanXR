//-- includes -----
#include "AlignCameraByUtilityMarker/AppStage_AlignCameraByUtilityMarker.h"
#include "AlignCameraByUtilityMarker/GuiPanel_AlignCameraByUtilityMarker.h"
#include "ModalMessageBox/ModalDialog_MessageBox.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "ArucoMarkerPoseSampler.h"
#include "CalibrationPatternFinder_Aruco.h"
#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "Colors.h"
#include "IMkFrameBuffer.h"
#include "IMkLineRenderer.h"
#include "IMkTriangulatedMesh.h"
#include "IMkTexture.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MathGLM.h"
#include "MathOpenCV.h"
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
#include "VRTrackingVolumeComponent.h"

#include "glm/gtc/quaternion.hpp"

#include "imgui.h"

//-- internal structs -----
struct TargetMarkerSamples
{
	std::vector<cv::Quatd>  quats;
	std::vector<cv::Vec3d>  positions;

	int size() const { return (int)positions.size(); }

	void clear()
	{
		quats.clear();
		positions.clear();
	}
};

//-- statics ----
const char* AppStage_AlignCameraByUtilityMarker::APP_STAGE_NAME = "AlignCameraByUtilityMarker";

//-- public methods -----
AppStage_AlignCameraByUtilityMarker::AppStage_AlignCameraByUtilityMarker(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_AlignCameraByUtilityMarker::APP_STAGE_NAME)
	, m_targetSamples(new TargetMarkerSamples())
	, m_frameBuffer(createMkFrameBuffer())
	, m_fullscreenQuad(createFullscreenQuadMesh(ownerWindow->getGraphicsContext().get(), false))
{
}

AppStage_AlignCameraByUtilityMarker::~AppStage_AlignCameraByUtilityMarker()
{
	delete m_targetSamples;
}

bool AppStage_AlignCameraByUtilityMarker::tryEnterCalibration(
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

	// 2. Target camera must NOT have a tracking mount (this method is for untracked cameras)
	if (targetCameraComponent->hasValidTrackingMountComponent())
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera already has a tracking mount. Use the standard alignment calibration instead.");
		return false;
	}

	// 3. Owner stage must have a VR tracking volume
	VRTrackingVolumeComponentConstPtr trackingVolume =
		targetCameraComponent->getVRTrackingVolumeComponent();
	if (!trackingVolume)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"Target camera's stage has no VR tracking volume. Please assign a VR tracking volume to the stage.");
		return false;
	}

	// 4. VR tracking volume must have a valid utility marker assigned
	const MikanMarkerID utilityMarkerId =
		trackingVolume->getVRTrackingVolumeDefinition()->getUtilityMarkerId();
	if (utilityMarkerId == INVALID_MIKAN_ID)
	{
		ModalDialog_MessageBox::showMessageBox(
			fromAppStage,
			"The stage's VR tracking volume has no Utility Marker assigned. Please assign a utility marker to the tracking volume.");
		return false;
	}

	auto* appStage = ownerWindow->pushAppStageOfType<AppStage_AlignCameraByUtilityMarker>();
	appStage->setTargetCameraComponent(targetCameraComponent);

	return true;
}

void AppStage_AlignCameraByUtilityMarker::setTargetCameraComponent(CameraComponentPtr cameraComponent)
{
	m_targetCameraComponent = cameraComponent;
}

// -- AppStage -- //
void AppStage_AlignCameraByUtilityMarker::enter()
{
	AppStage::enter();
	assert(m_targetCameraComponent != nullptr);

	// Fetch the utility marker ID from the tracking volume
	VRTrackingVolumeComponentConstPtr trackingVolume =
		m_targetCameraComponent->getVRTrackingVolumeComponent();
	assert(trackingVolume != nullptr);
	m_utilityMarkerId = trackingVolume->getVRTrackingVolumeDefinition()->getUtilityMarkerId();

	// Cache the target video source
	m_targetVideoSource = m_targetCameraComponent->getVideoSourceComponent();
	assert(m_targetVideoSource != nullptr);

	// Create GUI panel
	m_calibrationPanel = addGuiPanel<GuiPanel_AlignCameraByUtilityMarker>();
	m_calibrationPanel->OnBeginEvent   = [this]() { onBeginEvent(); };
	m_calibrationPanel->OnRestartEvent = [this]() { onRestartEvent(); };
	m_calibrationPanel->OnCancelEvent  = [this]() { onCancelEvent(); };
	m_calibrationPanel->OnReturnEvent  = [this]() { onReturnEvent(); };

	// Open the source camera selection dialog first
	openSourceCameraDialog();
}

void AppStage_AlignCameraByUtilityMarker::exit()
{
	setMenuState(eAlignCameraByUtilityMarkerMenuState::inactive);

	// Stop and release target video
	if (m_targetVideoSource)
	{
		m_targetVideoSource->stopVideoStream();
		m_targetVideoSource = nullptr;
	}

	// Stop and release source video
	if (m_sourceVideoSource)
	{
		m_sourceVideoSource->stopVideoStream();
		m_sourceVideoSource = nullptr;
	}

	// Free source calibration objects
	if (m_sourceMarkerSampler != nullptr)
	{
		delete m_sourceMarkerSampler;
		m_sourceMarkerSampler = nullptr;
	}
	if (m_sourceDistortionView != nullptr)
	{
		delete m_sourceDistortionView;
		m_sourceDistortionView = nullptr;
	}

	// Free target calibration objects
	if (m_targetMarkerFinder != nullptr)
	{
		delete m_targetMarkerFinder;
		m_targetMarkerFinder = nullptr;
	}
	if (m_targetDistortionView != nullptr)
	{
		delete m_targetDistortionView;
		m_targetDistortionView = nullptr;
	}

	m_sourceCameraComponent = nullptr;
	m_targetCameraComponent = nullptr;

	// Release frame buffer resources
	if (m_frameBuffer)
		m_frameBuffer->disposeResources();

	AppStage::exit();
}

void AppStage_AlignCameraByUtilityMarker::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	switch (m_calibrationPanel->getMenuState())
	{
	case eAlignCameraByUtilityMarkerMenuState::pendingVideoStart:
		{
			const eVideoStreamingStatus sourceStatus =
				m_sourceVideoSource ? m_sourceVideoSource->getVideoStreamingStatus()
									: eVideoStreamingStatus::failed;
			const eVideoStreamingStatus targetStatus = m_targetVideoSource->getVideoStreamingStatus();

			if (sourceStatus == eVideoStreamingStatus::failed ||
				targetStatus == eVideoStreamingStatus::failed)
			{
				setMenuState(eAlignCameraByUtilityMarkerMenuState::failedVideoStart);
			}
			else if (sourceStatus == eVideoStreamingStatus::started &&
					 targetStatus == eVideoStreamingStatus::started)
			{
				setupCalibrators();
				setMenuState(eAlignCameraByUtilityMarkerMenuState::verifySetup);
			}
		}
		break;

	case eAlignCameraByUtilityMarkerMenuState::verifySetup:
		updateVerifySetup();
		break;

	case eAlignCameraByUtilityMarkerMenuState::capturing:
		updateCapturing();
		break;

	case eAlignCameraByUtilityMarkerMenuState::testCalibration:
		{
			// Keep video feeds updated for preview
			if (m_sourceDistortionView)
				m_sourceDistortionView->readAndProcessVideoFrame();
			if (m_targetDistortionView)
				m_targetDistortionView->readAndProcessVideoFrame();
		}
		break;

	default:
		break;
	}
}

void AppStage_AlignCameraByUtilityMarker::onGui()
{
	AppStage::onGui();

	// Side-by-side video preview during active states
	const eAlignCameraByUtilityMarkerMenuState state = m_calibrationPanel->getMenuState();
	const bool bShowVideo =
		state == eAlignCameraByUtilityMarkerMenuState::verifySetup ||
		state == eAlignCameraByUtilityMarkerMenuState::capturing ||
		state == eAlignCameraByUtilityMarkerMenuState::testCalibration;

	if (bShowVideo)
	{
		const ImVec2 displaySize = ImGui::GetMainViewport()->Size;
		constexpr float k_panelWidth = 415.f;
		const float videoAreaWidth = displaySize.x - k_panelWidth;
		const float halfW = videoAreaWidth * 0.5f;
		const float h = displaySize.y;

		ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
		ImGui::SetNextWindowSize(ImVec2(videoAreaWidth, h));
		ImGui::SetNextWindowBgAlpha(0.0f);
		constexpr ImGuiWindowFlags k_bgFlags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;
		if (ImGui::Begin("##VideoBg", nullptr, k_bgFlags))
		{
			// Source video (left)
			IMkTexturePtr srcTex = m_sourceDistortionView
				? m_sourceDistortionView->getVideoTexture()
				: nullptr;
			if (srcTex && srcTex->getGlTextureId() != 0)
			{
				ImGui::Image(
					(void*)(intptr_t)srcTex->getGlTextureId(),
					ImVec2(halfW, h),
					ImVec2(0, 1), ImVec2(1, 0));
			}
			else
			{
				ImGui::Dummy(ImVec2(halfW, h));
			}

			ImGui::SameLine(0, 0);

			// Target video (right)
			IMkTexturePtr tgtTex = m_targetDistortionView
				? m_targetDistortionView->getVideoTexture()
				: nullptr;
			if (tgtTex && tgtTex->getGlTextureId() != 0)
			{
				ImGui::Image(
					(void*)(intptr_t)tgtTex->getGlTextureId(),
					ImVec2(halfW, h),
					ImVec2(0, 1), ImVec2(1, 0));
			}
			else
			{
				ImGui::Dummy(ImVec2(halfW, h));
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
	MkGuiScopedWindow panel("##AlignCameraByUtilityMarker", nullptr, k_flags);
	if (!panel) return;

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}

void AppStage_AlignCameraByUtilityMarker::render(IMkViewportPtr targetViewport)
{
	AppStage::render(targetViewport);

	// During testCalibration, render the target camera view with axes at the marker
	if (m_calibrationPanel->getMenuState() != eAlignCameraByUtilityMarkerMenuState::testCalibration)
		return;
	if (!m_frameBuffer->isValid())
		return;

	{
		MkScopedObjectBinding colorFramebufferBinding(
			m_ownerWindow->getGraphicsContext()->getMkStateStack().getCurrentState(),
			"AlignCameraByUtilityMarker Color Framebuffer Scope",
			m_frameBuffer);

		if (colorFramebufferBinding)
		{
			// Render target video as background
			if (m_targetDistortionView)
				m_targetDistortionView->renderSelectedVideoBuffers();

			// Draw axes at the utility marker's computed stage-space position
			IMkGraphicsContext* graphicsContext = getGraphicsContext();
			const glm::mat4 markerXform = glm::mat4(m_markerXform_VRSpace);
			drawTransformedAxes(graphicsContext, markerXform, 0.1f);

			TextStyle style = getDefaultTextStyle();
			drawTextAtWorldPosition(
				graphicsContext,
				style,
				glm_mat4_get_position(markerXform),
				L"Utility Marker");
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

void AppStage_AlignCameraByUtilityMarker::openSourceCameraDialog()
{
	setMenuState(eAlignCameraByUtilityMarkerMenuState::selectSourceCamera);

	ModalDialog_SelectCamera::selectCamera(
		this,
		[this](MikanCameraID cameraId) { onSourceCameraSelected(cameraId); },
		[this]() { onCancelEvent(); },
		[](CameraComponentPtr c) -> bool
		{
			return c->hasValidTrackingMountComponent() && c->hasValidApertureOffsetXform();
		});
}

void AppStage_AlignCameraByUtilityMarker::onSourceCameraSelected(MikanCameraID cameraId)
{
	CameraObjectSystemPtr cameraSystem = getObjectSystemOfType<CameraObjectSystem>();
	assert(cameraSystem != nullptr);

	m_sourceCameraComponent = cameraSystem->getCameraById(cameraId);
	if (!m_sourceCameraComponent)
	{
		ModalDialog_MessageBox::showMessageBox(
			this,
			"Failed to find selected source camera. Please try again.");
		openSourceCameraDialog();
		return;
	}

	VideoSourceComponentPtr sourceVideoSource = m_sourceCameraComponent->getVideoSourceComponent();
	if (!sourceVideoSource || !sourceVideoSource->areCameraIntrinsicsValid())
	{
		ModalDialog_MessageBox::showMessageBox(
			this,
			"Selected source camera has no valid video source or intrinsics.");
		m_sourceCameraComponent = nullptr;
		openSourceCameraDialog();
		return;
	}

	m_sourceVideoSource = sourceVideoSource;
	startVideoStreams();
}

void AppStage_AlignCameraByUtilityMarker::startVideoStreams()
{
	const eVideoStreamingStatus sourceStatus = m_sourceVideoSource->startVideoStream();
	const eVideoStreamingStatus targetStatus = m_targetVideoSource->startVideoStream();

	if (sourceStatus == eVideoStreamingStatus::failed ||
		targetStatus == eVideoStreamingStatus::failed)
	{
		setMenuState(eAlignCameraByUtilityMarkerMenuState::failedVideoStart);
		return;
	}

	if (sourceStatus == eVideoStreamingStatus::started &&
		targetStatus == eVideoStreamingStatus::started)
	{
		setupCalibrators();
		setMenuState(eAlignCameraByUtilityMarkerMenuState::verifySetup);
	}
	else
	{
		setMenuState(eAlignCameraByUtilityMarkerMenuState::pendingVideoStart);
	}
}

void AppStage_AlignCameraByUtilityMarker::setupCalibrators()
{
	// Get the utility marker definition
	MarkerObjectSystemPtr markerSystem = getObjectSystemOfType<MarkerObjectSystem>();
	assert(markerSystem != nullptr);

	MarkerComponentPtr utilityMarkerComponent = markerSystem->getMarkerById(m_utilityMarkerId);
	assert(utilityMarkerComponent != nullptr);

	MarkerDefinitionConstPtr utilityMarkerDef = utilityMarkerComponent->getMarkerDefinition();
	assert(utilityMarkerDef != nullptr);

	// Set up source camera (has tracking mount - ArucoMarkerPoseSampler handles stage-space composition)
	// Use explicit utilityMarkerDef so we track the utility marker, not the stage origin marker
	m_sourceDistortionView = new VideoFrameDistortionView(m_sourceVideoSource, VIDEO_FRAME_HAS_ALL);
	m_sourceDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);
	m_sourceMarkerSampler = new ArucoMarkerPoseSampler(
		m_sourceCameraComponent,
		m_sourceDistortionView,
		ALIGN_CAMERA_BY_UTILITY_MARKER_SAMPLE_COUNT,
		utilityMarkerDef);

	// Set up target camera (no tracking mount - use CalibrationPatternFinder_Aruco directly)
	m_targetDistortionView = new VideoFrameDistortionView(m_targetVideoSource, VIDEO_FRAME_HAS_ALL);
	m_targetDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);
	m_targetMarkerFinder = new CalibrationPatternFinder_Aruco(
		m_targetCameraComponent,
		m_targetDistortionView,
		utilityMarkerDef);

	// Set up framebuffer for testCalibration view
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_targetVideoSource->getCameraIntrinsics(cameraIntrinsics);
	const MikanMonoIntrinsics& monoIntrinsics = cameraIntrinsics.getMonoIntrinsics();
	m_frameBuffer->setName("AlignCameraByUtilityMarker");
	m_frameBuffer->setSize(monoIntrinsics.pixel_width, monoIntrinsics.pixel_height);
	m_frameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR);
	m_frameBuffer->createResources();
	m_frameBuffer->setClearColor(glm::vec4(Colors::CornflowerBlue, 1.f));

	// Clear sample accumulators
	m_targetSamples->clear();
}

void AppStage_AlignCameraByUtilityMarker::updateVerifySetup()
{
	if (!m_sourceDistortionView || !m_targetDistortionView)
		return;

	// Process latest video frames
	m_sourceDistortionView->readAndProcessVideoFrame();
	m_targetDistortionView->readAndProcessVideoFrame();

	// Check if source camera sees the utility marker
	const bool sourceCanSeeMarker = m_sourceMarkerSampler->computeVRSpaceMarkerXform();
	m_calibrationPanel->setSourceMarkerVisible(sourceCanSeeMarker);

	// Check if target camera sees the utility marker
	glm::dmat4 dummy;
	const bool targetCanSeeMarker = m_targetMarkerFinder->estimateNewCalibrationPatternPose(dummy);
	m_calibrationPanel->setTargetMarkerVisible(targetCanSeeMarker);
}

void AppStage_AlignCameraByUtilityMarker::updateCapturing()
{
	if (!m_sourceDistortionView || !m_targetDistortionView)
		return;

	// Process latest video frames
	m_sourceDistortionView->readAndProcessVideoFrame();
	m_targetDistortionView->readAndProcessVideoFrame();

	// Sample from source camera (computes marker_StageSpace)
	if (m_sourceMarkerSampler->computeVRSpaceMarkerXform())
	{
		if (!m_sourceMarkerSampler->hasFinishedSampling())
			m_sourceMarkerSampler->sampleLastVRSpaceMarkerXform();
	}

	// Sample from target camera (records raw camera-space apertureToMarker transform)
	if (m_targetSamples->size() < ALIGN_CAMERA_BY_UTILITY_MARKER_SAMPLE_COUNT)
	{
		glm::dmat4 apertureToMarkerXform;
		if (m_targetMarkerFinder->estimateNewCalibrationPatternPose(apertureToMarkerXform))
		{
			const glm::dvec3 t = glm::dvec3(apertureToMarkerXform[3]);
			const glm::dquat q = glm::quat_cast(apertureToMarkerXform);
			m_targetSamples->quats.push_back(glm_dquat_to_cv_quatd(q));
			m_targetSamples->positions.push_back(glm_dvec3_to_cv_vec3d(t));
		}
	}

	// Update progress
	const float sourceFraction = m_sourceMarkerSampler->getCalibrationProgress();
	const float targetFraction =
		(float)m_targetSamples->size() / (float)ALIGN_CAMERA_BY_UTILITY_MARKER_SAMPLE_COUNT;
	m_calibrationPanel->setSourceCaptureFraction(sourceFraction);
	m_calibrationPanel->setTargetCaptureFraction(targetFraction);

	// Check if both have finished sampling
	if (m_sourceMarkerSampler->hasFinishedSampling() &&
		m_targetSamples->size() >= ALIGN_CAMERA_BY_UTILITY_MARKER_SAMPLE_COUNT)
	{
		computeAndApplyTargetTransform();
	}
}

void AppStage_AlignCameraByUtilityMarker::computeAndApplyTargetTransform()
{
	// 1. Get averaged marker stage-space pose from source sampler
	MikanQuatd srcRot;
	MikanVector3d srcTrans;
	if (!m_sourceMarkerSampler->computeCalibratedMarkerPose(srcRot, srcTrans))
		return;

	const glm::dquat markerRot = MikanQuatd_to_glm_dquat(srcRot);
	const glm::dvec3 markerPos = MikanVector3d_to_glm_dvec3(srcTrans);
	m_markerXform_VRSpace = glm_mat4_from_pose(markerRot, markerPos);

	// 2. Average target camera-space samples
	cv::Quatd cv_avgTargetQuat;
	cv::Vec3d cv_avgTargetTrans;
	if (!opencv_quaternion_compute_average(m_targetSamples->quats, cv_avgTargetQuat) ||
		!opencv_vec3d_compute_average(m_targetSamples->positions, cv_avgTargetTrans))
		return;

	const glm::dquat avgTargetQuat = cv_quatd_to_glm_dquat(cv_avgTargetQuat);
	const glm::dvec3 avgTargetTrans = cv_vec3d_to_glm_dvec3(cv_avgTargetTrans);
	const glm::dmat4 apertureToMarker = glm_mat4_from_pose(avgTargetQuat, avgTargetTrans);

	// 3. Convert the target aperture transform in VRSpace
	const glm::dmat4 markerToAperture = glm::inverse(apertureToMarker);
	const glm::dmat4 targetApertureXform_VRSpace = glm_composite_xform(markerToAperture, m_markerXform_VRSpace);

	// 4. Convert the target aperture transform from VRSpace to StageSpace
	const auto stageComponent = m_targetCameraComponent->getOwnerStageComponent();
	const auto trackingVolumeDefinition =
		std::static_pointer_cast<const VRTrackingVolumeComponent>(
			stageComponent->getTrackingVolumeConst());
	glm::mat4 glmVRDevicePoseOffset = trackingVolumeDefinition->getVRDevicePoseOffset();

	m_targetApertureXform_StageSpace = 
		glm_composite_xform(targetApertureXform_VRSpace, glmVRDevicePoseOffset);

	// 4. Update the target camera's stage-space transform to match the computed aperture pose
	m_targetCameraComponent->setRelativeTransform(glm::mat4(m_targetApertureXform_StageSpace));

	setMenuState(eAlignCameraByUtilityMarkerMenuState::testCalibration);
}

void AppStage_AlignCameraByUtilityMarker::setMenuState(eAlignCameraByUtilityMarkerMenuState newState)
{
	if (m_calibrationPanel)
		m_calibrationPanel->setMenuState(newState);
}

void AppStage_AlignCameraByUtilityMarker::onBeginEvent()
{
	// Reset sample accumulators
	m_targetSamples->clear();
	if (m_sourceMarkerSampler)
		m_sourceMarkerSampler->resetCalibrationState();

	setMenuState(eAlignCameraByUtilityMarkerMenuState::capturing);
}

void AppStage_AlignCameraByUtilityMarker::onRestartEvent()
{
	// Reset and go back to verifySetup
	m_targetSamples->clear();
	if (m_sourceMarkerSampler)
		m_sourceMarkerSampler->resetCalibrationState();
	m_calibrationPanel->setSourceCaptureFraction(0.f);
	m_calibrationPanel->setTargetCaptureFraction(0.f);

	setMenuState(eAlignCameraByUtilityMarkerMenuState::verifySetup);
}

void AppStage_AlignCameraByUtilityMarker::onCancelEvent()
{
	m_ownerWindow->popAppState();
}

void AppStage_AlignCameraByUtilityMarker::onReturnEvent()
{
	m_ownerWindow->popAppState();
}
