#pragma once

//-- includes -----
#include "AppStage.h"

//-- definitions -----
class AppStage_CalibrationPatternSettings : public AppStage
{
public:
	AppStage_CalibrationPatternSettings(class App* app);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;

	virtual void onRmlClickEvent(const std::string& value) override;

	static const char* APP_STAGE_NAME;

protected:
	struct CalibrationPatternSettingsDataModel* m_dataModel = nullptr;
}; 
