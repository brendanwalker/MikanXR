#pragma once

//-- includes -----
#include "AppStage.h"
#include "CommonConfigFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "RmlFwd.h"
#include "SceneFwd.h"
#include "ScriptingFwd.h"

#include <filesystem>
#include <memory>
#include "CompositorConstants.h"

//-- definitions -----
class AppStage_Compositor : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_Compositor(class MainWindow* ownerWindow);
	virtual ~AppStage_Compositor();

	inline CompositorScriptContextPtr getCompositorScriptContext() { return m_scriptContext; }

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
	// TODO: Tracking
	// TODO: Markers
	void onToggleSettingsWindowEvent();
	void hideAllSubWindows();

	// Layers UI Events
	void onScreenshotClientSourceEvent(const std::string& clientSourceName);

	// Debug Rendering
	void debugRenderOrigin() const;

	ProjectConfigPtr m_project;

	AnchorObjectSystemPtr m_anchorObjectSystem;
	StencilObjectSystemPtr m_stencilObjectSystem;
	EditorObjectSystemPtr m_editorSystem;
	SceneObjectSystemPtr m_sceneObjectSystem;
	CompositorObjectSystemPtr m_compositorSystem; 

	class RmlModel_Compositor* m_compositorModel = nullptr;
	Rml::ElementDocument* m_compositiorView = nullptr;

	class RmlModel_CompositorScenes* m_compositorScenesModel = nullptr;
	class RmlModel_CompositorSelection* m_compositorSelectionModel = nullptr;
	Rml::ElementDocument* m_compositiorScenesView = nullptr;

	class RmlModel_CompositorStages* m_compositorStagesModel = nullptr;
	Rml::ElementDocument* m_compositiorStagesView = nullptr;

	class RmlModel_CompositorSources* m_compositorSourcesModel = nullptr;
	Rml::ElementDocument* m_compositiorSourcesView = nullptr;

	// TODO: Tracking
	// TODO: Markers

	class RmlModel_CompositorSettings* m_compositorSettingsModel = nullptr;
	Rml::ElementDocument* m_compositiorSettingsView = nullptr;

	CompositorScriptContextPtr m_scriptContext;

	MikanViewportPtr m_viewport;
	std::vector<CompositorComponentWeakPtr> m_activeCompositors;

	bool m_bAddingNewConfig= false;
};