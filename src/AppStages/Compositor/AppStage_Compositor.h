#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>
#include "FrameCompositorConstants.h"

//namespace ImGui
//{
//	class FileBrowser;
//};
namespace Rml
{
	class ElementDocument;
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
	virtual void update() override;
	virtual void render() override;
	//virtual void renderUI() override;

protected:
	bool startRecording();
	void stopRecording();
	void onNewFrameComposited();

	// Main Compositor UI Events
	void onReturnEvent();
	void onToggleLayersWindowEvent();
	void onToggleRecordingWindowEvent();
	void onToggleScriptingWindowEvent();
	void onToggleQuadStencilsWindowEvent();
	void onToggleBoxStencilsWindowEvent();
	void onToggleModelStencilsWindowEvent();

	// Layers UI Events
	void onLayerAlphaModeChangedEvent(int layerIndex, eCompositorLayerAlphaMode alphaMode);
	void onScreenshotLayerEvent(int layerIndex);

	// Quad Stencils UI Events
	void onAddQuadStencilEvent();
	void onDeleteQuadStencilEvent(int stencilID);
	void onModifyQuadStencilEvent(int stencilID);

	// Recording UI Events
	void onToggleRecordingEvent();
	void onVideoCodecChangedEvent(eSupportedCodec codec);

	// Scripting UI Events
	void onSelectCompositorScriptFileEvent();
	void onReloadCompositorScriptFileEvent();
	void onInvokeScriptTriggerEvent(const std::string& triggerEvent);

	void debugRenderOrigin() const;
	void debugRenderStencils() const;
	void debugRenderAnchors() const;

	class ProfileConfig* m_profile = nullptr;

	class RmlModel_Compositor* m_compositorModel = nullptr;
	Rml::ElementDocument* m_compositiorView = nullptr;

	class RmlModel_CompositorLayers* m_compositorLayersModel = nullptr;
	Rml::ElementDocument* m_compositiorLayersView = nullptr;

	class RmlModel_CompositorQuads* m_compositorQuadsModel = nullptr;
	Rml::ElementDocument* m_compositiorQuadsView = nullptr;

	class RmlModel_CompositorRecording* m_compositorRecordingModel = nullptr;
	Rml::ElementDocument* m_compositiorRecordingView = nullptr;

	class RmlModel_CompositorScripting* m_compositorScriptingModel = nullptr;
	Rml::ElementDocument* m_compositiorScriptingView = nullptr;

	class CompositorScriptContext* m_scriptContext= nullptr;
	class GlFrameCompositor* m_frameCompositor= nullptr;
	class GlCamera* m_camera= nullptr;

	bool m_bDebugRenderOrigin = true;
	bool m_bDebugRenderStencils = true;
	bool m_bDebugRenderAnchors = true;

	class VideoWriter* m_videoWriter= nullptr;
	int m_videoCodecIndex= 0;
	bool m_bIsRecording= false;

	//ImGui::FileBrowser* m_modelFileDialog = nullptr;
	//int m_pendingModelFilenameStencilID= -1;

	//ImGui::FileBrowser* m_scriptFileDialog = nullptr;
};