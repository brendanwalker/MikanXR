#pragma once

#include "LocalizationManager.h"
#include "RendererFwd.h"
#include "RmlFwd.h"

#include <string>
#include <vector>

//-- typedefs -----
typedef union SDL_Event SDL_Event;

class ModalDialog;

using GlViewportList = std::vector<GlViewportPtr>;

class AppStage
{
public:
	AppStage(
		class MainWindow* ownerWindow,
		const std::string& stageName);
	virtual ~AppStage();

	const std::string getAppStageName() const { return m_appStageName; }
	bool getHasEntered() const { return m_bIsEntered; }
	bool getIsPaused() const { return m_bIsPaused; }
	bool getIsUpdateActive() const { return getHasEntered() && !getIsPaused(); }

	virtual void enter();
	virtual void exit();
	virtual void pause();
	virtual void resume();
	virtual void update(float deltaSeconds);
	virtual void render();
	virtual void renderUI();

	virtual void onSDLEvent(const SDL_Event* event);

	GlViewportPtr getFirstViewport() const { return m_viewports[0]; }
	const GlViewportList& getViewportList() const { return m_viewports; }
	GlViewportConstPtr getRenderingViewport() const;
	GlViewportPtr addViewport();

	Rml::Context* getRmlContext() const;
	Rml::ElementDocument* addRmlDocument(const std::string& docFilename, bool isModal= false);
	bool AppStage::removeRmlDocument(Rml::ElementDocument* doc);
	virtual void onRmlClickEvent(const std::string& value) {}

	inline ModalDialog* getCurrentModalDialog() const
	{
		return (m_modalDialogStack.size() > 0) ? m_modalDialogStack[m_modalDialogStack.size() - 1] : nullptr;
	}

	template<typename t_modal_dialog>
	t_modal_dialog* pushModalDialog()
	{
		t_modal_dialog* modalDialog = new t_modal_dialog(this);
		m_modalDialogStack.push_back(modalDialog);

		return modalDialog;
	}

	void popModalDialog();

protected:
	class MainWindow* m_ownerWindow;
	bool m_bIsEntered= false;
	bool m_bIsPaused= false;
	std::string m_appStageName;
	GlViewportList m_viewports;
	std::vector<Rml::ElementDocument*> m_rmlDocuments;
	std::vector<class ModalDialog*> m_modalDialogStack;
};
