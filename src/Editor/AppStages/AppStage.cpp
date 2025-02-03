#include "App.h"
#include "AppStage.h"
#include "GlViewport.h"
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
	MainWindow* mainWindow,
	const std::string& stageName)
	: m_ownerWindow(mainWindow)
	, m_bIsEntered(false)
	, m_bIsPaused(false)
	, m_appStageName(stageName)
{
}

AppStage::~AppStage() 
{
}

GlViewportPtr AppStage::addViewport()
{
	GlViewportPtr viewport= 
		std::make_shared<GlViewport>(
			glm::i32vec2(m_ownerWindow->getWidth(), m_ownerWindow->getHeight()));
	m_viewports.push_back(viewport);

	// Start listing to mouse input
	viewport->bindInput();

	return viewport;
}

GlViewportConstPtr AppStage::getRenderingViewport() const
{
	return MainWindow::getInstance()->getRenderingViewport();
}

Rml::Context* AppStage::getRmlContext() const 
{
	return m_ownerWindow->getRmlManager()->getRmlUIContext(); 
}

void AppStage::enter() 
{
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
		for (GlViewportPtr viewport : m_viewports)
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

		// Force an update to clear all deleted documents
		getRmlContext()->Update();

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
	for (GlViewportPtr viewport : m_viewports)
	{
		viewport->update(deltaSeconds);
	}

	if (getRmlContext() != nullptr)
	{
		getRmlContext()->Update();
	}
}

void AppStage::render() 
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