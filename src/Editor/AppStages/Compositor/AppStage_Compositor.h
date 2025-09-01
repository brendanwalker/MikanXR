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
	//bool startStreaming();
	//bool getIsStreaming();
	//void stopStreaming();

	// Camera
	void createCompositorViewportCameras();
	void disposeCompositorViewportCameras();
	void updateCompositorCameras();
	void cyclePreviousCompositorCamera();
	void cycleNextCompositorCamera();

	// Scene
	void onSceneDeactivated(SceneComponentPtr oldScene);
	void onSceneActivated(SceneComponentPtr newScene);

	// Compositor Events
	//void onCompositorDeactivated(CompositorComponentPtr oldCompositor);
	//void onCompositorActivated(CompositorComponentPtr newCompositor);
	//void onNewStreamingFrameReady();

	// Project Config Events
	//void onProjectConfigMarkedDirty(
	//	CommonConfigPtr configPtr, 
	//	const class ConfigPropertyChangeSet& changedPropertySet);
	//void onSpoutOutputNameChanged();
	//void onSpoutStreamingFlagChanged();

	// Main Compositor UI Events
	void onReturnEvent();
	void onToggleCamerasWindowEvent();
	void onToggleSourcesEvent();
	void onToggleSettingsWindowEvent();
	// Deprecated
	void onToggleOutlinerWindowEvent();
	void onToggleLayersWindowEvent();
	void onToggleScriptingWindowEvent();
	// Deprecated
	void hideAllSubWindows();

	// Layers UI Events
	//void onGraphEditEvent();
	//void onGraphFileSelectEvent();
	void onScreenshotClientSourceEvent(const std::string& clientSourceName);

	// Scripting UI Events
	void onScriptFileChangeEvent(const std::filesystem::path& scriptFileChangeEvent);
	void onSelectCompositorScriptFileEvent();
	void onReloadCompositorScriptFileEvent();
	void onInvokeScriptTriggerEvent(const std::string& triggerEvent);

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

	class RmlModel_CompositorLayers* m_compositorLayersModel = nullptr;
	Rml::ElementDocument* m_compositiorLayersView = nullptr;
	
	class RmlModel_CompositorCameras* m_compositorCamerasModel = nullptr;
	Rml::ElementDocument* m_compositiorCamerasView = nullptr;

	class RmlModel_CompositorSources* m_compositorSourcesModel = nullptr;
	Rml::ElementDocument* m_compositiorSourcesView = nullptr;

	class RmlModel_CompositorScripting* m_compositorScriptingModel = nullptr;
	Rml::ElementDocument* m_compositiorScriptingView = nullptr;

	class RmlModel_CompositorOutliner* m_compositorOutlinerModel = nullptr;
	class RmlModel_CompositorSelection* m_compositorSelectionModel = nullptr;
	Rml::ElementDocument* m_compositiorOutlinerView = nullptr;

	class RmlModel_CompositorSettings* m_compositorSettingsModel = nullptr;
	Rml::ElementDocument* m_compositiorSettingsView = nullptr;

	CompositorScriptContextPtr m_scriptContext;

	MikanViewportPtr m_viewport;
	std::vector<CompositorComponentWeakPtr> m_activeCompositors;

	bool m_bAddingNewConfig= false;
};