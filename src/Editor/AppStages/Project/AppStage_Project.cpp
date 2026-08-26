///-- includes -----
#include "App.h"
#include "CameraComponent.h"
#include "CameraMath.h"
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
#include "LightEnvironmentComponent.h"
#include "LightEnvironmentSystem.h"
#include "LocText.h"
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
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "ShapeComponent.h"
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
const char* AppStage_Project::APP_STAGE_NAME= "Compositor";

//-- public methods -----
AppStage_Project::AppStage_Project(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_Project::APP_STAGE_NAME)
{
}

AppStage_Project::~AppStage_Project() { m_viewport= nullptr; }

void AppStage_Project::enter()
{
	AppStage::enter();

	// Create a mikan scene for 3d rendering
	m_mkScene= std::make_shared<MkScene>();
	m_mkScene->setLightColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
	m_mkScene->setLightDirection(glm::vec3(glm::normalize(glm::vec3(0.5f, -1.f, 0.5f))));

	// Cache a ref to the project
	m_project= getProjectConfig();

	// Cache object systems we'll be accessing
	ProjectManagerPtr objectSystemManager= m_ownerWindow->getProjectManager();
	m_editorSystem= objectSystemManager->getSystemOfType<EditorObjectSystem>();
	m_sceneObjectSystem= objectSystemManager->getSystemOfType<SceneObjectSystem>();
	m_anchorObjectSystem= objectSystemManager->getSystemOfType<AnchorObjectSystem>();
	m_cameraObjectSystem= objectSystemManager->getSystemOfType<CameraObjectSystem>();
	m_markerObjectSystem= objectSystemManager->getSystemOfType<MarkerObjectSystem>();
	m_quadStencilSystem= objectSystemManager->getSystemOfType<QuadStencilSystem>();
	m_boxStencilSystem= objectSystemManager->getSystemOfType<BoxStencilSystem>();
	m_modelStencilSystem= objectSystemManager->getSystemOfType<ModelStencilSystem>();
	m_quadShapeSystem= objectSystemManager->getSystemOfType<QuadShapeSystem>();
	m_boxShapeSystem= objectSystemManager->getSystemOfType<BoxShapeSystem>();
	m_modelShapeSystem= objectSystemManager->getSystemOfType<ModelShapeSystem>();
	m_pixelGridLightSystem= objectSystemManager->getSystemOfType<RGBPixelGridSystem>();
	m_spotLightSystem= objectSystemManager->getSystemOfType<RGBSpotLightSystem>();

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
		const glm::i32vec2 viewportOrigin= {0, 0};
		const glm::i32vec2 viewportSize= {1280, 720};

		m_viewport= getFirstViewport();
		m_viewport->setViewport(viewportOrigin, viewportSize);

		MikanCameraPtr sceneCamera= m_viewport->getCurrentMikanCamera();
		sceneCamera->setCameraMovementMode(eCameraMovementMode::fly);
		sceneCamera->setName("scene camera");

		// Register the scene with the primary viewport
		m_editorSystem.lock()->bindViewport(m_viewport);
	}

	// Create ImGui GuiPanel context and project panels
	{
		m_projectGuiPanelContext= new ProjectGuiPanelContext(this);
		m_projectGuiPanelContext->init();

		// Wire up component panel delegates
		m_projectGuiPanelContext->getMarkerPanel()->OnMarkerSelected=
			MakeDelegate(this, &AppStage_Project::onMarkerSelected);

		m_projectScenesPanel= addGuiPanel<GuiPanel_ProjectScenes>();
		m_projectScenesPanel->init(m_projectGuiPanelContext);

		m_projectStagesPanel= addGuiPanel<GuiPanel_ProjectStages>();
		m_projectStagesPanel->init(m_projectGuiPanelContext);

		m_projectSourcesPanel= addGuiPanel<GuiPanel_ProjectSources>();
		m_projectSourcesPanel->init(m_projectGuiPanelContext);

		m_projectTrackingPanel= addGuiPanel<GuiPanel_ProjectTracking>();
		m_projectTrackingPanel->init(m_projectGuiPanelContext);

		m_projectMarkersPanel= addGuiPanel<GuiPanel_ProjectMarkers>();
		m_projectMarkersPanel->init(m_projectGuiPanelContext);

		m_projectSettingsPanel= addGuiPanel<GuiPanel_ProjectSettings>();
		m_projectSettingsPanel->init(m_projectGuiPanelContext);
	}

	// Start of in the stages panel
	setActivePanel(eProjectAppStageActivePanel::Stages);
}

void AppStage_Project::exit()
{
	// Clean up the 3d scene
	m_mkScene= nullptr;

	// Clean up the GuiPanel Context (panels themselves are owned by AppStage::m_guiPanels)
	delete m_projectGuiPanelContext;
	m_projectGuiPanelContext= nullptr;
	m_projectScenesPanel= nullptr;
	m_projectStagesPanel= nullptr;
	m_projectSourcesPanel= nullptr;
	m_projectTrackingPanel= nullptr;
	m_projectMarkersPanel= nullptr;
	m_projectSettingsPanel= nullptr;

	// Unregister all viewports from the editor
	m_editorSystem.lock()->clearViewports();

	// GuiPanels are disposed automatically by AppStage::exit()
	AppStage::exit();
}

void AppStage_Project::pause() { AppStage::pause(); }

void AppStage_Project::resume() { AppStage::resume(); }

void AppStage_Project::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Update the timing dependent state for the project GuiPanels
	m_projectGuiPanelContext->update(deltaSeconds);
}

void AppStage_Project::onGui()
{
	// Process deferred events emitted due to Gui interaction
	AppStage::onGui();

	// Viewport view-mode selector overlay (perspective / orthographic), editor camera only.
	// Compositor cameras (pool index > 0) use real lens intrinsics and must not be switched.
	if (m_viewport && m_viewport->getCurrentCameraIndex() == 0)
	{
		MikanCameraPtr camera= m_viewport->getMikanCameraByIndex(0);
		if (camera)
		{
			const glm::i32vec2 vpOrigin= m_viewport->getViewportOrigin();
			ImGui::SetNextWindowPos(ImVec2((float)vpOrigin.x + 8, (float)vpOrigin.y + 8), ImGuiCond_Always);

			constexpr ImGuiWindowFlags k_overlayFlags= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
													   | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
													   | ImGuiWindowFlags_AlwaysAutoResize;

			MkGuiScopedWindow overlay("##ViewModeOverlay", nullptr, k_overlayFlags);
			if (overlay)
			{
				const char* k_viewLabels[]= {locText("project.viewPerspective"), locText("project.viewTop"),
											 locText("project.viewBottom"),      locText("project.viewFront"),
											 locText("project.viewBack"),        locText("project.viewLeft"),
											 locText("project.viewRight")};

				int currentIndex= 0;
				if (camera->getProjectionMode() == eCameraProjectionMode::orthographic)
				{
					switch (camera->getOrthoViewpoint())
					{
					case eCameraViewpoint::top:
						currentIndex= 1;
						break;
					case eCameraViewpoint::bottom:
						currentIndex= 2;
						break;
					case eCameraViewpoint::front:
						currentIndex= 3;
						break;
					case eCameraViewpoint::back:
						currentIndex= 4;
						break;
					case eCameraViewpoint::left:
						currentIndex= 5;
						break;
					case eCameraViewpoint::right:
						currentIndex= 6;
						break;
					}
				}

				ImGui::SetNextItemWidth(110.f);
				if (ImGui::Combo("##ViewMode", &currentIndex, k_viewLabels, IM_ARRAYSIZE(k_viewLabels)))
				{
					switch (currentIndex)
					{
					case 0:
						camera->setProjectionMode(eCameraProjectionMode::perspective);
						camera->setCameraMovementMode(eCameraMovementMode::fly);
						break;
					case 1:
						camera->setOrthographicViewpoint(eCameraViewpoint::top);
						break;
					case 2:
						camera->setOrthographicViewpoint(eCameraViewpoint::bottom);
						break;
					case 3:
						camera->setOrthographicViewpoint(eCameraViewpoint::front);
						break;
					case 4:
						camera->setOrthographicViewpoint(eCameraViewpoint::back);
						break;
					case 5:
						camera->setOrthographicViewpoint(eCameraViewpoint::left);
						break;
					case 6:
						camera->setOrthographicViewpoint(eCameraViewpoint::right);
						break;
					}
				}
			}
		}
	}

	// MR camera-alignment debug readout window
	if (getEditorSettings().bDebugCameraAlignment)
	{
		renderCameraAlignmentGui();
	}

	constexpr float k_panelWidth= 415.f;
	const float displayWidth= m_ownerWindow->getWidth();
	const float displayHeight= m_ownerWindow->getHeight();

	ImGui::SetNextWindowPos(ImVec2(displayWidth - k_panelWidth, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, displayHeight), ImGuiCond_Always);

	constexpr ImGuiWindowFlags k_panelFlags=
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

	MkGuiScopedWindow panel("##ProjectPanel", nullptr, k_panelFlags);
	if (!panel)
		return;

	const char* k_tabLabels[(int)eProjectAppStageActivePanel::COUNT]= {
		locLabel("project.tabScenes"),   locLabel("project.tabStages"),  locLabel("project.tabSources"),
		locLabel("project.tabTracking"), locLabel("project.tabMarkers"), locLabel("project.tabSettings")};
	IGuiPanel* k_tabPanels[(int)eProjectAppStageActivePanel::COUNT]= {m_projectScenesPanel,  m_projectStagesPanel,
																	  m_projectSourcesPanel, m_projectTrackingPanel,
																	  m_projectMarkersPanel, m_projectSettingsPanel};
	constexpr int k_tabCount= (int)(sizeof(k_tabLabels) / sizeof(k_tabLabels[0]));

	MkGuiScopedTabBar tabBar("##ProjectTabs");
	if (tabBar)
	{
		for (int i= 0; i < k_tabCount; i++)
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
		m_activePanel= newPanel;
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

// Compositor Model UI Events
void AppStage_Project::onReturnEvent() { m_ownerWindow->popAppState(); }

void AppStage_Project::onMarkerSelected(int arucoId)
{
	// Marker selected — arucoId can be used for custom rendering/preview in the future
}

void AppStage_Project::render(IMkViewportPtr targetViewport)
{
	IMkGraphicsContext* graphicsContext= getGraphicsContext();
	MikanCameraPtr viewportCamera= m_viewport->getCurrentMikanCamera();

	MkStateStack& stageStack= graphicsContext->getMkStateStack();
	MkScopedState scopedState= stageStack.createScopedState("AppStage_Project::render");
	IMkState* mkState= scopedState.getStackState();

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

	// Draw the floor grid using the configured extent / cell size
	const EditorSettings& editorSettings= getEditorSettings();
	const float gridExtent= (editorSettings.gridExtent > 0.f) ? editorSettings.gridExtent : 10.f;
	const float gridCellSize= (editorSettings.gridCellSize > 0.f) ? editorSettings.gridCellSize : 0.5f;
	int gridSubDiv= (int)((gridExtent / gridCellSize) + 0.5f);
	if (gridSubDiv < 1)
		gridSubDiv= 1;
	drawGrid(graphicsContext, glm::mat4(1.f), gridExtent, gridExtent, gridSubDiv, gridSubDiv, Colors::GhostWhite);

	// Draw the orthographic ruler overlay (no-op unless measuring in an ortho view)
	if (auto editorSystem= m_editorSystem.lock())
	{
		editorSystem->renderRuler(graphicsContext, m_viewport);
	}

	if (editorSettings.bRenderOrigin)
	{
		debugRenderOrigin();
	}

	if (editorSettings.bDebugCameraAlignment)
	{
		renderCameraAlignmentDebug(graphicsContext, viewportCamera);
	}
}

SceneComponentConstPtr AppStage_Project::getCurrentSceneConst() const
{
	return getSystemOfTypeConst<const SceneObjectSystem>()->getCurrentScene();
}

StageComponentConstPtr AppStage_Project::getCurrentStageConst() const
{
	SceneComponentConstPtr currentScene= getCurrentSceneConst();
	if (currentScene)
	{
		return currentScene->getParentStage();
	}
	return nullptr;
}

TrackingVolumeComponentConstPtr AppStage_Project::getCurrentTrackingVolumeConst() const
{
	StageComponentConstPtr currentStage= getCurrentStageConst();
	if (currentStage)
	{
		return currentStage->getTrackingVolumeConst();
	}
	return nullptr;
}

void AppStage_Project::renderProjectScene(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	SceneComponentConstPtr currentScene= getCurrentSceneConst();
	if (currentScene)
	{
		auto editorObjectSystem= getObjectSystemOfType<EditorObjectSystem>();
		const EditorSettings& editorSettings= editorObjectSystem->getEditorSettings();

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
			BoxStencilSystemPtr boxStencilSystem= m_boxStencilSystem.lock();

			addAllRenderablesToMkScene(boxStencilSystem, m_mkScene);
			boxStencilSystem->customRender(graphicsContext, viewportCamera);
		}
		if (editorSettings.bDebugRenderModelStencils)
		{
			ModelStencilSystemPtr modelStencilSystem= m_modelStencilSystem.lock();

			addAllRenderablesToMkScene(modelStencilSystem, m_mkScene);
			modelStencilSystem->customRender(graphicsContext, viewportCamera);
		}
		if (editorSettings.bDebugRenderQuadStencils)
		{
			QuadStencilSystemPtr quadStencilSystem= m_quadStencilSystem.lock();

			addAllRenderablesToMkScene(quadStencilSystem, m_mkScene);
			quadStencilSystem->customRender(graphicsContext, viewportCamera);
		}

		// Render the shapes if enabled
		const glm::mat4 viewportVpMatrix= viewportCamera->getViewProjectionMatrix();
		if (editorSettings.bDebugRenderQuadShapes)
		{
			QuadShapeSystemPtr quadShapeSystem= m_quadShapeSystem.lock();

			std::vector<QuadShapeComponentPtr> quadShapes;
			quadShapeSystem->getQuadShapeComponentList(quadShapes);

			bool bAnyLegacyShapes= false;
			for (auto& shape : quadShapes)
			{
				if (shape->hasValidShapeGraph())
					shape->renderShapeGraph(viewportVpMatrix, graphicsContext);
				else
					bAnyLegacyShapes= true;
			}
			if (bAnyLegacyShapes)
			{
				addAllRenderablesToMkScene(quadShapeSystem, m_mkScene);
				quadShapeSystem->customRender(graphicsContext, viewportCamera);
			}
		}
		if (editorSettings.bDebugRenderBoxShapes)
		{
			BoxShapeSystemPtr boxShapeSystem= m_boxShapeSystem.lock();

			std::vector<BoxShapeComponentPtr> boxShapes;
			boxShapeSystem->getBoxShapeComponentList(boxShapes);

			bool bAnyLegacyShapes= false;
			for (auto& shape : boxShapes)
			{
				if (shape->hasValidShapeGraph())
					shape->renderShapeGraph(viewportVpMatrix, graphicsContext);
				else
					bAnyLegacyShapes= true;
			}
			if (bAnyLegacyShapes)
			{
				addAllRenderablesToMkScene(boxShapeSystem, m_mkScene);
				boxShapeSystem->customRender(graphicsContext, viewportCamera);
			}
		}
		if (editorSettings.bDebugRenderModelShapes)
		{
			ModelShapeSystemPtr modelShapeSystem= m_modelShapeSystem.lock();

			std::vector<ModelShapeComponentPtr> modelShapes;
			modelShapeSystem->getModelShapeComponentList(modelShapes);

			bool bAnyLegacyShapes= false;
			for (auto& shape : modelShapes)
			{
				if (shape->hasValidShapeGraph())
					shape->renderShapeGraph(viewportVpMatrix, graphicsContext);
				else
					bAnyLegacyShapes= true;
			}
			if (bAnyLegacyShapes)
			{
				addAllRenderablesToMkScene(modelShapeSystem, m_mkScene);
				modelShapeSystem->customRender(graphicsContext, viewportCamera);
			}
		}
	}
}

void AppStage_Project::renderProjectStage(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	StageComponentConstPtr stageComponent= getCurrentStageConst();
	if (stageComponent)
	{
		// Render static meshes for cameras and light objects
		addAllRenderablesToMkScene(m_cameraObjectSystem.lock(), m_mkScene);
		addAllRenderablesToMkScene(m_pixelGridLightSystem.lock(), m_mkScene);
		addAllRenderablesToMkScene(m_spotLightSystem.lock(), m_mkScene);

		// Draw volumetric cone visualizations for spot lights
		if (auto spotLightSystem= m_spotLightSystem.lock())
			spotLightSystem->customRender(graphicsContext, viewportCamera);

		// Draw all the environment lights in the stage
		renderEnvironmentLightComponents(graphicsContext, viewportCamera, stageComponent);

		// Draw the cameras on the stage
		renderCameraComponents(graphicsContext, viewportCamera, stageComponent);

		// Draw the tracking volume for the stage
		renderProjectTracking(graphicsContext, viewportCamera);

		// Draw the state bounds
		stageComponent->renderStageBounds(graphicsContext, glm::mat4(1.f));
	}
}

void AppStage_Project::renderProjectTracking(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const
{
	TrackingVolumeComponentConstPtr trackingVolume= getCurrentTrackingVolumeConst();
	if (trackingVolume)
	{
		switch (trackingVolume->getTrackingVolumeType())
		{
		case eTrackingVolumeType::vr:
			renderVRTrackingVolume(graphicsContext, viewportCamera,
								   std::dynamic_pointer_cast<const VRTrackingVolumeComponent>(trackingVolume));
			break;
		case eTrackingVolumeType::marker:
			renderMarkerTrackingVolume(graphicsContext, viewportCamera,
									   std::dynamic_pointer_cast<const MarkerTrackingVolumeComponent>(trackingVolume));
			break;
		}
	}
}

void AppStage_Project::renderVRTrackingVolume(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
											  VRTrackingVolumeComponentConstPtr vrTrackingVolume) const
{
	VRObjectSystemPtr vrObjectSystem= getObjectSystemOfType<VRObjectSystem>();

	// Resolve VRSpace -> StageSpace offset from the VR tracking volume
	glm::mat4 vrSpaceToStageSpace= vrTrackingVolume->getVRSpaceToStageSpace();

	// Get the tracking volume origin marker (if it exists) so we can render it in the correct space
	const MikanMarkerID originMarkerId= vrTrackingVolume->getOriginMarkerId();
	MarkerComponentPtr markerComp;
	if (originMarkerId != INVALID_MIKAN_ID)
	{
		MarkerObjectSystemPtr markerSystem= getObjectSystemOfType<MarkerObjectSystem>();
		markerComp= markerSystem->getMarkerById(originMarkerId);
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
				glm::mat4 stageSpaceToVRSpace= glm::inverse(vrSpaceToStageSpace);

				markerComp->renderArucoMarker(graphicsContext, viewportCamera, stageSpaceToVRSpace);
			}
		}
		break;
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

void AppStage_Project::renderMarkerTrackingVolume(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
												  MarkerTrackingVolumeComponentConstPtr markerTrackingVolume) const
{
	const MikanMarkerID originMarkerId= markerTrackingVolume->getOriginMarkerId();
	if (originMarkerId != INVALID_MIKAN_ID)
	{
		MarkerObjectSystemPtr markerSystem= getObjectSystemOfType<MarkerObjectSystem>();
		MarkerComponentPtr markerComp= markerSystem->getMarkerById(originMarkerId);
		if (markerComp)
		{
			markerComp->renderArucoMarker(graphicsContext, viewportCamera, glm::mat4(1.f));
		}
	}
}

void AppStage_Project::renderEnvironmentLightComponents(IMkGraphicsContext* graphicsContext,
														MikanCameraPtr viewportCamera,
														StageComponentConstPtr stageComponent) const
{
	if (viewportCamera->getProjectionMode() == eCameraProjectionMode::perspective)
	{
		getObjectSystemOfType<LightEnvironmentSystem>()->customRender(
			graphicsContext, viewportCamera, [stageComponent](LightEnvironmentComponentPtr lightEnv)
			{ return lightEnv->getOwnerStageComponent() == stageComponent; });
	}
}

void AppStage_Project::renderCameraComponents(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
											  StageComponentConstPtr stageComponent) const
{
	getObjectSystemOfType<CameraObjectSystem>()->customRender(
		graphicsContext, viewportCamera,
		[stageComponent](CameraComponentPtr camera) { return camera->getOwnerStageComponent() == stageComponent; });
}

void AppStage_Project::debugRenderOrigin() const
{
	IMkGraphicsContext* graphicsContext= getGraphicsContext();
	TextStyle style= getDefaultTextStyle();

	drawTransformedAxes(graphicsContext, glm::mat4(1.f), 1.f, 1.f, 1.f);
	drawTextAtWorldPosition(graphicsContext, style, glm::vec3(0.f, 0.f, 0.f), L"(0,0,0)");
}

void AppStage_Project::renderCameraAlignmentDebug(IMkGraphicsContext* graphicsContext,
												  MikanCameraPtr viewportCamera) const
{
	TextStyle style= getDefaultTextStyle();

	// Stage origin (identity) — the frame every calibrated transform is expressed relative to
	drawTransformedAxes(graphicsContext, glm::mat4(1.f), 0.25f, true);
	drawTextAtWorldPosition(graphicsContext, style, glm::vec3(0.f), L"stage origin");

	// For each compositor on the current scene, draw its tracking puck + camera aperture in stage
	// space so the pose chain (VR->stage, puck->aperture lever arm) is directly inspectable in 3D.
	SceneComponentConstPtr currentScene= getCurrentSceneConst();
	if (!currentScene)
		return;

	for (const CompositorComponentPtr& compositor : currentScene->getOutputCompositors())
	{
		if (!compositor)
			continue;

		CameraComponentPtr cameraComponent= compositor->getCameraComponent();
		if (!cameraComponent)
			continue;

		glm::mat4 aperturePose(1.f);
		if (!cameraComponent->getStageSpaceAperturePose(aperturePose))
			continue;

		const glm::vec3 aperturePos= glm::vec3(aperturePose[3]);
		drawTransformedAxes(graphicsContext, aperturePose, 0.1f, true);
		drawTextAtWorldPosition(graphicsContext, style, aperturePos, L"aperture");

		// aperturePose = puckStagePose * puckToApertureOffset  =>  puckStagePose = aperturePose * inverse(offset)
		glm::mat4 offsetXform(1.f);
		if (cameraComponent->hasValidApertureOffsetXform() && cameraComponent->getApertureOffsetXform(offsetXform))
		{
			const glm::mat4 puckPose= aperturePose * glm::inverse(offsetXform);
			const glm::vec3 puckPos= glm::vec3(puckPose[3]);

			drawTransformedAxes(graphicsContext, puckPose, 0.1f, true);
			drawTextAtWorldPosition(graphicsContext, style, puckPos, L"puck");

			// Lever arm from tracking puck to camera aperture
			drawSegment(graphicsContext, glm::mat4(1.f), puckPos, aperturePos, glm::vec3(1.f, 0.5f, 0.f));
		}
	}
}

void AppStage_Project::renderCameraAlignmentGui()
{
	const glm::i32vec2 vpOrigin= m_viewport ? m_viewport->getViewportOrigin() : glm::i32vec2(0, 45);
	ImGui::SetNextWindowPos(ImVec2((float)vpOrigin.x + 8, (float)vpOrigin.y + 44), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(340.f, 0.f), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(locWindowTitle("windows.cameraAlignmentDebug")))
	{
		const ImVec4 k_warnColor(1.f, 0.3f, 0.3f, 1.f);
		int compositorCount= 0;

		SceneComponentConstPtr currentScene= getCurrentSceneConst();
		const std::vector<CompositorComponentPtr> compositors=
			currentScene ? currentScene->getOutputCompositors() : std::vector<CompositorComponentPtr>();

		for (const CompositorComponentPtr& compositor : compositors)
		{
			if (!compositor)
				continue;

			CameraComponentPtr cameraComponent= compositor->getCameraComponent();
			VideoSourceComponentPtr videoSource= compositor->getVideoSourceComponent();
			if (!cameraComponent || !videoSource)
				continue;

			ImGui::PushID(compositorCount++);
			ImGui::Separator();
			ImGui::Text("%s", compositor->getName().c_str());

			// Resolution: live video mode vs the resolution the intrinsics were calibrated at.
			// A mismatch silently invalidates the projection (no rescaling is done anywhere).
			int liveWidth= 0, liveHeight= 0;
			videoSource->getVideoPixelDimensions(liveWidth, liveHeight);

			MikanVideoSourceIntrinsics intrinsics;
			videoSource->getCameraIntrinsics(intrinsics);
			if (intrinsics.intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS)
			{
				const MikanMonoIntrinsics& mono= intrinsics.getMonoIntrinsics();
				const int calibWidth= (int)mono.pixel_width;
				const int calibHeight= (int)mono.pixel_height;

				ImGui::Text(locText("project.liveResolutionFmt"), liveWidth, liveHeight);
				ImGui::Text(locText("project.calibResolutionFmt"), calibWidth, calibHeight);
				if (liveWidth != calibWidth || liveHeight != calibHeight)
					ImGui::TextColored(k_warnColor, "%s", locText("project.resolutionMismatch"));

				float fx= 0.f, fy= 0.f, cx= 0.f, cy= 0.f, skew= 0.f;
				extractCameraIntrinsicMatrixParameters(mono.undistorted_camera_matrix, fx, fy, cx, cy, skew);
				ImGui::Text(locText("project.fxFyFmt"), fx, fy);
				ImGui::Text(locText("project.cxCyFmt"), cx, cy);
				ImGui::Text(locText("project.hfovVfovFmt"), (float)mono.hfov, (float)mono.vfov);
				ImGui::Text(locText("project.znearZfarFmt"), (float)mono.znear, (float)mono.zfar);
			}
			else
			{
				ImGui::TextColored(k_warnColor, "%s", locText("project.noMonoIntrinsics"));
			}

			// Tracking + calibrated offsets
			CameraDefinitionPtr cameraDef= cameraComponent->getCameraDefinition();
			ImGui::Text(locText("project.trackingFrameDelayFmt"), cameraDef->getTrackingFrameDelay());
			ImGui::Text(locText("project.puckPoseValidFmt"), cameraComponent->hasValidTrackingMountPoseView()
																 ? locText("project.yes")
																 : locText("project.no"));

			const bool offsetValid= cameraDef->hasValidApertureOffset();
			ImGui::Text(locText("project.apertureOffsetValidFmt"),
						offsetValid ? locText("project.yes") : locText("project.no"));
			if (offsetValid)
			{
				const MikanVector3d p= cameraDef->getApertureOffsetPosition();
				const MikanQuatd q= cameraDef->getApertureOffsetOrientation();
				ImGui::Text(locText("project.aperturePosFmt"), p.x * 1000.0, p.y * 1000.0, p.z * 1000.0);
				ImGui::Text(locText("project.apertureQuatFmt"), q.w, q.x, q.y, q.z);
			}

			ImGui::PopID();
		}

		if (compositorCount == 0)
			ImGui::TextDisabled("%s", locText("project.noActiveCompositorCameras"));
	}
	ImGui::End();
}

// -- IRemoteControllable Interface -- //
bool AppStage_Project::handleRemoteControlCommand(const std::string& command,
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