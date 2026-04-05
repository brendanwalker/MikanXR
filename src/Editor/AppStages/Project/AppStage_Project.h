#pragma once

//-- includes -----
#include "AppStage.h"
#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
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
	virtual void onGui() override;
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

	// Marker UI Events
	void onMarkerSelected(int arucoId);

	// Debug Rendering
	void debugRenderOrigin() const;

	// -- IRemoteControllable Interface -- //
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults) override;

	ProjectConfigPtr m_project;

	EditorObjectSystemWeakPtr m_editorSystem;
	SceneObjectSystemWeakPtr m_sceneObjectSystem;

	// Shared context for GuiPanel component/system panels
	class ProjectGuiPanelContext* m_projectGuiPanelContext = nullptr;

	// Project-level ImGui panels
	class GuiPanel_ProjectScenes* m_projectScenesPanel = nullptr;
	class GuiPanel_ProjectStages* m_projectStagesPanel = nullptr;
	class GuiPanel_ProjectSources* m_projectSourcesPanel = nullptr;
	class GuiPanel_ProjectTracking* m_projectTrackingPanel = nullptr;
	class GuiPanel_ProjectMarkers* m_projectMarkersPanel = nullptr;
	class GuiPanel_ProjectSettings* m_projectSettingsPanel = nullptr;

	MikanViewportPtr m_viewport;
	std::vector<CompositorComponentWeakPtr> m_activeCompositors;

	bool m_bAddingNewConfig= false;
};