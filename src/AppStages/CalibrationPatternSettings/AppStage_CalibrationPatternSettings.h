#pragma once

//-- includes -----
#include "AppStage.h"

//-- definitions -----
class AppStage_CalibrationPatternSettings : public AppStage
{
public:
	AppStage_CalibrationPatternSettings(class MainWindow* ownerWindow);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;

	virtual void onRmlClickEvent(const std::string& value) override;

	static const char* APP_STAGE_NAME;

protected:
	struct CalibrationPatternSettingsDataModel* m_dataModel = nullptr;
}; 
