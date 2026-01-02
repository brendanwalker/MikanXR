#pragma once

//-- includes -----
#include "AppStage.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "RmlFwd.h"

//-- definitions -----
class AppStage_MainMenu : public AppStage
{
public:
	AppStage_MainMenu(class IEditorWindow* ownerWindow);

	virtual void enter() override;

	static const char* APP_STAGE_NAME;

protected:
	void onResumeProject();
	void onOpenProject();
	void onNewProject();
	void onTutorial();
	void onExit();

private:
	AppSettingsConfigPtr m_appSettingsConfig;
	ProjectManagerPtr m_projectManager;
};
