#pragma once

//-- includes -----
#include "AppStage.h"
#include "MkGuiFwd.h"
#include "MikanRendererFwd.h"
#include "SdlFwd.h"
#include "IEditorWindow.h"
#include "IMkWindowEventListener.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "SDL_events.h"

#include <memory>
#include <string>
#include <vector>

#include <assert.h>

//-- definitions -----
class MainWindow : public IEditorWindow, public IMkWindowEventListener
{
public:
	MainWindow(class App* ownerApp);
	~MainWindow();

	// -- IMkWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return m_isRenderingStage; }

	virtual IMkViewportPtr getRenderingViewport() const override;
	virtual MkStateStack& getMkStateStack() override;
	virtual IMkLineRenderer* getLineRenderer() override;
	virtual IMkTextRenderer* getTextRenderer() override;
	virtual MikanModelResourceManager* getModelResourceManager() override;
	virtual IMkShaderCache* getShaderCache() override;
	virtual IMkTextureCache* getTextureCache() override;
	virtual SdlWindow& getSdlWindow() override;
	virtual class EventBus* getEventBus() const override;
	virtual class MkGuiStyleManager* getMkGuiStyleManager() const override { return nullptr; }
	virtual class LocalizationManager* getLocalizationManager() const override;

	// -- ISdlEventListener ----
	virtual bool onWindowEvent(const SDL_Event* event) override;

	// -- IEditorWindow ----
	virtual class MikanServer* getMikanServer() const override { return m_mikanServer; }
	virtual class ClientSourceManager* getClientSourceManager() const { return m_clientSourceManager; }
	virtual class InputManager* getInputManager() const override { return m_inputManager; }
	virtual ProjectManagerPtr getProjectManager() const override { return m_projectManager; }
	virtual class OpenCVManager* getOpenCVManager() const override { return m_openCVManager; }
	virtual class MikanFontManager* getFontManager() const override { return m_fontManager; }

	virtual class App* getOwnerApp() const override { return m_ownerApp; }
	virtual AppStage* getCurrentAppStage() const override;
	virtual AppStage* getParentAppStage() const override;
	virtual AppStage* pushAppStage(const std::string& appStageName) override;
	virtual void popAppState() override;

	void processPendingAppStageOps();

protected:
	void renderStageViewport(AppStage* appStage, IMkViewportPtr targetViewport);
	void renderStageUI(AppStage* appStage);

private:
	// App that owns this window
	class App* m_ownerApp = nullptr;

	// Mikan API Server
	class MikanServer* m_mikanServer = nullptr;

	// Client Source Manager
	class ClientSourceManager* m_clientSourceManager = nullptr;

	// Input Manager
	class InputManager* m_inputManager = nullptr;

	// Object System manager
	ProjectManagerPtr m_projectManager;

	// OpenCV management
	class OpenCVManager* m_openCVManager;

	// OpenGL/SDL font/baked text string texture cache
	class MikanFontManager* m_fontManager = nullptr;

	// App Stages
	AppStageFactory m_appStageFactory;
	int m_appStageStackIndex = -1;
	std::vector<AppStagePtr> m_appStageStack;

	SdlWindowUniquePtr m_sdlWindow;
	IMkViewportPtr m_uiViewport;
	IMkViewportPtr m_renderingViewport;

	MkStateStackUniquePtr m_mkStateStack;
	IMkLineRendererPtr m_lineRenderer;
	IMkTextRendererPtr m_textRenderer;
	MikanModelResourceManagerUniquePtr m_modelResourceManager;
	MkGuiContextPtr m_guiContext;
	std::unique_ptr<class MkGuiStyleManager> m_styleManager;

	enum class AppStageOperation : int
	{
		enter,
		exit
	};

	struct PendingAppStageOperation
	{
		AppStagePtr parentAppStage;
		AppStagePtr appStage;
		AppStageOperation op;
	};
	std::vector<PendingAppStageOperation> m_pendingAppStageOps;
	bool bAppStackOperationAllowed = true;

	bool m_isRenderingStage;
	bool m_isRenderingUI;
	bool m_bIsDebugGuiEnabled = false;

	// OpenGL shader program cache
	MikanShaderCacheUniquePtr m_shaderCache;

	// OpenGL texture program cache
	MikanTextureCacheUniquePtr m_textureCache;
};
