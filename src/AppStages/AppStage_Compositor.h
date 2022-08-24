#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>

namespace ImGui
{
	class FileBrowser;
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
	virtual void renderUI() override;

protected:
	bool startRecording();
	void stopRecording();
	void onNewFrameComposited();

	void debugRenderOrigin() const;
	void debugRenderStencils() const;
	void debugRenderAnchors() const;

	class CompositorScriptContext* m_scriptContext= nullptr;
	class GlFrameCompositor* m_frameCompositor= nullptr;
	class GlCamera* m_camera= nullptr;

	bool m_bDebugRenderOrigin = true;
	bool m_bDebugRenderStencils = true;
	bool m_bDebugRenderAnchors = true;

	class VideoWriter* m_videoWriter= nullptr;
	int m_videoCodecIndex= 0;
	bool m_bIsRecording= false;

	ImGui::FileBrowser* m_modelFileDialog = nullptr;
	int m_pendingModelFilenameStencilID= -1;

	ImGui::FileBrowser* m_scriptFileDialog = nullptr;
};