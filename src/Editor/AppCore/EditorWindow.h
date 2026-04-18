#pragma once

//-- includes -----
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

	// -- IMkWindow ----
	virtual const char* getTitle() const override;
	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;

	virtual void getMouseScreenPosition(int& outScreenX, int& outScreenY) const override;

	virtual eWindowAPI getWindowAPI() const override;
	virtual void* getNativeWindowHandle() const override;
	virtual IMkGraphicsContextPtr getGraphicsContext() const override;
	virtual void makeContextCurrent() override;
	virtual bool wantsDestroy() const override;
	virtual void present() override;
	virtual void setTitle(const std::string& title) override;
	virtual void setSize(int width, int height) override;
	virtual void handleEvents(class IMkWindowEventListener* eventListener) override;
	virtual bool hasMouseFocus() const override;
	virtual bool hasKeyboardFocus() const override;

	// -- IEditorWindow ----
	virtual MikanModelResourceManager* getModelResourceManager() override;
	virtual class MkGuiStyleManager* getMkGuiStyleManager() const override;
	virtual class App* getOwnerApp() const override;

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
	IMkWindowPtr m_mkWindow;
	IMkGraphicsContextPtr m_graphicsContext;
	MkGuiContextPtr m_guiContext;
	std::unique_ptr<class MkGuiStyleManager> m_styleManager;
	MikanModelResourceManagerUniquePtr m_modelResourceManager;
};
