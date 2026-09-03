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
// What the 3d viewport draws and which object systems are pickable, derived
// from the outliner's selected node kind
enum class eProjectViewMode : int
{
	INVALID= -1,

	scene= 0,
	stage= 1,
	tracking= 2,

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

	virtual bool getUsesDockspace() const override { return true; }
	virtual void onMenuBarGui() override;
	virtual void onBuildDefaultDockLayout(unsigned int dockspaceId) override;

protected:
	SceneComponentConstPtr getCurrentSceneConst() const;
	StageComponentConstPtr getCurrentStageConst() const;
	TrackingVolumeComponentConstPtr getCurrentTrackingVolumeConst() const;

	// Project Rendering
	void renderProjectScene(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderProjectStage(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderProjectTracking(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderEnvironmentLightComponents(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
										  StageComponentConstPtr stageComponent) const;
	void renderCameraComponents(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
								StageComponentConstPtr stageComponent) const;
	void renderVRTrackingVolume(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
								VRTrackingVolumeComponentConstPtr vrTrackingVolume) const;
	void renderMarkerTrackingVolume(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera,
									MarkerTrackingVolumeComponentConstPtr markerTrackingVolume) const;

	// Viewport view mode
	void setViewMode(eProjectViewMode newViewMode);
	void onViewModeChanged();

	// Main Compositor UI Events
	void onReturnEvent();

	// Debug Rendering
	void debugRenderOrigin() const;

	// MR camera-alignment debug overlay (toggled by EditorSettings::bDebugCameraAlignment)
	void renderCameraAlignmentDebug(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const;
	void renderCameraAlignmentGui();
	void applyPendingProjectActions();

	// -- IRemoteControllable Interface -- //
	virtual bool handleRemoteControlCommand(const std::string& command, const std::vector<std::string>& parameters,
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
	LightEnvironmentSystemWeakPtr m_lightEnvironmentSystem;
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
	class GuiPanel_ProjectOutliner* m_projectOutlinerPanel= nullptr;
	class GuiPanel_ProjectSettings* m_projectSettingsPanel= nullptr;
	class GuiPanel_HttpTriggers* m_httpTriggersPanel= nullptr;
	eProjectViewMode m_viewMode= eProjectViewMode::INVALID;
	// Window visibility, driven by the View menu (session-only)
	bool m_bOutlinerVisible= true;
	bool m_bSettingsPanelVisible= true;
	bool m_bHttpTriggersPanelVisible= true;
	bool m_bShowLogPanel= true;
	// Deferred project actions: a menu click must not swap the project out from
	// under the panels drawing this frame
	bool m_bPendingCloseProject= false;
	bool m_bPendingExit= false;
	std::filesystem::path m_pendingLoadProjectPath;
	std::filesystem::path m_pendingNewProjectPath;

	MikanViewportPtr m_viewport;
	IMkScenePtr m_mkScene;

	bool m_bAddingNewConfig= false;
};