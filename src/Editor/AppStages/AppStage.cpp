#include "App.h"
#include "AppStage.h"
#include "MikanViewport.h"
#include "GlRmlUiRenderer.h"
#include "MainWindow.h"
#include "RmlManager.h"
#include "SdlWindow.h"
#include "Shared/ModalDialog.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

#include <filesystem>

AppStage::AppStage(
	IEditorWindow* ownerWindow,
	const std::string& stageName)
	: m_ownerWindow(ownerWindow)
	, m_bIsEntered(false)
	, m_bIsPaused(false)
	, m_appStageName(stageName)
{
}

AppStage::~AppStage() 
{
}

ProjectManagerPtr AppStage::getProjectManager() const
{
	return m_ownerWindow->getProjectManager();
}

ProjectConfigPtr AppStage::getProjectConfig() const
{
	return getProjectManager()->getProjectConfig();
}

MikanViewportPtr AppStage::addViewport()
{
	auto viewport= 
		std::make_shared<MikanViewport>(
			m_ownerWindow,
			glm::i32vec2(m_ownerWindow->getWidth(), m_ownerWindow->getHeight()));
	m_viewports.push_back(viewport);

	// Start listing to mouse input
	viewport->bindInput();

	return viewport;
}

MikanViewportConstPtr AppStage::getRenderingViewport() const
{
	return std::static_pointer_cast<const MikanViewport>(MainWindow::getInstance()->getRenderingViewport());
}

Rml::Context* AppStage::getRmlContext() const 
{
	return m_rmlContext;
}

void AppStage::enter() 
{
	// Allocate a dedicated RmlUi context for this app stage
	m_rmlContext = m_ownerWindow->getRmlManager()->pushContext(m_appStageName);

	if (!m_bIsEntered)
	{
		// Add a default fullscreen viewport for each appstage
		addViewport();

		m_bIsEntered = true;
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

		// Clean up any remaining docs allocated by the stage
		for (auto it = m_rmlDocuments.begin(); it != m_rmlDocuments.end(); it++)
		{
			Rml::ElementDocument* document = *it;

			if (document != nullptr)
			{
				document->Close();
			}
		}
		m_rmlDocuments.clear();

		// Dispose and delete all registered RmlModels
		for (IRmlModel* model : m_rmlModels)
		{
			model->dispose();
			delete model;
		}
		m_rmlModels.clear();

		// Force an update to clear all deleted documents
		getRmlContext()->Update();

		// Deallocate the RmlUi context
		m_ownerWindow->getRmlManager()->popContext(m_rmlContext);
		m_rmlContext = nullptr;

		m_bIsEntered = false;
	}
}

void AppStage::onSDLEvent(const SDL_Event* event)
{
	SdlWindow& sdlWindow = m_ownerWindow->getSdlWindow();
	const bool bHasKeyboardFocus = sdlWindow.hasKeyboardFocus();

	switch (event->type)
	{
	case SDL_KEYDOWN:
		if (bHasKeyboardFocus && event->key.keysym.sym == SDLK_F5)
		{
			for (auto it = m_rmlDocuments.begin(); it != m_rmlDocuments.end(); it++)
			{
				Rml::ElementDocument* document= *it;

				if (document != nullptr)
				{
					document->ReloadStyleSheet();
				}
			}
		}
		break;
	default:
		break;
	}
}

Rml::ElementDocument* AppStage::addRmlDocument(const std::string& docFilename, bool isModal)
{
	std::filesystem::path relDocPath = "rml";
	relDocPath/= docFilename;

	Rml::ElementDocument* document = getRmlContext()->LoadDocument(relDocPath.string());
	if (document != nullptr)
	{
		m_rmlDocuments.push_back(document);
		document->Show(
			isModal ? Rml::ModalFlag::Modal : Rml::ModalFlag::None, 
			Rml::FocusFlag::Document);
	}

	return document;
}

bool AppStage::removeRmlDocument(Rml::ElementDocument* doc)
{
	for (auto it = m_rmlDocuments.begin(); it != m_rmlDocuments.end(); it++)
	{
		if (*it == doc)
		{
			m_rmlDocuments.erase(it);
			doc->Close();
			return true;
		}
	}

	return false;
}

void AppStage::pause() 
{
	if (!m_bIsPaused)
	{
		for (Rml::ElementDocument* doc : m_rmlDocuments)
		{
			doc->Hide();
		}

		m_bIsPaused= true;
	}
}

void AppStage::resume() 
{
	if (m_bIsPaused)
	{
		for (Rml::ElementDocument* doc : m_rmlDocuments)
		{
			doc->Show();
		}

		m_bIsPaused= false;
	}
}

void AppStage::update(float deltaSeconds) 
{
	// Update the modal dialog on the top of the stack
	ModalDialog* modalDialog= getCurrentModalDialog();
	if (modalDialog != nullptr)
	{
		modalDialog->update();
	}

	// Process input in each viewport
	for (MikanViewportPtr viewport : m_viewports)
	{
		viewport->update(deltaSeconds);
	}

	if (getRmlContext() != nullptr)
	{
		getRmlContext()->Update();

		// Update all registered RmlModels after RmlUI context update
		for (IRmlModel* model : m_rmlModels)
		{
			model->update(deltaSeconds);
		}
	}
}

void AppStage::render(IMkViewportPtr targetViewport)
{
	// Override this method in derived classes to render the stage specific 3D geometry
}

void AppStage::renderUI() 
{
	// Override this method in derived classes to render the stage specific UI
	// By default, always render the RmlUi in every app stage
	m_ownerWindow->getRmlUiRenderer()->render();
}

void AppStage::popModalDialog()
{
	ModalDialog* modalDialog = getCurrentModalDialog();
	if (modalDialog != nullptr)
	{
		m_modalDialogStack.pop_back();
		delete modalDialog;
	}
}