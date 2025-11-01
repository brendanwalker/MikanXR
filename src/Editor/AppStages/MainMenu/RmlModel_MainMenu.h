#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_MainMenu : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, AppSettingsConfigPtr appSettings);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnResumeProject;
	SinglecastDelegate<void()> OnOpenProject;
	SinglecastDelegate<void()> OnNewProject;
	SinglecastDelegate<void()> OnTutorial;
	SinglecastDelegate<void()> OnExit;

protected:
	AppSettingsConfigWeakPtr m_appSettingsConfig;
	std::vector<std::string> m_languageIdList;
	std::string m_selectedLangugeId;
};
