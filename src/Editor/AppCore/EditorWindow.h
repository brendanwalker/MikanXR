#pragma once

//-- includes -----
#include "IMkWindow.h"
#include "IEditorWindow.h"
#include "IMkWindowEventListener.h"
#include "MkGuiFwd.h"
#include "MikanRendererFwd.h"
#include "MkWindowFwd.h"

#include <memory>
#include <string>

//-- definitions -----
class EditorWindow : public IEditorWindow, public IMkWindowEventListener
{
public:
	EditorWindow(class App* ownerApp);
	virtual ~EditorWindow();

	// -- IEditorWindow ----
	//virtual bool startup() = 0;
	//virtual void update(float deltaSeconds) = 0;
	//virtual void render() = 0;
	//virtual void shutdown() = 0;

	//virtual bool getIsRenderingStage() const = 0;
	//virtual IMkViewportPtr getRenderingViewport() const = 0;
	virtual const char* getTitle() const override;
	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual void getMouseScreenPosition(int& outScreenX, int& outScreenY) const override;

	virtual MikanModelResourceManager* getModelResourceManager() override;
	//virtual ProjectManagerPtr getProjectManager() const = 0;
	//virtual class MikanServer* getMikanServer() const = 0;
	//virtual class IMkFontManager* getFontManager() const = 0;
	//virtual class InputManager* getInputManager() const = 0;
	//virtual class OpenCVManager* getOpenCVManager() const = 0;
	//virtual class ClientSourceManager* getClientSourceManager() const = 0;
	//virtual class LocalizationManager* getLocalizationManager() const = 0;
	//virtual class EventBus* getEventBus() const = 0;
	virtual class MkGuiStyleManager* getMkGuiStyleManager() const override;

	virtual IMkGraphicsContextPtr getGraphicsContext() const override;
	virtual IMkWindowPtr getMkWindowContext() const override;
	virtual class App* getOwnerApp() const override;
	//virtual class AppStage* getCurrentAppStage() const = 0;
	//virtual class AppStage* getParentAppStage() const = 0;
	//virtual class AppStage* pushAppStage(const std::string& appStageName) = 0;
	//virtual void popAppState() = 0;

	// -- IMkWindowContext Helpers ----
	eWindowAPI getWindowAPI() const;
	void* getNativeWindowHandle() const;
	void makeContextCurrent();
	bool wantsDestroy() const;
	void present();
	void setTitle(const std::string& title);
	void setSize(int width, int height);
	void handleEvents(class IMkWindowEventListener* eventListener);
	bool hasMouseFocus() const;
	bool hasKeyboardFocus() const;

protected:
	bool startupWindow(const std::string& title, int width, int height);
	bool startupGuiContext();
	bool startupStyleManager();
	bool startupModelResourceManager();

	void shutdownModelResourceManager();
	void shutdownStyleManager();
	void shutdownGuiContext();
	void shutdownWindow();

	class App* m_ownerApp = nullptr;
	IMkWindowPtr m_mkWindowContext;
	IMkGraphicsContextPtr m_graphicsContext;
	MkGuiContextPtr m_guiContext;
	std::unique_ptr<class MkGuiStyleManager> m_styleManager;
	MikanModelResourceManagerUniquePtr m_modelResourceManager;
};
