///-- includes -----
#include "App.h"
#include "CameraComponent.h"
#include "ClientSourceManager.h"
#include "Colors.h"
#include "CompositorComponent.h"
#include "EditorObjectSystem.h"
#include "InputManager.h"
#include "IMkGraphicsContext.h"
#include "IMkLineRenderer.h"
#include "IMkState.h"
#include "IMkTextRenderer.h"
#include "IMkTexture.h"
#include "IMkWireframeMesh.h"
#include "MathGLM.h"
#include "MathMikan.h"
#include "MainWindow.h"
#include "MarkerObjectSystem.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanCamera.h"
#include "MikanViewport.h"
#include "ObjectSystemRenderQueries.h"
#include "MikanObjectSystem.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanObject.h"
#include "MkScene.h"
#include "MkStateStack.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "Project/GuiPanel_ProjectMarkers.h"
#include "Project/GuiPanel_ProjectScenes.h"
#include "Project/GuiPanel_ProjectSettings.h"
#include "Project/GuiPanel_ProjectSources.h"
#include "Project/GuiPanel_ProjectStages.h"
#include "Project/GuiPanel_ProjectTracking.h"
#include "BoxShapeSystem.h"
#include "ModelShapeSystem.h"
#include "QuadShapeSystem.h"
#include "RGBSpotLightSystem.h"
#include "RGBPixelGridSystem.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "TextStyle.h"
#include "TrackingVolumeComponent.h"
#include "VRTrackingVolumeComponent.h"
#include "VRObjectSystem.h"
#include "VideoSourceComponent.h"

#include <easy/profiler.h>

#include "MkGuiScopedTabBar.h"
#include "MkGuiScopedTabItem.h"
#include "MkGuiScopedWindow.h"
#include "opencv2/opencv.hpp"

//-- statics ----
const char* AppStage_Project::APP_STAGE_NAME = "Compositor";

//-- public methods -----
AppStage_Project::AppStage_Project(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_Project::APP_STAGE_NAME)	
{
}

AppStage_Project::~AppStage_Project()
{
	m_viewport = nullptr;
	m_activeCompositors.clear();
}

void AppStage_Project::enter()
{
	AppStage::enter();

	// Create a mikan scene for 3d rendering
	m_mkScene = std::make_shared<MkScene>();
	m_mkScene->setLightColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
	m_mkScene->setLightDirection(glm::vec3(glm::normalize(glm::vec3(0.5f, -1.f, 0.5f))));

	// Cache a ref to the project
	m_project = getProjectConfig();

	// Cache object systems we'll be accessing
	ProjectManagerPtr objectSystemManager = m_ownerWindow->getProjectManager();
	m_editorSystem = objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_sceneObjectSystem = objectSystemManager->getSystemOfType<SceneObjectSystem>();
	m_anchorObjectSystem = objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_cameraObjectSystem = objectSystemManager->getSystemOfType<CameraObjectSystem>();
	m_markerObjectSystem = objectSystemManager->getSystemOfType<MarkerObjectSystem>();
	m_quadStencilSystem = objectSystemManager->getSystemOfType<QuadStencilSystem>();
	m_boxStencilSystem = objectSystemManager->getSystemOfType<BoxStencilSystem>();
	m_modelStencilSystem = objectSystemManager->getSystemOfType<ModelStencilSystem>();
	m_quadShapeSystem = objectSystemManager->getSystemOfType<QuadShapeSystem>();
	m_boxShapeSystem = objectSystemManager->getSystemOfType<BoxShapeSystem>();
	m_modelShapeSystem = objectSystemManager->getSystemOfType<ModelShapeSystem>();
	m_pixelGridLightSystem = objectSystemManager->getSystemOfType<RGBPixelGridSystem>();
	m_spotLightSystem = objectSystemManager->getSystemOfType<RGBSpotLightSystem>();

	// Stage Panel collision set
	m_stageObjectSystemFilter.insert(m_editorSystem.lock().get());
	m_stageObjectSystemFilter.insert(m_cameraObjectSystem.lock().get());
	m_stageObjectSystemFilter.insert(m_pixelGridLightSystem.lock().get());
	m_stageObjectSystemFilter.insert(m_spotLightSystem.lock().get());

	// Scene Panel collision set
	m_sceneObjectSystemFilter.insert(m_anchorObjectSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_quadStencilSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_boxStencilSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_modelStencilSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_quadShapeSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_boxShapeSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_modelShapeSystem.lock().get());
	m_sceneObjectSystemFilter.insert(m_stageObjectSystemFilter.begin(), m_stageObjectSystemFilter.end());

	// Tracking Panel collision set
	// nothing for now

	// Setup Scene viewport
	{
		const glm::i32vec2 viewportOrigin = { 0, 45 };
		const glm::i32vec2 viewportSize = { 1280, 720 };

		m_viewport = getFirstViewport();
		m_viewport->setViewport(viewportOrigin, viewportSize);

		MikanCameraPtr sceneCamera = m_viewport->getCurrentMikanCamera();
		sceneCamera->setCameraMovementMode(eCameraMovementMode::fly);
		sceneCamera->setName("scene camera");

		// Register the scene with the primary viewport
		m_editorSystem.lock()->bindViewport(m_viewport);
	}

	// Listen for changes to the current active scene
	{
		SceneObjectSystemPtr sceneSystem = getSystemOfType<SceneObjectSystem>();

		sceneSystem->OnSceneActivated +=
			MakeDelegate(this, &AppStage_Project::onSceneActivated);
		sceneSystem->OnSceneDeactivated +=
			MakeDelegate(this, &AppStage_Project::onSceneDeactivated);
		
		// Rebuild compositor viewports for the active scene
		SceneComponentPtr activeScene= sceneSystem->getCurrentScene();
		if (activeScene)
		{
			onSceneActivated(activeScene);
		}
	}

	// Setup hotkeys
	{
		InputManager* inputManager = getOwnerWindow()->getInputManager();

		// Hotkeys for switching between viewport modes
		inputManager->fetchOrAddKeyBindings(MkKey::COMMA)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Project::cyclePreviousCompositorCamera);
		inputManager->fetchOrAddKeyBindings(MkKey::PERIOD)->OnKeyPressed +=
			MakeDelegate(this, &AppStage_Project::cycleNextCompositorCamera);
	}

	// Create ImGui GuiPanel context and project panels
	{
		m_projectGuiPanelContext = new ProjectGuiPanelContext(this);
		m_projectGuiPanelContext->init();

		// Wire up component panel delegates
		m_projectGuiPanelContext->getMarkerPanel()->OnMarkerSelected =
			MakeDelegate(this, &AppStage_Project::onMarkerSelected);

		m_projectScenesPanel = addGuiPanel<GuiPanel_ProjectScenes>();
		m_projectScenesPanel->init(m_projectGuiPanelContext);

		m_projectStagesPanel = addGuiPanel<GuiPanel_ProjectStages>();
		m_projectStagesPanel->init(m_projectGuiPanelContext);

		m_projectSourcesPanel = addGuiPanel<GuiPanel_ProjectSources>();
		m_projectSourcesPanel->init(m_projectGuiPanelContext);

		m_projectTrackingPanel = addGuiPanel<GuiPanel_ProjectTracking>();
		m_projectTrackingPanel->init(m_projectGuiPanelContext);

		m_projectMarkersPanel = addGuiPanel<GuiPanel_ProjectMarkers>();
		m_projectMarkersPanel->init(m_projectGuiPanelContext);

		m_projectSettingsPanel = addGuiPanel<GuiPanel_ProjectSettings>();
		m_projectSettingsPanel->init(m_projectGuiPanelContext);
	}

	// Start of in the stages panel
	setActivePanel(eProjectAppStageActivePanel::Stages);
}

void AppStage_Project::exit()
{
	// Clean up the 3d scene
	m_mkScene = nullptr;

	// Clean up the GuiPanel Context (panels themselves are owned by AppStage::m_guiPanels)
	delete m_projectGuiPanelContext;
	m_projectGuiPanelContext = nullptr;
	m_projectScenesPanel = nullptr;
	m_projectStagesPanel = nullptr;
	m_projectSourcesPanel = nullptr;
	m_projectTrackingPanel = nullptr;
	m_projectMarkersPanel = nullptr;
	m_projectSettingsPanel = nullptr;

	{
		SceneObjectSystemPtr sceneSystem = m_sceneObjectSystem.lock();

		// Rebuild compositor viewports for the active scene
		SceneComponentPtr activeScene = sceneSystem->getCurrentScene();
		if (activeScene)
		{
			onSceneDeactivated(activeScene);
		}

		sceneSystem->OnSceneActivated -=
			MakeDelegate(this, &AppStage_Project::onSceneActivated);
		sceneSystem->OnSceneDeactivated -=
			MakeDelegate(this, &AppStage_Project::onSceneDeactivated);
	}

	// Unregister all viewports from the editor
	m_editorSystem.lock()->clearViewports();

	// GuiPanels are disposed automatically by AppStage::exit()
	AppStage::exit();
}

void AppStage_Project::pause()
{
	AppStage::pause();
}

void AppStage_Project::resume()
{
	AppStage::resume();
}

void AppStage_Project::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the timing dependent state for the project GuiPanels
	m_projectGuiPanelContext->update(deltaSeconds);

	// Update the camera pose for the currently active camera
	updateCompositorCameras();
}

void AppStage_Project::onGui()
{
	// Process deferred events emitted due to Gui interaction
	AppStage::onGui();

	constexpr float k_panelWidth = 415.f;
	const float displayWidth = m_ownerWindow->getWidth();
	const float displayHeight = m_ownerWindow->getHeight();

	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);

	constexpr ImGuiWindowFlags k_panelFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar;

	MkGuiScopedWindow panel("##ProjectPanel", nullptr, k_panelFlags);
	if (!panel)
		return;

	static const char* k_tabLabels[(int)eProjectAppStageActivePanel::COUNT] = {
		"Scenes", "Stages", "Sources", "Tracking", "Markers", "Settings"
	};
	IGuiPanel* k_tabPanels[(int)eProjectAppStageActivePanel::COUNT] = {
		m_projectScenesPanel,
		m_projectStagesPanel,
		m_projectSourcesPanel,
		m_projectTrackingPanel,
		m_projectMarkersPanel,
		m_projectSettingsPanel
	};
	constexpr int k_tabCount = (int)(sizeof(k_tabLabels) / sizeof(k_tabLabels[0]));

	MkGuiScopedTabBar tabBar("##ProjectTabs");
	if (tabBar)
	{
		for (int i = 0; i < k_tabCount; i++)
		{
			MkGuiScopedTabItem tabItem(k_tabLabels[i]);

			if (tabItem)
			{
				if (k_tabPanels[i] != nullptr)
				{
					setActivePanel((eProjectAppStageActivePanel)i);
					k_tabPanels[i]->onGui();
				}
			}
		}
	}
}

// Panel Selection
void AppStage_Project::setActivePanel(eProjectAppStageActivePanel newPanel)
{
	if (newPanel != m_activePanel)
	{
		m_activePanel = newPanel;
		onActivePanelChanged();
	}
}

void AppStage_Project::onActivePanelChanged()
{
	// Update the collision filter on the editor system
	switch (m_activePanel)
	{
	case eProjectAppStageActivePanel::Scenes:
	case eProjectAppStageActivePanel::Settings:
		m_editorSystem.lock()->setObjectSystemSelectionFilter(m_sceneObjectSystemFilter);
		break;
	case eProjectAppStageActivePanel::Stages:
	case eProjectAppStageActivePanel::Sources:
		m_editorSystem.lock()->setObjectSystemSelectionFilter(m_stageObjectSystemFilter);
		break;
	default:
		m_editorSystem.lock()->setObjectSystemSelectionFilter(m_emptyObjectSystemFilter);
	}
}

// Scene
void AppStage_Project::onSceneDeactivated(SceneComponentPtr oldScene)
{
	disposeCompositorViewportCameras();
}

void AppStage_Project::onSceneActivated(SceneComponentPtr newScene)
{
	createCompositorViewportCameras();
}

void AppStage_Project::createCompositorViewportCameras()
{
	// Create a camera for each active compositor in the scene
	for (const CompositorComponentWeakPtr compositorWeakPtr : m_activeCompositors)
	{
		CompositorComponentPtr compositor= compositorWeakPtr.lock();
		MikanCameraPtr mikanCamera= m_viewport->addMikanCamera();

		mikanCamera->setName(compositor->getName());

		// Camera transform will be updated each frame in updateCompositorCameras()
		mikanCamera->setCameraMovementMode(eCameraMovementMode::stationary);

		// Apply video source camera intrinsics to the camera
		VideoSourceComponentPtr videoSourceComponent = compositor->getVideoSourceComponent();
		if (videoSourceComponent != nullptr)
		{
			MikanVideoSourceIntrinsics cameraIntrinsics;
			videoSourceComponent->getCameraIntrinsics(cameraIntrinsics);

			mikanCamera->applyMonoCameraIntrinsics(&cameraIntrinsics);
		}
	}

	// Apply initial camera transforms from the compositors
	updateCompositorCameras();
}

void AppStage_Project::disposeCompositorViewportCameras()
{
	// Remove all but the first camera from the main viewport
	while (m_viewport->getCameraCount() > 1)
	{
		m_viewport->removeCameraByIndex(m_viewport->getCameraCount() - 1);
	}
}

void AppStage_Project::updateCompositorCameras()
{
	for (size_t compositorIndex = 0; compositorIndex < m_activeCompositors.size(); compositorIndex++)
	{
		size_t cameraIndex = compositorIndex + 1; // Skip the first camera which is the vr camera
		CompositorComponentPtr compositor = m_activeCompositors[compositorIndex].lock();
		MikanCameraPtr camera = m_viewport->getMikanCameraByIndex((int)cameraIndex);

		if (compositor && camera)
		{
			CameraComponentPtr cameraComponent = compositor->getCameraComponent();
			if (cameraComponent)
			{
				glm::mat4 cameraXform;
				if (cameraComponent->getStageSpaceAperturePose(cameraXform))
				{
					camera->setCameraTransform(cameraXform);
				}
			}
		}
	}
}

void AppStage_Project::cyclePreviousCompositorCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		int newCameraIndex = m_viewport->getCurrentCameraIndex() - 1;
		if (newCameraIndex < 0)
			newCameraIndex = m_viewport->getCameraCount() - 1;

		m_viewport->setCurrentCamera(newCameraIndex);
	}
}

void AppStage_Project::cycleNextCompositorCamera()
{
	if (m_viewport->getIsMouseInViewport())
	{
		int newCameraIndex = m_viewport->getCurrentCameraIndex() + 1;
		if (newCameraIndex >= m_viewport->getCameraCount())
			newCameraIndex = 0;

		m_viewport->setCurrentCamera(newCameraIndex);
	}
}

// Compositor Model UI Events
void AppStage_Project::onReturnEvent()
{
	m_ownerWindow->popAppState();
}


void AppStage_Project::onMarkerSelected(int arucoId)
{
	// Marker selected — arucoId can be used for custom rendering/preview in the future
}

void AppStage_Project::render(IMkViewportPtr targetViewport)
{
	IMkGraphicsContext* graphicsContext = getGraphicsContext();
	MikanCameraPtr viewportCamera = m_viewport->getCurrentMikanCamera();

	MkStateStack& stageStack = graphicsContext->getMkStateStack();
	MkScopedState scopedState = stageStack.createScopedState("AppStage_Project::render");
	IMkState* mkState = scopedState.getStackState();

	// Enable Depth Test while drawing the scene
	mkState->enableFlag(eMkStateFlagType::depthTest);

	// Clear the scene of any previously rendered instances
	m_mkScene->removeAllInstances();

	// Panel specific rendering
	switch (m_activePanel)
	{
	case eProjectAppStageActivePanel::Scenes:
	case eProjectAppStageActivePanel::Settings:
		renderProjectScene(graphicsContext, viewportCamera);
		break;
	case eProjectAppStageActivePanel::Stages:
	case eProjectAppStageActivePanel::Sources:
		renderProjectStage(graphicsContext, viewportCamera);
		break;
	case eProjectAppStageActivePanel::Tracking:
	case eProjectAppStageActivePanel::Markers:
		renderProjectTracking(graphicsContext, viewportCamera);
		break;
	}

	// Render the 3d scene
	m_mkScene->render(viewportCamera, graphicsContext->getMkStateStack());

	// Draw tracking space
	drawGrid(graphicsContext, glm::mat4(1.f), 10.f, 10.f, 20, 20, Colors::GhostWhite);

	if (getEditorSettings().bRenderOrigin)
	{
		debugRenderOrigin();
	}
}

SceneComponentConstPtr AppStage_Project::getCurrentSceneConst() const
{
	return getSystemOfTypeConst<const SceneObjectSystem>()->getCurrentScene();
}

StageComponentConstPtr AppStage_Project::getCurrentStageConst() const
{
	SceneComponentConstPtr currentScene = getCurrentSceneConst();
	if (currentScene)
	{
		return currentScene->getParentStage();
	}
	return nullptr;
}

TrackingVolumeComponentConstPtr AppStage_Project::getCurrentTrackingVolumeConst() const
{
	StageComponentConstPtr currentStage = getCurrentStageConst();
	if (currentStage)
	{
		return currentStage->getTrackingVolumeConst();
	}
	return nullptr;
}

void AppStage_Project::renderProjectScene(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	SceneComponentConstPtr currentScene = getCurrentSceneConst();
	if (currentScene)
	{
		auto editorObjectSystem = getObjectSystemOfType<EditorObjectSystem>();
		const EditorSettings& editorSettings = editorObjectSystem->getEditorSettings();

		// Render the stage
		renderProjectStage(graphicsContext, viewportCamera);

		// Render the editor gizmo if an actor is selected
		editorObjectSystem->customRender(graphicsContext, viewportCamera);

		// Render anchors if enabled
		if (editorSettings.bDebugRenderAnchors)
		{
			AnchorObjectSystemPtr anchorSystem= m_anchorObjectSystem.lock();

			addAllRenderablesToMkScene(anchorSystem, m_mkScene);
			anchorSystem->customRender(graphicsContext, viewportCamera);
		}

		// Render the stencils if enabled
		if (editorSettings.bDebugRenderBoxStencils)
		{
			BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();

			addAllRenderablesToMkScene(boxStencilSystem, m_mkScene);
			boxStencilSystem->customRender(graphicsContext, viewportCamera);
		}
		if (editorSettings.bDebugRenderModelStencils)
		{
			ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();

			addAllRenderablesToMkScene(modelStencilSystem, m_mkScene);
			modelStencilSystem->customRender(graphicsContext, viewportCamera);
		}
		if (editorSettings.bDebugRenderQuadStencils)
		{
			QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();

			addAllRenderablesToMkScene(quadStencilSystem, m_mkScene);
			quadStencilSystem->customRender(graphicsContext, viewportCamera);
		}

		// Render the shapes if enabled
		if (editorSettings.bDebugRenderQuadShapes)
		{
			QuadShapeSystemPtr quadShapeSystem = m_quadShapeSystem.lock();

			addAllRenderablesToMkScene(quadShapeSystem, m_mkScene);
			quadShapeSystem->customRender(graphicsContext, viewportCamera);
		}
		if (editorSettings.bDebugRenderBoxShapes)
		{
			BoxShapeSystemPtr boxShapeSystem = m_boxShapeSystem.lock();

			addAllRenderablesToMkScene(boxShapeSystem, m_mkScene);
			boxShapeSystem->customRender(graphicsContext, viewportCamera);
		}
		if (editorSettings.bDebugRenderModelShapes)
		{
			ModelShapeSystemPtr modelShapeSystem = m_modelShapeSystem.lock();

			addAllRenderablesToMkScene(modelShapeSystem, m_mkScene);
			modelShapeSystem->customRender(graphicsContext, viewportCamera);
		}
	}
}

void AppStage_Project::renderProjectStage(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	StageComponentConstPtr stageComponent = getCurrentStageConst();
	if (stageComponent)
	{
		// Render static meshes for cameras and light objects
		addAllRenderablesToMkScene(m_cameraObjectSystem.lock(), m_mkScene);
		addAllRenderablesToMkScene(m_pixelGridLightSystem.lock(), m_mkScene);
		addAllRenderablesToMkScene(m_spotLightSystem.lock(), m_mkScene);

		// Draw volumetric cone visualizations for spot lights
		if (auto spotLightSystem = m_spotLightSystem.lock())
			spotLightSystem->customRender(graphicsContext, viewportCamera);

		// Draw the cameras on the stage
		renderCameraComponents(graphicsContext, viewportCamera, stageComponent);

		// Draw the tracking volume for the stage
		renderProjectTracking(graphicsContext, viewportCamera);

		// Draw the state bounds
		stageComponent->renderStageBounds(graphicsContext, glm::mat4(1.f));
	}
}

void AppStage_Project::renderProjectTracking(
	IMkGraphicsContext* graphicsContext, 
	MikanCameraPtr viewportCamera) const
{
	TrackingVolumeComponentConstPtr trackingVolume = getCurrentTrackingVolumeConst();
	if (trackingVolume)
	{
		switch (trackingVolume->getTrackingVolumeType())
		{
		case eTrackingVolumeType::vr:
			renderVRTrackingVolume(
				graphicsContext, 
				viewportCamera, 
				std::dynamic_pointer_cast<const VRTrackingVolumeComponent>(trackingVolume));
			break;
		case eTrackingVolumeType::marker:
			renderMarkerTrackingVolume(
				graphicsContext, 
				viewportCamera, 
				std::dynamic_pointer_cast<const MarkerTrackingVolumeComponent>(trackingVolume));
			break;
		}
	}
}

void AppStage_Project::renderVRTrackingVolume(
	IMkGraphicsContext* graphicsContext, 
	MikanCameraPtr viewportCamera, 
	VRTrackingVolumeComponentConstPtr vrTrackingVolume) const
{
	VRObjectSystemPtr vrObjectSystem = getObjectSystemOfType<VRObjectSystem>();

	// Resolve VRSpace -> StageSpace offset from the VR tracking volume
	glm::mat4 vrSpaceToStageSpace = vrTrackingVolume->getVRSpaceToStageSpace();

	// Get the tracking volume origin marker (if it exists) so we can render it in the correct space
	const MikanMarkerID originMarkerId = vrTrackingVolume->getOriginMarkerId();
	MarkerComponentPtr markerComp;
	if (originMarkerId != INVALID_MIKAN_ID)
	{
		MarkerObjectSystemPtr markerSystem = getObjectSystemOfType<MarkerObjectSystem>();
		markerComp = markerSystem->getMarkerById(originMarkerId);
	}

	if (m_activePanel == eProjectAppStageActivePanel::Tracking)
	{

		switch (vrTrackingVolume->getDisplayTrackingSpace())
		{
		case MikanTrackingSpace_VR:
			{
				// Render the VR devices in VR space
				addAllVRDevicesToMkScene(vrObjectSystem, m_mkScene, glm::mat4(1.f));

				// Render the VR Device info in VR space
				renderAllVRDeviceInfo(vrObjectSystem, graphicsContext, viewportCamera, glm::mat4(1.f));

				// Render the origin marker as a textured quad at VRSpace Origin
				if (markerComp)
				{
					glm::mat4 stageSpaceToVRSpace = glm::inverse(vrSpaceToStageSpace);

					markerComp->renderArucoMarker(graphicsContext, viewportCamera, stageSpaceToVRSpace);
				}
			} break;
		case MikanTrackingSpace_Stage:
			{
				// Render the VR devices in stage space
				addAllVRDevicesToMkScene(vrObjectSystem, m_mkScene, vrSpaceToStageSpace);

				// Render the VR Device info in stage space
				renderAllVRDeviceInfo(vrObjectSystem, graphicsContext, viewportCamera, vrSpaceToStageSpace);

				// Render the origin marker as a textured quad at world origin
				if (markerComp)
				{
					markerComp->renderArucoMarker(graphicsContext, viewportCamera, glm::mat4(1.f));
				}
			}
			break;
		}
	}
	else
	{
		// Render the VR devices in scene space
		addAllVRDevicesToMkScene(vrObjectSystem, m_mkScene, vrSpaceToStageSpace);

		// Render the origin marker as a textured quad at world origin
		if (markerComp)
		{
			markerComp->renderArucoMarker(graphicsContext, viewportCamera, glm::mat4(1.f));
		}
	}
}

void AppStage_Project::renderMarkerTrackingVolume(
	IMkGraphicsContext* graphicsContext,
	MikanCameraPtr viewportCamera,
	MarkerTrackingVolumeComponentConstPtr markerTrackingVolume) const
{
	const MikanMarkerID originMarkerId = markerTrackingVolume->getOriginMarkerId();
	if (originMarkerId != INVALID_MIKAN_ID)
	{
		MarkerObjectSystemPtr markerSystem = getObjectSystemOfType<MarkerObjectSystem>();
		MarkerComponentPtr markerComp = markerSystem->getMarkerById(originMarkerId);
		if (markerComp)
		{
			markerComp->renderArucoMarker(graphicsContext, viewportCamera, glm::mat4(1.f));
		}
	}
}

void AppStage_Project::renderCameraComponents(
	IMkGraphicsContext* graphicsContext, 
	MikanCameraPtr viewportCamera,
	StageComponentConstPtr stageComponent) const
{
	getObjectSystemOfType<CameraObjectSystem>()->customRender(
		graphicsContext, 
		viewportCamera, 
		[stageComponent](CameraComponentPtr camera) {
			return camera->getOwnerStageComponent() == stageComponent;
		});
}

void AppStage_Project::debugRenderOrigin() const
{
	IMkGraphicsContext* graphicsContext = getGraphicsContext();
	TextStyle style = getDefaultTextStyle();

	drawTransformedAxes(graphicsContext, glm::mat4(1.f), 1.f, 1.f, 1.f);
	drawTextAtWorldPosition(graphicsContext, style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
}

// -- IRemoteControllable Interface -- //
bool AppStage_Project::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	if (command == "add_new_script")
	{
		// TODO: Implement script removal - need to determine component ID parameter handling
		return true;
	}
	else if (command == "remove_script")
	{
		// TODO: Implement script removal - need to determine component ID parameter handling
		return true;
	}

	return AppStage::handleRemoteControlCommand(command, parameters, outResults);
}