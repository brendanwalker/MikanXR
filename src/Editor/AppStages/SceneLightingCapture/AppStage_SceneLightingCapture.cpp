#include "SceneLightingCapture/AppStage_SceneLightingCapture.h"
#include "SceneLightingCapture/GuiPanel_SceneLightingCapture.h"

#include "imgui.h"
#include "MkGuiScopedWindow.h"

#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "Colors.h"
#include "IEditorWindow.h"
#include "IMkGraphicsContext.h"
#include "IMkLineRenderer.h"
#include "IMkViewport.h"
#include "LightEnvironmentComponent.h"
#include "Logger.h"
#include "MikanCamera.h"
#include "MikanViewport.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include "TransformComponent.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"

#include <opencv2/opencv.hpp>

#include <filesystem>

const char* AppStage_SceneLightingCapture::APP_STAGE_NAME= "SceneLightingCapture";

// Model directory relative to the working directory, matching the convention
// used elsewhere in the Mikan family (see docs/reference/scene-lighting.md).
static const char* k_defaultModelSubdirectory= "models/marigold";

AppStage_SceneLightingCapture::AppStage_SceneLightingCapture(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, APP_STAGE_NAME)
{
}

AppStage_SceneLightingCapture::~AppStage_SceneLightingCapture() {}

void AppStage_SceneLightingCapture::setTargetProbe(LightEnvironmentComponentPtr targetProbe)
{
	m_targetProbe= targetProbe;
}

void AppStage_SceneLightingCapture::setSourceCamera(CameraComponentPtr cameraComponent)
{
	m_currentSceneCameraComponent= cameraComponent;
	m_videoSourceComponent= cameraComponent ? cameraComponent->getVideoSourceComponent() : nullptr;
}

void AppStage_SceneLightingCapture::enter()
{
	AppStage::enter();

	m_mkCamera= getFirstViewport()->getCurrentMikanCamera();
	m_mkCamera->setCameraMovementMode(eCameraMovementMode::stationary);

	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_currentSceneCameraComponent->getApertureIntrinsics(cameraIntrinsics);
	m_mkCamera->applyMonoCameraIntrinsics(&cameraIntrinsics);

	// The distortion view is the stream ownership token, same as the other
	// calibration stages.
	m_monoDistortionView= new VideoFrameDistortionView(m_videoSourceComponent, eVideoFrameProcessorMode::CALIBRATION);
	m_monoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);
	// The fit consumes the undistorted color buffer, so color undistortion must
	// be on regardless of what the rest of the app left it set to.
	m_monoDistortionView->setColorUndistortDisabled(false);

	m_videoSourceComponent->startVideoStream(m_monoDistortionView);

	m_capturePanel= addGuiPanel<GuiPanel_SceneLightingCapture>();
	m_capturePanel->setProbeName(m_targetProbe ? m_targetProbe->getName() : std::string("<none>"));
	m_capturePanel->OnCaptureEvent= [this]() { onCaptureEvent(); };
	m_capturePanel->OnApplyEvent= [this]() { onApplyEvent(); };
	m_capturePanel->OnRedoEvent= [this]() { onRedoEvent(); };
	m_capturePanel->OnCancelEvent= [this]() { onCancelEvent(); };
	m_capturePanel->OnOkEvent= [this]() { onCancelEvent(); };

	m_bHasResult= false;

	setMenuState(eSceneLightingCaptureMenuState::pendingVideoStartStreamRequest);
}

void AppStage_SceneLightingCapture::exit()
{
	setMenuState(eSceneLightingCaptureMenuState::inactive);

	// Free the ONNX sessions before anything else; they hold gigabytes.
	m_estimator= nullptr;

	m_currentSceneCameraComponent= nullptr;
	m_mkCamera= nullptr;
	m_targetProbe= nullptr;

	if (m_monoDistortionView != nullptr)
	{
		if (m_videoSourceComponent)
			m_videoSourceComponent->stopVideoStream(m_monoDistortionView);
		delete m_monoDistortionView;
		m_monoDistortionView= nullptr;
	}

	m_videoSourceComponent= nullptr;

	AppStage::exit();
}

void AppStage_SceneLightingCapture::setMenuState(eSceneLightingCaptureMenuState newState)
{
	if (m_capturePanel != nullptr)
		m_capturePanel->setMenuState(newState);
}

void AppStage_SceneLightingCapture::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	const eSceneLightingCaptureMenuState menuState= m_capturePanel->getMenuState();

	if (menuState == eSceneLightingCaptureMenuState::pendingVideoStartStreamRequest)
	{
		if (m_monoDistortionView->isReceivingFrames())
		{
			setMenuState(eSceneLightingCaptureMenuState::verifyCameraSetup);
		}
		else if (m_videoSourceComponent->getVideoStreamingStatus() == eVideoStreamingStatus::failed)
		{
			setMenuState(eSceneLightingCaptureMenuState::failedVideoStartStreamRequest);
		}
		return;
	}

	// Keep the undistorted color buffer current while the operator is framing
	// the shot; runCapture() reads whatever the last processed frame produced.
	if (menuState == eSceneLightingCaptureMenuState::verifyCameraSetup)
	{
		m_monoDistortionView->readAndProcessVideoFrame();
	}
}

void AppStage_SceneLightingCapture::onGui()
{
	AppStage::onGui();

	constexpr float k_panelWidth= 415.f;
	const float displayWidth= m_ownerWindow->getWidth();
	const float displayHeight= m_ownerWindow->getHeight();

	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);
	constexpr ImGuiWindowFlags k_flags=
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	MkGuiScopedWindow panel("##SceneLightingCapture", nullptr, k_flags);
	if (!panel)
		return;

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}

void AppStage_SceneLightingCapture::onCaptureEvent()
{
	// Show the "running" state before the blocking inference call so the panel
	// does not sit on "Capture" for the several seconds this takes.
	setMenuState(eSceneLightingCaptureMenuState::runningInference);

	runCapture();
}

void AppStage_SceneLightingCapture::runCapture()
{
	if (m_estimator == nullptr)
	{
		const std::filesystem::path modelDirectory= std::filesystem::current_path() / k_defaultModelSubdirectory;

		SceneLightingEstimator::Config config;
		config.modelDirectory= modelDirectory.string();

		auto estimator= std::make_unique<SceneLightingEstimator>();
		if (!estimator->startup(config))
		{
			m_capturePanel->setFailureReason(StringUtils::stringify(
				"Could not load the Marigold models from '", modelDirectory.string(),
				"'. Run tools/export_marigold_onnx.py to produce them. See MikanCmd.log for details."));
			setMenuState(eSceneLightingCaptureMenuState::failedInference);
			return;
		}

		m_estimator= std::move(estimator);
		m_capturePanel->setExecutionProvider(m_estimator->getActiveExecutionProvider());
	}

	// Make sure the buffer reflects the newest frame rather than whatever was
	// last processed.
	m_monoDistortionView->readAndProcessVideoFrame();

	cv::Mat* bgrBuffer= m_monoDistortionView->getBGRUndistortBuffer();
	if (bgrBuffer == nullptr || bgrBuffer->empty())
	{
		m_capturePanel->setFailureReason("No undistorted color frame was available from the video source.");
		setMenuState(eSceneLightingCaptureMenuState::failedInference);
		return;
	}

	// Marigold returns camera-space normals, so the fit needs the camera's
	// orientation to place the recovered environment in world space.
	glm::mat4 cameraPose(1.f);
	if (!m_currentSceneCameraComponent->getStageSpaceAperturePose(cameraPose))
	{
		m_capturePanel->setFailureReason("Could not resolve the camera pose. Is the camera tracked?");
		setMenuState(eSceneLightingCaptureMenuState::failedInference);
		return;
	}
	const glm::mat3 cameraToWorldRotation(cameraPose);

	if (!m_estimator->estimate(*bgrBuffer, cameraToWorldRotation, m_result))
	{
		m_capturePanel->setFailureReason("Lighting estimation failed. See MikanCmd.log for details.");
		setMenuState(eSceneLightingCaptureMenuState::failedInference);
		return;
	}

	m_bHasResult= true;

	const glm::vec3 ambient= m_result.environment.coefficients[0] * 0.282095f * 3.14159265f;
	m_capturePanel->setEstimateSummary(m_result.directionality,
									   StringUtils::stringify(m_result.keyLightDirection.x, ", ",
															  m_result.keyLightDirection.y, ", ",
															  m_result.keyLightDirection.z),
									   StringUtils::stringify(ambient.r, ", ", ambient.g, ", ", ambient.b),
									   m_result.sampleCount, m_result.negativeSolidAngleFraction);

	setMenuState(eSceneLightingCaptureMenuState::verifyEstimate);
}

void AppStage_SceneLightingCapture::applyEstimate()
{
	if (!m_bHasResult || !m_targetProbe)
		return;

	LightEnvironmentDefinitionPtr definition= m_targetProbe->getLightEnvironmentDefinition();
	if (!definition)
		return;

	definition->setLightingEnvironment(m_result.environment);

	MIKAN_LOG_INFO("AppStage_SceneLightingCapture")
		<< "Applied estimated lighting to probe '" << m_targetProbe->getName() << "' (directionality "
		<< m_result.directionality << ")";
}

void AppStage_SceneLightingCapture::onApplyEvent()
{
	applyEstimate();
	setMenuState(eSceneLightingCaptureMenuState::captureComplete);
}

void AppStage_SceneLightingCapture::onRedoEvent()
{
	m_bHasResult= false;
	setMenuState(eSceneLightingCaptureMenuState::verifyCameraSetup);
}

void AppStage_SceneLightingCapture::onCancelEvent() { getOwnerWindow()->popAppState(); }

void AppStage_SceneLightingCapture::renderLitSpherePreview()
{
	if (!m_bHasResult)
		return;

	IMkGraphicsContext* graphicsContext= getGraphicsContext();
	const float frameWidth= (float)m_monoDistortionView->getFrameWidth();
	const float frameHeight= (float)m_monoDistortionView->getFrameHeight();

	// A disc of points in the corner of the video frame, each colored by the
	// irradiance the recovered environment delivers to that surface normal.
	// This is the same construction as the lit sphere in
	// tools/sh_lighting_fit.py, so the two can be compared directly.
	const float radius= fminf(frameWidth, frameHeight) * 0.18f;
	const glm::vec2 center(frameWidth - radius - 16.f, radius + 16.f);
	const int steps= 48;

	// Normalize against the brightest sample so the preview is readable
	// regardless of the scene's absolute exposure, which is only recovered up
	// to a global scale anyway.
	float peak= 1e-6f;
	for (int y= -steps; y <= steps; ++y)
	{
		for (int x= -steps; x <= steps; ++x)
		{
			const float nx= (float)x / (float)steps;
			const float ny= (float)y / (float)steps;
			const float r2= nx * nx + ny * ny;
			if (r2 > 1.f)
				continue;

			const glm::vec3 normal(nx, ny, sqrtf(fmaxf(0.f, 1.f - r2)));
			const glm::vec3 irradiance= m_result.cameraSpaceEnvironment.evalIrradiance(normal);
			peak= fmaxf(peak, fmaxf(irradiance.r, fmaxf(irradiance.g, irradiance.b)));
		}
	}

	// Resolve the line renderer and viewport bounds ONCE. drawPointList2d does
	// this internally per call, so calling it per point would repeat the
	// viewport lookup several thousand times for what the renderer batches
	// anyway. The bounds convention here matches that helper exactly.
	IMkLineRenderer* lineRenderer= graphicsContext->getLineRenderer();
	IMkViewportPtr renderingViewport= graphicsContext->getRenderingViewport();
	glm::i32vec2 viewportOrigin, viewportSize;
	if (lineRenderer == nullptr || renderingViewport == nullptr
		|| !renderingViewport->getRenderingViewport(viewportOrigin, viewportSize))
	{
		return;
	}

	const float viewportX0= (float)viewportOrigin.x;
	const float viewportY0= (float)viewportOrigin.y;
	const float viewportX1= (float)viewportOrigin.x + (float)viewportSize.x - 1.f;
	const float viewportY1= (float)viewportOrigin.y + (float)viewportSize.y - 1.f;

	for (int y= -steps; y <= steps; ++y)
	{
		for (int x= -steps; x <= steps; ++x)
		{
			const float nx= (float)x / (float)steps;
			const float ny= (float)y / (float)steps;
			const float r2= nx * nx + ny * ny;
			if (r2 > 1.f)
				continue;

			const glm::vec3 normal(nx, ny, sqrtf(fmaxf(0.f, 1.f - r2)));

			// Preview in CAMERA space: it is being compared against the video
			// frame, which is also camera space.
			glm::vec3 color= m_result.cameraSpaceEnvironment.evalIrradiance(normal) / peak;
			// Order-2 SH can evaluate negative; clamp exactly as the renderer does.
			color= glm::vec3(fmaxf(color.r, 0.f), fmaxf(color.g, 0.f), fmaxf(color.b, 0.f));

			const glm::vec2 framePoint(center.x + nx * radius, center.y - ny * radius);
			const glm::vec2 windowPoint= remapPointIntoTarget(frameWidth, frameHeight, viewportX0, viewportY0,
															  viewportX1, viewportY1, framePoint);

			lineRenderer->addPoint2d(windowPoint, color, 3.f);
		}
	}
}

void AppStage_SceneLightingCapture::render(IMkViewportPtr targetViewport)
{
	switch (m_capturePanel->getMenuState())
	{
	case eSceneLightingCaptureMenuState::verifyCameraSetup:
	case eSceneLightingCaptureMenuState::runningInference:
		m_monoDistortionView->renderSelectedVideoBuffers();
		break;

	case eSceneLightingCaptureMenuState::verifyEstimate:
	case eSceneLightingCaptureMenuState::captureComplete:
		m_monoDistortionView->renderSelectedVideoBuffers();
		renderLitSpherePreview();
		break;

	default:
		break;
	}

	getGraphicsContext()->getLineRenderer()->render(true);
}
