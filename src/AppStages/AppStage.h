#pragma once

#include "LocalizationManager.h"
#include <string>

//-- typedefs -----
typedef union SDL_Event SDL_Event;

namespace Rml {
	class Context;
	class ElementDocument;
};

class AppStage
{
public:
	AppStage(
		class App* app,
		const std::string& stageName)
		: m_app(app)
		, m_bIsEntered(false)
		, m_bIsPaused(false)
		, m_appStageName(stageName)
	{ }
	virtual ~AppStage() {}

	const std::string getAppStageName() const { return m_appStageName; }
	bool getHasEntered() const { return m_bIsEntered; }
	bool getIsPaused() const { return m_bIsPaused; }
	bool getIsUpdateActive() const { return getHasEntered() && !getIsPaused(); }

	virtual void enter();
	virtual void exit();
	virtual void pause();
	virtual void resume();
	virtual void update();
	virtual void render();
	virtual void renderUI();

	virtual void onSDLEvent(SDL_Event* event);

	Rml::Context* getRmlContext() const;
	Rml::ElementDocument* getCurrentRmlDocument() const;
	Rml::ElementDocument* pushRmlDocument(const std::string& docPath);
	bool popRmlDocument();
	virtual void onRmlClickEvent(const std::string& value) {}

protected:
	class App* m_app;
	bool m_bIsEntered= false;
	bool m_bIsPaused= false;
	std::string m_appStageName;
	std::vector<Rml::ElementDocument*> m_documentStack;
};
