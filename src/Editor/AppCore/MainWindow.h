#pragma once

//-- includes -----
#include "AppStage.h"
#include "EditorWindow.h"
#include "IMkFontManager.h"
#include "MikanRendererFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"

#include <memory>
#include <string>
#include <vector>

#include <assert.h>

//-- definitions -----
class MainWindow : public EditorWindow
{
public:
	MainWindow(class App* ownerApp);
	~MainWindow();

	// -- IMkWindowEventListener ----
	virtual bool onWindowEvent(const MkWindowEvent& event) override;

	// -- IEditorWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual bool getIsRenderingStage() const override { return m_isRenderingStage; }
	virtual IMkViewportPtr getRenderingViewport() const override;

	virtual class EventBus* getEventBus() const override;
	virtual class LocalizationManager* getLocalizationManager() const override;
	virtual class MikanServer* getMikanServer() const override { return m_mikanServer; }
	virtual class ClientSourceManager* getClientSourceManager() const override { return m_clientSourceManager; }
	virtual class InputManager* getInputManager() const override { return m_inputManager; }
	virtual ProjectManagerPtr getProjectManager() const override { return m_projectManager; }
	virtual class OpenCVManager* getOpenCVManager() const override { return m_openCVManager; }
	virtual class IMkFontManager* getFontManager() const override { return m_fontManager.get(); }

	virtual AppStage* getCurrentAppStage() const override;
	virtual AppStage* getParentAppStage() const override;
	virtual AppStage* pushAppStage(const std::string& appStageName) override;
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

	IMkViewportPtr m_uiViewport;

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
	bool m_bIsMainWindowGuiHidden = false;
};
