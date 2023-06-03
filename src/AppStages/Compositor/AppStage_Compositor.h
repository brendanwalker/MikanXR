#pragma once

//-- includes -----
#include "AppStage.h"
#include "RendererFwd.h"
#include "RmlFwd.h"
#include "SceneFwd.h"
#include "ScriptingFwd.h"

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

	AppStage_Compositor(class App* app);
	virtual ~AppStage_Compositor();

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	bool startRecording();
	void stopRecording();

	// Camera
	void setupCameras();
	void setCurrentCameraMode(eCompositorViewpointMode viewportMode);
	eCompositorViewpointMode getCurrentCameraMode() const;
	GlCameraPtr getViewpointCamera(eCompositorViewpointMode viewportMode) const;
	void updateCamera();
	void setXRCamera();
	void setVRCamera();

	// Compositor Events
	void onCompositorShadersReloaded();
	void onNewFrameComposited();

	// Main Compositor UI Events
	void onReturnEvent();
	void onToggleOutlinerWindowEvent();
	void onToggleLayersWindowEvent();
	void onToggleAnchorsWindowEvent();
	void onToggleRecordingWindowEvent();
	void onToggleScriptingWindowEvent();
	void onToggleQuadStencilsWindowEvent();
	void onToggleBoxStencilsWindowEvent();
	void onToggleModelStencilsWindowEvent();
	void onToggleSourcesWindowEvent();
	void hideAllSubWindows();

	// Layers UI Events
	void onConfigAddEvent();
	void onConfigDeleteEvent();
	void onConfigNameChangeEvent(const std::string& newConfigName);
	void onConfigSelectEvent(const std::string& configName);
	void onLayerAddEvent();
	void onLayerDeleteEvent(const int layerIndex);
	void onMaterialNameChangeEvent(const int layerIndex, const std::string& materialName);
	void onVerticalFlipChangedEvent(const int layerIndex, bool bIsFlipped);
	void onBlendModeChangedEvent(const int layerIndex, eCompositorBlendMode blendMode);
	void onInvertQuadsFlagChangeEvent(const int layerIndex, bool bInvertFlag);
	void onQuadStencilModeChangeEvent(const int layerIndex, eCompositorStencilMode stencilMode);
	void onBoxStencilModeChangeEvent(const int layerIndex, eCompositorStencilMode stencilMode);
	void onModelStencilModeChangeEvent(const int layerIndex, eCompositorStencilMode stencilMode);
	void onStencilRefAddedEvent(const int layerIndex, int stencilId);
	void onStencilRefRemovedEvent(const int layerIndex, int stencilId);
	void onFloatMappingChangedEvent(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void onFloat2MappingChangedEvent(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void onFloat3MappingChangedEvent(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void onFloat4MappingChangedEvent(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void onMat4MappingChangedEvent(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void onColorTextureMappingChangedEvent(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void onScreenshotClientSourceEvent(const std::string& clientSourceName);

	// Quad Stencils UI Events
	void onAddQuadStencilEvent();
	void onDeleteQuadStencilEvent(int stencilID);

	// Box Stencils UI Events
	void onAddBoxStencilEvent();
	void onDeleteBoxStencilEvent(int stencilID);

	// Model Stencils UI Events
	void onAddModelStencilEvent();
	void onDeleteModelStencilEvent(int stencilID);

	// Recording UI Events
	void onToggleRecordingEvent();

	// Scripting UI Events
	void onScriptFileChangeEvent(const std::filesystem::path& scriptFileChangeEvent);
	void onSelectCompositorScriptFileEvent();
	void onReloadCompositorScriptFileEvent();
	void onInvokeScriptTriggerEvent(const std::string& triggerEvent);

	// Debug Rendering
	void debugRenderOrigin() const;
	void debugRenderStencils() const;
	void debugRenderAnchors() const;

	ProfileConfigPtr m_profile;

	AnchorObjectSystemPtr m_anchorObjectSystem;
	StencilObjectSystemPtr m_stencilObjectSystem;
	EditorObjectSystemPtr m_editorSystem;

	class RmlModel_Compositor* m_compositorModel = nullptr;
	Rml::ElementDocument* m_compositiorView = nullptr;

	class RmlModel_CompositorLayers* m_compositorLayersModel = nullptr;
	Rml::ElementDocument* m_compositiorLayersView = nullptr;

	class RmlModel_CompositorAnchors* m_compositorAnchorsModel = nullptr;
	Rml::ElementDocument* m_compositiorAnchorsView = nullptr;

	class RmlModel_CompositorQuads* m_compositorQuadsModel = nullptr;
	Rml::ElementDocument* m_compositiorQuadsView = nullptr;

	class RmlModel_CompositorBoxes* m_compositorBoxesModel = nullptr;
	Rml::ElementDocument* m_compositiorBoxesView = nullptr;

	class RmlModel_CompositorModels* m_compositorModelsModel = nullptr;
	Rml::ElementDocument* m_compositiorModelsView = nullptr;

	class RmlModel_CompositorRecording* m_compositorRecordingModel = nullptr;
	Rml::ElementDocument* m_compositiorRecordingView = nullptr;

	class RmlModel_CompositorScripting* m_compositorScriptingModel = nullptr;
	Rml::ElementDocument* m_compositiorScriptingView = nullptr;

	class RmlModel_CompositorSources* m_compositorSourcesModel = nullptr;
	Rml::ElementDocument* m_compositiorSourcesView = nullptr;

	class RmlModel_CompositorOutliner* m_compositorOutlinerModel = nullptr;
	class RmlModel_CompositorSelection* m_compositorSelectionModel = nullptr;
	Rml::ElementDocument* m_compositiorOutlinerView = nullptr;

	CompositorScriptContextPtr m_scriptContext;
	class GlFrameCompositor* m_frameCompositor= nullptr;

	GlViewportPtr m_viewport;
	eCompositorViewpointMode m_viewportMode= eCompositorViewpointMode::mixedRealityViewpoint;

	bool m_bAddingNewConfig= false;

	class VideoWriter* m_videoWriter= nullptr;
};