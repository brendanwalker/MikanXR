#pragma once

#include "LocalizationManager.h"
#include <string>

//-- typedefs -----
typedef union SDL_Event SDL_Event;

namespace Rml {
	class Context;
};

class AppStage
{
public:
	AppStage(
		class App* app,
		const std::string& stageName)
		: m_app(app)
		, m_appStageName(stageName)
	{ }
	virtual ~AppStage() {}

	const std::string getAppStageName() const { return m_appStageName; }
	Rml::Context* getRmlContext() const;

	virtual void enter();
	virtual void exit();
	virtual void pause();
	virtual void resume();
	virtual void update();
	virtual void render();
	virtual void renderUI();

	virtual void onSDLEvent(SDL_Event* event) {}

	virtual void onRmlClickEvent(const std::string& value) {}

protected:
	class App* m_app;
	std::string m_appStageName;
};
