#pragma once

//-- includes -----
#include "AppStage.h"

namespace sdlgui {
	class Screen;
}

namespace Rml {
	class ElementDocument;
}

//-- definitions -----
class AppStage_MainMenu : public AppStage
{
public:
	AppStage_MainMenu(class App* app);

	virtual void enter() override;
	virtual void exit() override;

	virtual void onRmlClickEvent(const std::string& value) override;

	static const char* APP_STAGE_NAME;

protected:
	Rml::ElementDocument* m_document= nullptr;
};
