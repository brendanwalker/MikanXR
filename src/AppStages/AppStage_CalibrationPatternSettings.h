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

	virtual void renderUI() override;

	static const char* APP_STAGE_NAME;
}; 
