#pragma once

#include "ISdlMkWindow.h"
#include "ObjectSystemFwd.h"
#include "MulticastDelegate.h"

#include <string>

class IEditorWindow : public ISdlMkWindow
{
public:
	virtual ProjectManagerPtr getProjectManager() const = 0;
	virtual class MikanServer* getMikanServer() const = 0;
	virtual class MikanFontManager* getFontManager() const = 0;
	virtual class InputManager* getInputManager() const = 0;
	virtual class OpenCVManager* getOpenCVManager() const = 0;
	virtual class ClientSourceManager* getClientSourceManager() const = 0;
	virtual class LocalizationManager* getLocalizationManager() const = 0;
	virtual class EventBus* getEventBus() const = 0;
	virtual class MkGuiStyleManager* getMkGuiStyleManager() const = 0;

	virtual class App* getOwnerApp() const = 0;
	virtual class AppStage* getCurrentAppStage() const = 0;
	virtual class AppStage* getParentAppStage() const = 0;
	virtual class AppStage* pushAppStage(const std::string& appStageName) = 0;
	virtual void popAppState() = 0;

	template<typename t_app_stage>
	t_app_stage* pushAppStageOfType()
	{
		const std::string appStageName = t_app_stage::APP_STAGE_NAME;
		return static_cast<t_app_stage*>(pushAppStage(appStageName));
	}

	MulticastDelegate<void(class AppStage* oldAppStage, class AppStage* newAppStage)> OnAppStageEntered;
	MulticastDelegate<void(class AppStage* oldAppStage, class AppStage* newAppStage)> OnAppStageExited;
};