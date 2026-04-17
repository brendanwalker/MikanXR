#pragma once

//-- includes -----
#include "AppStage.h"
#include "IMkFontManager.h"
#include "MkGuiFwd.h"
#include "MikanRendererFwd.h"
#include "IEditorWindow.h"
#include "IMkWindowEventListener.h"
#include "MkWindowFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"

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
	virtual void present() override;
	virtual void shutdown() override;

	virtual const char* getTitle() const override;
	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return m_isRenderingStage; }

	virtual void getMouseScreenPosition(int& outScreenX, int& outScreenY) const override;

	virtual eWindowAPI getWindowAPI() const override;
	virtual void* getNativeWindowHandle() const override;
	virtual IMkGraphicsContextPtr getGraphicsContext() const override;
	virtual IMkViewportPtr getRenderingViewport() const override;
	virtual void makeContextCurrent() override;
	virtual bool wantsDestroy() const override;
	virtual void setTitle(const std::string& title) override;
	virtual void setSize(int width, int height) override;
	virtual void handleEvents(class IMkWindowEventListener* eventListener) override;
	virtual bool hasMouseFocus() const override;
	virtual bool hasKeyboardFocus() const override;

	virtual MikanModelResourceManager* getModelResourceManager() override;
	virtual class EventBus* getEventBus() const override;
	virtual class MkGuiStyleManager* getMkGuiStyleManager() const override;
	virtual class LocalizationManager* getLocalizationManager() const override;

	// -- IMkWindowEventListener ----
	virtual bool onWindowEvent(const MkWindowEvent& event) override;

	// -- IEditorWindow ----
	virtual class MikanServer* getMikanServer() const override { return m_mikanServer; }
	virtual class ClientSourceManager* getClientSourceManager() const { return m_clientSourceManager; }
	virtual class InputManager* getInputManager() const override { return m_inputManager; }
	virtual ProjectManagerPtr getProjectManager() const override { return m_projectManager; }
	virtual class OpenCVManager* getOpenCVManager() const override { return m_openCVManager; }
	virtual class IMkFontManager* getFontManager() const override { return m_fontManager.get(); }

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
	IMkFontManagerPtr m_fontManager;

	// App Stages
	AppStageFactory m_appStageFactory;
	int m_appStageStackIndex = -1;
	std::vector<AppStagePtr> m_appStageStack;

	IMkWindowPtr m_mkWindow;
	IMkGraphicsContextPtr m_graphicsContext;
	IMkViewportPtr m_uiViewport;
	IMkViewportPtr m_renderingViewport;

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
};
