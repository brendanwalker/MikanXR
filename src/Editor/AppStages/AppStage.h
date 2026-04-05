#pragma once

#include "IEditorWindow.h"
#include "IRemoteControllable.h"
#include "LocalizationManager.h"
#include "MikanRendererFwd.h"
#include "ProjectManager.h"
#include "Shared/GuiPanel.h"

#include <string>
#include <vector>
#include <memory>

//-- typedefs -----
typedef union SDL_Event SDL_Event;

class ModalDialog;

using AppStagePtr = std::shared_ptr<class AppStage>;
using AppStageFactoryPtr = std::shared_ptr<class AppStageFactory>;
using MikanViewportList = std::vector<MikanViewportPtr>;

class AppStage : public IRemoteControllable
{
public:
	AppStage(
		IEditorWindow* ownerWindow,
		const std::string& stageName);
	virtual ~AppStage();

	IEditorWindow* getOwnerWindow() const { return m_ownerWindow; }
	ProjectManagerPtr getProjectManager() const;
	ProjectConfigPtr getProjectConfig() const;
	const struct EditorSettings& getEditorSettings() const;
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
	virtual void onGui();
	virtual void render(IMkViewportPtr targetViewport);

	virtual void onSDLEvent(const SDL_Event* event);

	MikanViewportPtr getFirstViewport() const { return m_viewports[0]; }
	const MikanViewportList& getViewportList() const { return m_viewports; }
	MikanViewportConstPtr getRenderingViewport() const;
	MikanViewportPtr addViewport();

	template<typename t_gui_panel>
	t_gui_panel* addGuiPanel()
	{
		t_gui_panel* panel = new t_gui_panel(this);
		m_guiPanels.push_back(panel);
		return panel;
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

	// -- IRemoteControllable Interface -- //
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults) override;

protected:
	IEditorWindow* m_ownerWindow;
	bool m_bIsEntered= false;
	bool m_bIsPaused= false;
	std::string m_appStageName;
	MikanViewportList m_viewports;
	std::vector<IGuiPanel*> m_guiPanels;
	std::vector<class ModalDialog*> m_modalDialogStack;
};

class AppStageFactory
{
public:
	using AppStageConstructorFunc = std::function<AppStagePtr()>;

	AppStageFactory() = default;
	AppStageFactory(IEditorWindow* ownerWindow) 
		: m_ownerWindow(ownerWindow)
	{}

	AppStagePtr allocateAppStage(const std::string& stageName) const
	{
		auto it = m_appStageConstructors.find(stageName);
		if (it != m_appStageConstructors.end())
		{
			return it->second();
		}
		return AppStagePtr();
	}

	template <class t_app_stage_factory_class>
	void addAppStageConstructor()
	{
		const std::string stageName = t_app_stage_factory_class::APP_STAGE_NAME;

		const auto constructorFunc = [this]() -> AppStagePtr
		{
			return std::make_shared<t_app_stage_factory_class>(m_ownerWindow);
		};

		m_appStageConstructors.insert({stageName, constructorFunc});
	}

protected:
	IEditorWindow* m_ownerWindow= nullptr;
	std::map<std::string, AppStageConstructorFunc> m_appStageConstructors;
};
