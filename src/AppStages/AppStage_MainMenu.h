#pragma once

//-- includes -----
#include "AppStage.h"

namespace sdlgui {
	class Screen;
}

//-- definitions -----
class AppStage_MainMenu : public AppStage
{
public:
	AppStage_MainMenu(class App* app);

	virtual void renderUI() override;

	static const char* APP_STAGE_NAME;
};
