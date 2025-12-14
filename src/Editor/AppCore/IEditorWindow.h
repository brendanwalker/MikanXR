#pragma once

#include "ISdlMkWindow.h"
#include "ObjectSystemFwd.h"
#include "MulticastDelegate.h"

class IEditorWindow : public ISdlMkWindow
{
public:
	virtual ProjectManagerPtr getProjectManager() const = 0;
	virtual class MikanServer* getMikanServer() const = 0;
	virtual class MikanFontManager* getFontManager() const = 0;
	virtual class RmlManager* getRmlManager() const = 0;
	virtual class GlRmlUiRender* getRmlUiRenderer() const = 0;
	virtual class InputManager* getInputManager() const = 0;
	virtual class OpenCVManager* getOpenCVManager() const = 0;
	virtual class ClientSourceManager* getClientSourceManager() const = 0;

	virtual class App* getOwnerApp() const = 0;
	virtual class AppStage* getCurrentAppStage() const = 0;
	virtual class AppStage* getParentAppStage() const = 0;
	virtual void pushAppStage(class AppStage* appStage) = 0;
	virtual void popAppState() = 0;

	template<typename t_app_stage>
	t_app_stage* pushAppStageOfType()
	{
		t_app_stage* appStage = new t_app_stage(this);
		pushAppStage(appStage);
		return appStage;
	}

	MulticastDelegate<void(class AppStage* oldAppStage, class AppStage* newAppStage)> OnAppStageEntered;
	MulticastDelegate<void(class AppStage* oldAppStage, class AppStage* newAppStage)> OnAppStageExited;
};