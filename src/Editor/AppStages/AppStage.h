#pragma once

#include "IEditorWindow.h"
#include "LocalizationManager.h"
#include "MikanRendererFwd.h"
#include "ProjectManager.h"
#include "RmlFwd.h"
#include "Shared/RmlModel.h"

#include <string>
#include <vector>
#include <memory>

//-- typedefs -----
typedef union SDL_Event SDL_Event;

class ModalDialog;

using MikanViewportList = std::vector<MikanViewportPtr>;

class AppStage
{
public:
	AppStage(
		IEditorWindow* ownerWindow,
		const std::string& stageName);
	virtual ~AppStage();

	IEditorWindow* getOwnerWindow() const { return m_ownerWindow; }
	ProjectManagerPtr getProjectManager() const;
	ProjectConfigPtr getProjectConfig() const;
	template <class t_object_system_type>
	std::shared_ptr<t_object_system_type> getObjectSystemOfType() const
	{
		return getProjectManager()->getSystemOfType<t_object_system_type>();
	}

	const std::string getAppStageName() const { return m_appStageName; }
	bool getHasEntered() const { return m_bIsEntered; }
	bool getIsPaused() const { return m_bIsPaused; }
	bool getIsUpdateActive() const { return getHasEntered() && !getIsPaused(); }

	virtual void enter();
	virtual void exit();
	virtual void pause();
	virtual void resume();
	virtual void update(float deltaSeconds);
	virtual void render(IMkViewportPtr targetViewport);
	virtual void renderUI();

	virtual void onSDLEvent(const SDL_Event* event);

	MikanViewportPtr getFirstViewport() const { return m_viewports[0]; }
	const MikanViewportList& getViewportList() const { return m_viewports; }
	MikanViewportConstPtr getRenderingViewport() const;
	MikanViewportPtr addViewport();

	Rml::Context* getRmlContext() const;
	Rml::ElementDocument* addRmlDocument(const std::string& docFilename, bool isModal= false);
	bool removeRmlDocument(Rml::ElementDocument* doc);
	virtual void onRmlClickEvent(const std::string& value) {}

	template<typename t_rml_model>
	t_rml_model* addRmlModel()
	{
		t_rml_model* model = new t_rml_model();
		m_rmlModels.push_back(model);
		return model;
	}

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

	template<class t_system_type>
	std::shared_ptr<t_system_type> getSystemOfType()
	{
		return m_ownerWindow->getProjectManager()->getSystemOfType<t_system_type>();
	}

protected:
	IEditorWindow* m_ownerWindow;
	bool m_bIsEntered= false;
	bool m_bIsPaused= false;
	std::string m_appStageName;
	MikanViewportList m_viewports;
	std::vector<Rml::ElementDocument*> m_rmlDocuments;
	std::vector<RmlModel*> m_rmlModels;
	std::vector<class ModalDialog*> m_modalDialogStack;
};
