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
		while (popRmlDocument());
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
			Rml::ElementDocument* document= getCurrentRmlDocument();
			if (document != nullptr)
			{
				document->ReloadStyleSheet();
			}
		}
		break;
	default:
		break;
	}
}

Rml::ElementDocument* AppStage::getCurrentRmlDocument() const
{
	return (m_documentStack.size() > 0) ? m_documentStack.back() : nullptr;
}

Rml::ElementDocument* AppStage::pushRmlDocument(const std::string& docPath)
{
	Rml::ElementDocument* document = getRmlContext()->LoadDocument(docPath);
	if (document != nullptr)
	{
		m_documentStack.push_back(document);
		document->Show(Rml::ModalFlag::None, Rml::FocusFlag::Document);
	}

	return document;
}

bool AppStage::popRmlDocument()
{
	if (m_documentStack.size() > 0)
	{
		m_documentStack.back()->Close();
		m_documentStack.pop_back();
		return true;
	}

	return false;
}

void AppStage::pause() 
{
	if (!m_bIsPaused)
	{
		for (Rml::ElementDocument* doc : m_documentStack)
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
		for (Rml::ElementDocument* doc : m_documentStack)
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