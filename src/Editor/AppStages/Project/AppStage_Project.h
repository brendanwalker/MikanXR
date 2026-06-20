#pragma once

//-- includes -----
#include "AppStage.h"
#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "LightSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "SceneFwd.h"

#include <filesystem>
#include <memory>
#include "CompositorConstants.h"

//-- definitions -----
enum class eProjectAppStageActivePanel : int
{
	INVALID= -1,

	Scenes= 0,
	Stages= 1,
	Sources= 2,
	Tracking= 3,
	Markers= 4,
	Settings= 5,

	COUNT
};

class AppStage_Project : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_Project(class IEditorWindow* ownerWindow);
	virtual ~AppStage_Project();

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
	SceneComponentConstPtr getCurrentSceneConst() const;
	StageComponentConstPtr getCurrentStageConst() const;
	TrackingVolumeComponentConstPtr getCurrentTrackingVolumeConst() const;

	// Project Rendering
	void renderProjectScene(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderProjectStage(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderProjectTracking(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderCameraComponents(
		IMkGraphicsContext* graphicsContext,
		MikanCameraPtr viewportCamera,
		StageComponentConstPtr stageComponent) const;
	void renderVRTrackingVolume(
		IMkGraphicsContext* graphicsContext,
		MikanCameraPtr viewportCamera,
		VRTrackingVolumeComponentConstPtr vrTrackingVolume) const;
	void renderMarkerTrackingVolume(
		IMkGraphicsContext* graphicsContext,
		MikanCameraPtr viewportCamera,
		MarkerTrackingVolumeComponentConstPtr markerTrackingVolume) const;

	// Panel Selection
	void setActivePanel(eProjectAppStageActivePanel newPanel);
	void onActivePanelChanged();

	// Camera
	void createCompositorViewportCameras();
	void disposeCompositorViewportCameras();
	void updateCompositorCameras();
	void cyclePreviousCompositorCamera();
	void cycleNextCompositorCamera();

	// Scene
	void onSceneDeactivated(SceneComponentPtr oldScene);
	void onSceneActivated(SceneComponentPtr newScene);

	// Main Compositor UI Events
	void onReturnEvent();

	// Marker UI Events
	void onMarkerSelected(int arucoId);

	// Debug Rendering
	void debugRenderOrigin() const;

	// -- IRemoteControllable Interface -- //
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults) override;

protected:
	ProjectConfigPtr m_project;

	EditorObjectSystemWeakPtr m_editorSystem;
	SceneObjectSystemWeakPtr m_sceneObjectSystem;

	// Systems with object that care about depending on active panel
	AnchorObjectSystemWeakPtr m_anchorObjectSystem;
	CameraObjectSystemWeakPtr m_cameraObjectSystem;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	QuadStencilSystemWeakPtr m_quadStencilSystem;
	BoxStencilSystemWeakPtr m_boxStencilSystem;
	ModelStencilSystemWeakPtr m_modelStencilSystem;
	QuadShapeSystemWeakPtr m_quadShapeSystem;
	BoxShapeSystemWeakPtr m_boxShapeSystem;
	ModelShapeSystemWeakPtr m_modelShapeSystem;
	RGBPixelGridSystemWeakPtr m_pixelGridLightSystem;
	RGBSpotLightSystemWeakPtr m_spotLightSystem;

	// Collision Systems Filters
	std::set<const MikanObjectSystem*> m_sceneObjectSystemFilter;
	std::set<const MikanObjectSystem*> m_stageObjectSystemFilter;
	std::set<const MikanObjectSystem*> m_emptyObjectSystemFilter;

	// Shared context for GuiPanel component/system panels
	class ProjectGuiPanelContext* m_projectGuiPanelContext= nullptr;

	// Project-level ImGui panels
	class GuiPanel_ProjectScenes* m_projectScenesPanel= nullptr;
	class GuiPanel_ProjectStages* m_projectStagesPanel= nullptr;
	class GuiPanel_ProjectSources* m_projectSourcesPanel= nullptr;
	class GuiPanel_ProjectTracking* m_projectTrackingPanel= nullptr;
	class GuiPanel_ProjectMarkers* m_projectMarkersPanel= nullptr;
	class GuiPanel_ProjectSettings* m_projectSettingsPanel= nullptr;
	eProjectAppStageActivePanel m_activePanel= eProjectAppStageActivePanel::INVALID;

	MikanViewportPtr m_viewport;
	std::vector<CompositorComponentWeakPtr> m_activeCompositors;
	IMkScenePtr m_mkScene;

	bool m_bAddingNewConfig= false;
};