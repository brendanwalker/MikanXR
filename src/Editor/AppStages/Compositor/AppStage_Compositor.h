#pragma once

//-- includes -----
#include "AppStage.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "RmlFwd.h"
#include "SceneFwd.h"
#include "ScriptingFwd.h"
#include "SharedTextureFwd.h"

#include <filesystem>
#include <memory>
#include "FrameCompositorConstants.h"

enum class eCompositorViewpointMode : int
{
	INVALID = -1,

	mixedRealityViewpoint,
	vrViewpoint,

	COUNT
};

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
	virtual void render() override;

protected:
	bool startStreaming();
	void stopStreaming();

	// Camera
	void setupCameras();
	void setCurrentCameraMode(eCompositorViewpointMode viewportMode);
	eCompositorViewpointMode getCurrentCameraMode() const;
	MikanCameraPtr getViewpointCamera(eCompositorViewpointMode viewportMode) const;
	void updateCamera();
	void setXRCamera();
	void setVRCamera();

	// Compositor Events
	void onNewStreamingFrameReady();

	// Main Compositor UI Events
	void onReturnEvent();
	void onToggleOutlinerWindowEvent();
	void onToggleLayersWindowEvent();
	void onToggleVideoWindowEvent();
	void onToggleScriptingWindowEvent();
	void onToggleSettingsWindowEvent();
	void hideAllSubWindows();

	// Layers UI Events
	void onGraphEditEvent();
	void onGraphFileSelectEvent();
	void onConfigAddEvent();
	void onConfigDeleteEvent();
	void onConfigNameChangeEvent(const std::string& newConfigName);
	void onConfigSelectEvent(const std::string& configName);
	void onScreenshotClientSourceEvent(const std::string& clientSourceName);

	// Recording UI Events
	void onToggleRecordingEvent();
	void onToggleStreamingEvent();

	// Scripting UI Events
	void onScriptFileChangeEvent(const std::filesystem::path& scriptFileChangeEvent);
	void onSelectCompositorScriptFileEvent();
	void onReloadCompositorScriptFileEvent();
	void onInvokeScriptTriggerEvent(const std::string& triggerEvent);

	// Debug Rendering
	void debugRenderOrigin() const;

	ProfileConfigPtr m_profile;

	AnchorObjectSystemPtr m_anchorObjectSystem;
	StencilObjectSystemPtr m_stencilObjectSystem;
	EditorObjectSystemPtr m_editorSystem;

	class RmlModel_Compositor* m_compositorModel = nullptr;
	Rml::ElementDocument* m_compositiorView = nullptr;

	class RmlModel_CompositorLayers* m_compositorLayersModel = nullptr;
	Rml::ElementDocument* m_compositiorLayersView = nullptr;

	class RmlModel_CompositorVideo* m_compositorVideoModel = nullptr;
	Rml::ElementDocument* m_compositiorVideoView = nullptr;

	class RmlModel_CompositorScripting* m_compositorScriptingModel = nullptr;
	Rml::ElementDocument* m_compositiorScriptingView = nullptr;

	class RmlModel_CompositorOutliner* m_compositorOutlinerModel = nullptr;
	class RmlModel_CompositorSelection* m_compositorSelectionModel = nullptr;
	Rml::ElementDocument* m_compositiorOutlinerView = nullptr;

	class RmlModel_CompositorSettings* m_compositorSettingsModel = nullptr;
	Rml::ElementDocument* m_compositiorSettingsView = nullptr;

	CompositorScriptContextPtr m_scriptContext;
	class GlFrameCompositor* m_frameCompositor= nullptr;

	MikanViewportPtr m_viewport;
	eCompositorViewpointMode m_viewportMode= eCompositorViewpointMode::mixedRealityViewpoint;

	bool m_bAddingNewConfig= false;

	ISharedTextureWriteAccessorPtr m_renderTargetWriteAccessor;
};