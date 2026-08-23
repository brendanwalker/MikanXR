#include "App.h"
#include "AppStage.h"
#include "Shared/GuiPanel.h"
#include "EditorObjectSystem.h"
#include "MikanViewport.h"
#include "IEditorWindow.h"
#include "Shared/ModalDialog.h"

#include <filesystem>

AppStage::AppStage(IEditorWindow* ownerWindow, const std::string& stageName)
	: m_ownerWindow(ownerWindow)
	, m_bIsEntered(false)
	, m_bIsPaused(false)
	, m_appStageName(stageName)
{
}

AppStage::~AppStage() {}

ProjectManagerPtr AppStage::getProjectManager() const { return m_ownerWindow->getProjectManager(); }

IMkGraphicsContext* AppStage::getGraphicsContext() const { return m_ownerWindow->getGraphicsContext().get(); }

ProjectConfigPtr AppStage::getProjectConfig() const { return getProjectManager()->getProjectConfig(); }

const EditorSettings& AppStage::getEditorSettings() const
{
	return getProjectManager()->getSystemOfType<EditorObjectSystem>()->getEditorSettings();
}

MikanViewportPtr AppStage::addViewport()
{
	auto viewport= std::make_shared<MikanViewport>(m_ownerWindow,
												   glm::i32vec2(m_ownerWindow->getWidth(), m_ownerWindow->getHeight()));
	m_viewports.push_back(viewport);

	// Start listing to mouse input
	viewport->bindInput();

	return viewport;
}

MikanViewportConstPtr AppStage::getRenderingViewport() const
{
	return std::static_pointer_cast<const MikanViewport>(getOwnerWindow()->getRenderingViewport());
}

void AppStage::enter()
{
	if (!m_bIsEntered)
	{
		// Add a default fullscreen viewport for each appstage
		addViewport();

		m_bIsEntered= true;
	}
}

void AppStage::exit()
{
	if (m_bIsEntered)
	{
		// Destroy all viewports
		for (MikanViewportPtr viewport : m_viewports)
		{
			viewport->unbindInput();
		}
		m_viewports.clear();

		// Destroy all modal dialogs first
		while (m_modalDialogStack.size() > 0)
		{
			popModalDialog();
		}

		// Dispose and delete all registered GuiPanels
		for (IGuiPanel* panel : m_guiPanels)
		{
			panel->dispose();
			delete panel;
		}
		m_guiPanels.clear();

		m_bIsEntered= false;
	}
}

void AppStage::onWindowEvent(const MkWindowEvent& event) {}

void AppStage::pause()
{
	if (!m_bIsPaused)
	{
		m_bIsPaused= true;
	}
}

void AppStage::resume()
{
	if (m_bIsPaused)
	{
		m_bIsPaused= false;
	}
}

void AppStage::onGui()
{
	// Render the top-most modal dialog (if any)
	ModalDialog* modalDialog= getCurrentModalDialog();
	if (modalDialog != nullptr)
	{
		modalDialog->onGui();
	}

	// Override this method in derived classes to render the stage specific Mk GUI
}

void AppStage::update(float deltaSeconds)
{
	// Process deferred events emitted due to Gui interaction (e.g. button clicks, etc)
	for (IGuiPanel* panel : m_guiPanels)
	{
		panel->processDeferredGuiEvents();
	}

	// Process input in each viewport
	for (MikanViewportPtr viewport : m_viewports)
	{
		viewport->update(deltaSeconds);
	}
}

void AppStage::render(IMkViewportPtr targetViewport)
{
	// Override this method in derived classes to render the stage specific 3D geometry
}

void AppStage::popModalDialog()
{
	ModalDialog* modalDialog= getCurrentModalDialog();
	if (modalDialog != nullptr)
	{
		m_modalDialogStack.pop_back();
		delete modalDialog;
	}
}

// -- IRemoteControllable Interface -- //
bool AppStage::handleRemoteControlCommand(const std::string& command, const std::vector<std::string>& parameters,
										  std::vector<std::string>& outResults)
{
	// by default, we don't handle any commands
	return false;
}
