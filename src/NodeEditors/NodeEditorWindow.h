#pragma once

//-- includes -----
#include "SdlFwd.h"
#include "IGlWindow.h"

#include <memory>

//-- definitions -----
class NodeEditorWindow : public IGlWindow
{
public:
	NodeEditorWindow();
	~NodeEditorWindow();

	// -- IGlWindow ----
	virtual bool startup() override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return false; }
	virtual bool getIsRenderingUI() const override { return m_isRenderingUI; }

	virtual GlViewportConstPtr getRenderingViewport() const override { return nullptr; }
	virtual GlStateStack& getGlStateStack() override;
	virtual GlLineRenderer* getLineRenderer() override;
	virtual GlTextRenderer* getTextRenderer() override;

	virtual bool onSDLEvent(const SDL_Event* event)  override;

protected:
	virtual void configImGui();
	virtual void configImNodes();
	virtual void renderUI();
	virtual void pushImGuiStyles();
	virtual void popImGuiStyles();

private:
	SdlWindowUniquePtr m_sdlWindow;
	GlStateStackUniquePtr m_glStateStack;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont= nullptr;
	struct ImFont* m_BigIconFont= nullptr;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
	bool m_IsPlaying= false;
};
