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
	Renderer* renderer= Renderer::getInstance();
	int window_width = renderer->getSDLWindowWidth();
	int window_height = renderer->getSDLWindowHeight();
}

void AppStage::exit() 
{
	while (popRmlDocument());
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
		document->Show();
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
}

void AppStage::resume() 
{
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