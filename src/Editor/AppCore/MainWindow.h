#pragma once

//-- includes -----
#include "AppStage.h"
#include "MikanRendererFwd.h"
#include "SdlFwd.h"
#include "IEditorWindow.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "SDL_events.h"

#include <memory>
#include <string>
#include <vector>

#include <assert.h>

//-- definitions -----
class MainWindow : public IEditorWindow
{
public:
	MainWindow();
	~MainWindow();

	static MainWindow* getInstance()
	{
		return m_instance;
	}

	// -- IMkWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return m_isRenderingStage; }
	virtual bool getIsRenderingUI() const override { return m_isRenderingUI; }

	virtual IMkViewportPtr getRenderingViewport() const override;
	virtual MkStateStack& getMkStateStack() override;
	virtual IMkLineRenderer* getLineRenderer() override;
	virtual IMkTextRenderer* getTextRenderer() override;
	virtual MikanModelResourceManager* getModelResourceManager() override;
	virtual IMkShaderCache* getShaderCache() override;
	virtual IMkTextureCache* getTextureCache() override;
	virtual SdlWindow& getSdlWindow() override;

	virtual bool onSDLEvent(const SDL_Event* event) override;

	// -- IEditorWindow ----
	virtual ObjectSystemManagerPtr getObjectSystemManager() const override { return m_objectSystemManager; }
	virtual class MikanServer* getMikanServer() const override { return m_mikanServer; }
	virtual class MikanFontManager* getFontManager() const override { return m_fontManager; }
	virtual class RmlManager* getRmlManager() const override { return m_rmlManager; }
	virtual class GlRmlUiRender* getRmlUiRenderer() const override { return m_rmlUiRenderer.get(); }
	virtual class InputManager* getInputManager() const override { return m_inputManager; }
	virtual class OpenCVManager* getOpenCVManager() const override { return m_openCVManager; }

	virtual AppStage* getCurrentAppStage() const override;
	virtual AppStage* getParentAppStage() const override;
	virtual void pushAppStage(AppStage* appStage) override;
	virtual void popAppState() override;

	void processPendingAppStageOps();

protected:
	void renderStageViewport(AppStage* appStage, IMkViewportPtr targetViewport);
	void renderStageUI(AppStage* appStage);

private:
	// Mikan API Server
	class MikanServer* m_mikanServer = nullptr;

	// Client Source Manager
	class ClientSourceManager* m_clientSourceManager = nullptr;

	// Input Manager
	class InputManager* m_inputManager = nullptr;

	// Rml UI Manager
	class RmlManager* m_rmlManager = nullptr;

	// Object System manager
	ObjectSystemManagerPtr m_objectSystemManager;

	// OpenCV management
	class OpenCVManager* m_openCVManager;

	// OpenGL/SDL font/baked text string texture cache
	class MikanFontManager* m_fontManager = nullptr;

	// App Stages
	int m_appStageStackIndex = -1;
	std::vector<AppStage*> m_appStageStack;

	SdlWindowUniquePtr m_sdlWindow;
	IMkViewportPtr m_uiViewport;
	IMkViewportPtr m_renderingViewport;

	MkStateStackUniquePtr m_MkStateStack;
	IMkLineRendererPtr m_lineRenderer;
	IMkTextRendererPtr m_textRenderer;
	MikanModelResourceManagerUniquePtr m_modelResourceManager;
	GlRmlUiRenderUniquePtr m_rmlUiRenderer;

	enum class AppStageOperation : int
	{
		enter,
		exit
	};

	struct PendingAppStageOperation
	{
		AppStage* parentAppStage;
		AppStage* appStage;
		AppStageOperation op;
	};
	std::vector<PendingAppStageOperation> m_pendingAppStageOps;
	bool bAppStackOperationAllowed = true;

	bool m_isRenderingStage;
	bool m_isRenderingUI;

	// OpenGL shader program cache
	MikanShaderCacheUniquePtr m_shaderCache;

	// OpenGL texture program cache
	MikanTextureCacheUniquePtr m_textureCache;

	static MainWindow* m_instance;
};
