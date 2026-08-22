#include "DepthMeshCapture/AppStage_DepthMeshCapture.h"
#include "DepthMeshCapture/GuiPanel_DepthMeshCapture.h"

#include "imgui.h"
#include "MkGuiScopedWindow.h"

#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "IEditorWindow.h"
#include "IMkGraphicsContext.h"
#include "IMkLineRenderer.h"
#include "IMkViewport.h"
#include "Logger.h"
#include "MikanCamera.h"
#include "MikanViewport.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include "Transform.h"
#include "TransformComponent.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"

#include <opencv2/opencv.hpp>

#include <ctime>
#include <filesystem>

const char* AppStage_DepthMeshCapture::APP_STAGE_NAME= "DepthMeshCapture";

// Model directory relative to the working directory, same convention as the
// Marigold models (see docs/reference/scene-lighting.md). Uniquely named
// because the unity build merges this file with the other stages' .cpps.
static const char* k_depthMeshMoGeModelSubdirectory= "models/moge2";

AppStage_DepthMeshCapture::AppStage_DepthMeshCapture(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, APP_STAGE_NAME)
{
}

AppStage_DepthMeshCapture::~AppStage_DepthMeshCapture() {}

void AppStage_DepthMeshCapture::setSourceCamera(CameraComponentPtr cameraComponent)
{
	m_currentSceneCameraComponent= cameraComponent;
	m_videoSourceComponent= cameraComponent ? cameraComponent->getVideoSourceComponent() : nullptr;
}

void AppStage_DepthMeshCapture::enter()
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
	// The model consumes the undistorted color buffer - the calibrated FOV it
	// is given describes the undistorted image, not the raw one.
	m_monoDistortionView->setColorUndistortDisabled(false);

	m_videoSourceComponent->startVideoStream(m_monoDistortionView);

	m_capturePanel= addGuiPanel<GuiPanel_DepthMeshCapture>();
	m_capturePanel->OnCaptureEvent= [this]() { onCaptureEvent(); };
	m_capturePanel->OnApplyEvent= [this]() { onApplyEvent(); };
	m_capturePanel->OnRedoEvent= [this]() { onRedoEvent(); };
	m_capturePanel->OnCancelEvent= [this]() { onCancelEvent(); };
	m_capturePanel->OnOkEvent= [this]() { onCancelEvent(); };

	m_bHasResult= false;

	setMenuState(eDepthMeshCaptureMenuState::pendingVideoStartStreamRequest);
}

void AppStage_DepthMeshCapture::exit()
{
	setMenuState(eDepthMeshCaptureMenuState::inactive);

	// Free the ONNX session before anything else; it holds over a gigabyte.
	m_inference= nullptr;

	m_currentSceneCameraComponent= nullptr;
	m_mkCamera= nullptr;

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

void AppStage_DepthMeshCapture::setMenuState(eDepthMeshCaptureMenuState newState)
{
	if (m_capturePanel != nullptr)
		m_capturePanel->setMenuState(newState);
}

void AppStage_DepthMeshCapture::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	const eDepthMeshCaptureMenuState menuState= m_capturePanel->getMenuState();

	if (menuState == eDepthMeshCaptureMenuState::pendingVideoStartStreamRequest)
	{
		if (m_monoDistortionView->isReceivingFrames())
		{
			setMenuState(eDepthMeshCaptureMenuState::verifyCameraSetup);
		}
		else if (m_videoSourceComponent->getVideoStreamingStatus() == eVideoStreamingStatus::failed)
		{
			setMenuState(eDepthMeshCaptureMenuState::failedVideoStartStreamRequest);
		}
		return;
	}

	// Keep the undistorted color buffer current while the operator is framing
	// the shot; runCapture() reads whatever the last processed frame produced.
	if (menuState == eDepthMeshCaptureMenuState::verifyCameraSetup)
	{
		m_monoDistortionView->readAndProcessVideoFrame();
	}
}

void AppStage_DepthMeshCapture::onGui()
{
	AppStage::onGui();

	constexpr float k_panelWidth= 415.f;
	const float displayWidth= m_ownerWindow->getWidth();
	const float displayHeight= m_ownerWindow->getHeight();

	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);
	constexpr ImGuiWindowFlags k_flags=
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
	MkGuiScopedWindow panel("##DepthMeshCapture", nullptr, k_flags);
	if (!panel)
		return;

	for (IGuiPanel* guiPanel : m_guiPanels)
		guiPanel->onGui();
}

void AppStage_DepthMeshCapture::onCaptureEvent()
{
	// Show the "running" state before the blocking inference call so the panel
	// does not sit on "Capture" while the model runs.
	setMenuState(eDepthMeshCaptureMenuState::runningInference);

	runCapture();
}

void AppStage_DepthMeshCapture::runCapture()
{
	if (m_inference == nullptr)
	{
		const std::filesystem::path modelDirectory= std::filesystem::current_path() / k_depthMeshMoGeModelSubdirectory;

		MoGeInference::Config config;
		config.modelDirectory= modelDirectory.string();

		auto inference= std::make_unique<MoGeInference>();
		if (!inference->startup(config))
		{
			m_capturePanel->setFailureReason(StringUtils::stringify(
				"Could not load the MoGe-2 model from '", modelDirectory.string(),
				"'. Run tools/fetch_moge2_onnx.py to download it. See MikanCmd.log for details."));
			setMenuState(eDepthMeshCaptureMenuState::failedInference);
			return;
		}

		m_inference= std::move(inference);
		m_capturePanel->setExecutionProvider(m_inference->getActiveExecutionProvider());
	}

	// Make sure the buffer reflects the newest frame rather than whatever was
	// last processed.
	m_monoDistortionView->readAndProcessVideoFrame();

	cv::Mat* bgrBuffer= m_monoDistortionView->getBGRUndistortBuffer();
	if (bgrBuffer == nullptr || bgrBuffer->empty())
	{
		m_capturePanel->setFailureReason("No undistorted color frame was available from the video source.");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return;
	}

	// Metric scale rides directly on the assumed FOV (a wrong guess shifts
	// depth by tens of percent), so the calibrated value is required, not a
	// nicety. See docs/reference/scene-lighting.md.
	MikanVideoSourceIntrinsics cameraIntrinsics;
	if (!m_currentSceneCameraComponent->getApertureIntrinsics(cameraIntrinsics))
	{
		m_capturePanel->setFailureReason("Could not resolve the camera intrinsics. Is the camera calibrated?");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return;
	}
	const float fovXDegrees= (float)cameraIntrinsics.getMonoIntrinsics().hfov;

	if (!m_inference->run(*bgrBuffer, fovXDegrees, m_geometry))
	{
		m_capturePanel->setFailureReason("Depth inference failed. See MikanCmd.log for details.");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return;
	}

	DepthMeshGenerator::Config meshConfig;
	if (!DepthMeshGenerator::generateMesh(m_geometry, meshConfig, m_mesh, m_meshStats))
	{
		m_capturePanel->setFailureReason("Mesh generation produced no usable geometry.");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return;
	}

	m_bHasResult= true;
	m_capturePanel->setMeshSummary((int)m_mesh.vertices.size(), (int)m_mesh.getTriangleCount(),
								   m_meshStats.culledDiscontinuityEdges, m_meshStats.nearDepth, m_meshStats.farDepth);

	setMenuState(eDepthMeshCaptureMenuState::verifyMesh);
}

bool AppStage_DepthMeshCapture::createStencilFromMesh()
{
	if (!m_bHasResult)
		return false;

	// The mesh vertices are camera-local, so the stencil's world transform is
	// simply the capturing camera's pose. An untracked camera would place the
	// proxy at a stale or default pose, so fail loudly instead.
	glm::mat4 cameraPose(1.f);
	if (!m_currentSceneCameraComponent->getStageSpaceAperturePose(cameraPose))
	{
		m_capturePanel->setFailureReason("Could not resolve the camera pose. Is the camera tracked?");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return false;
	}

	const std::filesystem::path projectDirectory= PathUtils::getProjectDirectory();
	if (projectDirectory.empty())
	{
		m_capturePanel->setFailureReason("No project is loaded, so there is nowhere to save the mesh.");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return false;
	}

	// Unique per capture: regenerating under a reused path would collide with
	// the model resource cache, which keys on the file path.
	std::time_t now= std::time(nullptr);
	std::tm timeInfo;
	localtime_s(&timeInfo, &now);
	char stencilName[64];
	std::strftime(stencilName, sizeof(stencilName), "DepthProxy_%Y%m%d_%H%M%S", &timeInfo);

	const std::filesystem::path modelsDirectory= projectDirectory / "models";
	std::filesystem::create_directories(modelsDirectory);
	const std::filesystem::path objPath= modelsDirectory / (std::string(stencilName) + ".obj");

	if (!DepthMeshGenerator::saveObj(m_mesh, objPath.string(), stencilName))
	{
		m_capturePanel->setFailureReason(
			StringUtils::stringify("Failed to write the mesh to '", objPath.string(), "'."));
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return false;
	}

	// The stencil stores an absolute model path - the importer loads the path
	// verbatim, so a relative one would silently fail to resolve.
	auto modelStencilSystem= getSystemOfType<ModelStencilSystem>();
	ModelStencilComponentPtr stencilComponent= modelStencilSystem->addNewObjectByTypedDefinition(
		[&](ModelStencilDefinitionPtr definition)
		{
			definition->setComponentName(stencilName);
			definition->setRelativeTransform(GlmTransform());
			definition->setIsDisabled(false);
			definition->setModelPath(objPath);
			return true;
		});
	if (!stencilComponent)
	{
		m_capturePanel->setFailureReason("Failed to create the model stencil.");
		setMenuState(eDepthMeshCaptureMenuState::failedInference);
		return false;
	}

	stencilComponent->setWorldTransform(cameraPose);

	m_capturePanel->setCreatedStencilName(stencilName);

	MIKAN_LOG_INFO("AppStage_DepthMeshCapture")
		<< "Created depth proxy stencil '" << stencilName << "' (" << m_mesh.getTriangleCount() << " triangles, "
		<< objPath.string() << ")";

	return true;
}

void AppStage_DepthMeshCapture::onApplyEvent()
{
	if (createStencilFromMesh())
		setMenuState(eDepthMeshCaptureMenuState::captureComplete);
}

void AppStage_DepthMeshCapture::onRedoEvent()
{
	m_bHasResult= false;
	setMenuState(eDepthMeshCaptureMenuState::verifyCameraSetup);
}

void AppStage_DepthMeshCapture::onCancelEvent() { getOwnerWindow()->popAppState(); }

void AppStage_DepthMeshCapture::renderDepthPreview()
{
	if (!m_bHasResult)
		return;

	IMkGraphicsContext* graphicsContext= getGraphicsContext();
	IMkLineRenderer* lineRenderer= graphicsContext->getLineRenderer();
	IMkViewportPtr renderingViewport= graphicsContext->getRenderingViewport();
	glm::i32vec2 viewportOrigin, viewportSize;
	if (lineRenderer == nullptr || renderingViewport == nullptr
		|| !renderingViewport->getRenderingViewport(viewportOrigin, viewportSize))
	{
		return;
	}

	const float frameWidth= (float)m_monoDistortionView->getFrameWidth();
	const float frameHeight= (float)m_monoDistortionView->getFrameHeight();
	const float viewportX0= (float)viewportOrigin.x;
	const float viewportY0= (float)viewportOrigin.y;
	const float viewportX1= (float)viewportOrigin.x + (float)viewportSize.x - 1.f;
	const float viewportY1= (float)viewportOrigin.y + (float)viewportSize.y - 1.f;

	// Colored by normalized inverse depth (red = near, blue = far), matching
	// the panel text. Inverse depth spreads the resolution toward the near
	// field, which is where the proxy has to be right.
	const float nearInverse= 1.f / std::max(m_meshStats.nearDepth, 1e-3f);
	const float farInverse= 1.f / std::max(m_meshStats.farDepth, 1e-3f);
	const float inverseRange= std::max(nearInverse - farInverse, 1e-6f);

	// A stride independent of the mesh stride: the preview needs readability,
	// not density.
	constexpr int k_previewStride= 6;
	for (int y= 0; y < m_geometry.depth.rows; y+= k_previewStride)
	{
		const float* depthRow= m_geometry.depth.ptr<float>(y);
		const uint8_t* maskRow= m_geometry.mask.ptr<uint8_t>(y);
		for (int x= 0; x < m_geometry.depth.cols; x+= k_previewStride)
		{
			if (maskRow[x] == 0 || !std::isfinite(depthRow[x]))
				continue;

			const float normalized= (1.f / depthRow[x] - farInverse) / inverseRange;
			const glm::vec3 color(normalized, 0.25f, 1.f - normalized);

			const glm::vec2 framePoint((float)x + 0.5f, (float)y + 0.5f);
			const glm::vec2 windowPoint= remapPointIntoTarget(frameWidth, frameHeight, viewportX0, viewportY0,
															  viewportX1, viewportY1, framePoint);
			lineRenderer->addPoint2d(windowPoint, color, 2.f);
		}
	}
}

void AppStage_DepthMeshCapture::render(IMkViewportPtr targetViewport)
{
	switch (m_capturePanel->getMenuState())
	{
	case eDepthMeshCaptureMenuState::verifyCameraSetup:
	case eDepthMeshCaptureMenuState::runningInference:
		m_monoDistortionView->renderSelectedVideoBuffers();
		break;

	case eDepthMeshCaptureMenuState::verifyMesh:
	case eDepthMeshCaptureMenuState::captureComplete:
		m_monoDistortionView->renderSelectedVideoBuffers();
		renderDepthPreview();
		break;

	default:
		break;
	}

	getGraphicsContext()->getLineRenderer()->render(true);
}
