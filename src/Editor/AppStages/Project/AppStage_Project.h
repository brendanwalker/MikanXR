#pragma once

//-- includes -----
#include "AppStage.h"
#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "RmlFwd.h"
#include "SceneFwd.h"

#include <filesystem>
#include <memory>
#include "CompositorConstants.h"

//-- definitions -----
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
	virtual void render(IMkViewportPtr targetViewport) override;

protected:
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
	void onToggleScenesWindowEvent();
	void onToggleStagesWindowEvent();
	void onToggleSourcesEvent();
	void onToggleTrackingEvent();
	void onToggleMarkersEvent();
	void onToggleSettingsWindowEvent();
	void hideAllSubWindows();

	// Layers UI Events
	void onScreenshotClientSourceEvent(const std::string& clientSourceName);

	// Marker UI Events
	void onMarkerSelected(int arucoId);

	// Debug Rendering
	void debugRenderOrigin() const;

	ProjectConfigPtr m_project;

	AnchorObjectSystemWeakPtr m_anchorObjectSystem;
	CameraObjectSystemWeakPtr m_cameraSystem;
	CompositorObjectSystemWeakPtr m_compositorSystem;
	EditorObjectSystemWeakPtr m_editorSystem;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	StencilObjectSystemWeakPtr m_stencilObjectSystem;
	SceneObjectSystemWeakPtr m_sceneObjectSystem;
	StageObjectSystemWeakPtr m_stageSystem;
	TextureSourceSystemWeakPtr m_textureSourceSystem;
	TrackingVolumeObjectSystemWeakPtr m_trackingVolumeSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountSystem;
	VideoSourceSystemWeakPtr m_videoObjectSystem;

	// Shared context for component and object system models used in project panels
	class ProjectRmlModelContext* m_projectRmlModelContext = nullptr;

	class RmlModel_Project* m_projectModel= nullptr;

	class RmlModel_ProjectScenes* m_projectScenesModel;
	Rml::ElementDocument* m_projectScenesView = nullptr;

	class RmlModel_ProjectStages* m_projectStagesModel;
	Rml::ElementDocument* m_projectStagesView = nullptr;

	class RmlModel_ProjectSources* m_projectSourcesModel;
	Rml::ElementDocument* m_projectSourcesView = nullptr;

	class RmlModel_ProjectTracking* m_projectTrackingModel;
	Rml::ElementDocument* m_projectTrackingView = nullptr;

	class RmlModel_ProjectMarkers* m_projectMarkersModel;
	Rml::ElementDocument* m_projectMarkersView = nullptr;

	class RmlModel_ProjectSettings* m_projectSettingsModel;
	Rml::ElementDocument* m_projectSettingsView = nullptr;

	MikanViewportPtr m_viewport;
	std::vector<CompositorComponentWeakPtr> m_activeCompositors;

	bool m_bAddingNewConfig= false;
};