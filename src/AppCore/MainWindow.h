#pragma once

//-- includes -----
#include "SdlFwd.h"
#include "IGlWindow.h"

#include <memory>
#include <string>
#include <vector>

//-- definitions -----
class MainWindow : public IGlWindow
{
public:
	MainWindow();
	~MainWindow();

	static MainWindow* getInstance()
	{
		return m_instance;
	}

	// -- IGlWindow ----
	virtual bool startup() override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return m_isRenderingStage; }
	virtual bool getIsRenderingUI() const override { return m_isRenderingUI; }

	virtual GlViewportConstPtr getRenderingViewport() const override;
	virtual GlStateStack& getGlStateStack() override;
	virtual GlLineRenderer* getLineRenderer() override;
	virtual GlTextRenderer* getTextRenderer() override;
	virtual GlModelResourceManager* getModelResourceManager() override;
	virtual GlShaderCache* getShaderCache() override;
	virtual SdlWindow& getSdlWindow() override;

	virtual bool onSDLEvent(const SDL_Event* event)  override;

protected:
	void renderBegin();
	void renderStageBegin(GlViewportConstPtr targetViewport);
	void renderStageEnd();
	void renderUIBegin();
	void renderUIEnd();
	void renderEnd();

private:
	SdlWindowUniquePtr m_sdlWindow;
	GlViewportPtr m_uiViewport;
	GlViewportConstPtr m_renderingViewport;

	GlStateStackUniquePtr m_glStateStack;
	GlLineRendererUniquePtr m_lineRenderer;
	GlTextRendererUniquePtr m_textRenderer;
	GlModelResourceManagerUniquePtr m_modelResourceManager;
	GlRmlUiRenderUniquePtr m_rmlUiRenderer;

	bool m_isRenderingStage;
	bool m_isRenderingUI;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	static MainWindow* m_instance;
};
