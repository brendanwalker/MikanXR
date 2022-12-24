#include "App.h"
#include "AppStage.h"
#include "Renderer.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

Rml::Context* AppStage::getRmlContext() const 
{
	return m_app->getRmlUIContext(); 
}

void AppStage::enter() 
{
	if (!m_bIsEntered)
	{
		Renderer* renderer = Renderer::getInstance();
		int window_width = renderer->getSDLWindowWidth();
		int window_height = renderer->getSDLWindowHeight();

		m_bIsEntered = true;
	}
}

void AppStage::exit() 
{
	if (m_bIsEntered)
	{
		for (auto it = m_rmlDocuments.begin(); it != m_rmlDocuments.end(); it++)
		{
			Rml::ElementDocument* document = *it;

			if (document != nullptr)
			{
				document->Close();
			}
		}
		m_rmlDocuments.clear();

		m_bIsEntered = false;
	}
}

void AppStage::onSDLEvent(SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_F5)
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

Rml::ElementDocument* AppStage::addRmlDocument(const std::string& docPath)
{
	Rml::ElementDocument* document = getRmlContext()->LoadDocument(docPath);
	if (document != nullptr)
	{
		m_rmlDocuments.push_back(document);
		document->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);
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

void AppStage::update() 
{
	if (getRmlContext() != nullptr)
	{
		getRmlContext()->Update();
	}
}

void AppStage::render() 
{
}

void AppStage::renderUI() 
{
	if (getRmlContext() != nullptr)
	{
		getRmlContext()->Render();
	}
}