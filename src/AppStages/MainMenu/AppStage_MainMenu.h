#pragma once

//-- includes -----
#include "AppStage.h"
#include "RmlFwd.h"

namespace sdlgui {
	class Screen;
}

//-- definitions -----
class AppStage_MainMenu : public AppStage
{
public:
	AppStage_MainMenu(class App* app);

	virtual void enter() override;

	virtual void onRmlClickEvent(const std::string& value) override;

	static const char* APP_STAGE_NAME;	
};
