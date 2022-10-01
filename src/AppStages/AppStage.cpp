#include "AppStage.h"
#include "Renderer.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Debugger.h>

void AppStage::enter() 
{
	Renderer* renderer= Renderer::getInstance();
	int window_width = renderer->getSDLWindowWidth();
	int window_height = renderer->getSDLWindowHeight();

	m_context = Rml::CreateContext(m_appStageName, Rml::Vector2i(window_width, window_height));
	if (m_context != nullptr)
	{
		Rml::Debugger::Initialise(m_context);
	}
}

void AppStage::exit() 
{
	if (m_context != nullptr)
	{
		Rml::Debugger::Shutdown();
		Rml::RemoveContext(m_appStageName);
		m_context = nullptr;
	}
}

void AppStage::pause() 
{
}

void AppStage::resume() 
{
}

void AppStage::update() 
{
	if (m_context != nullptr)
	{
		m_context->Update();
	}
}

void AppStage::render() 
{
}

void AppStage::renderUI() 
{
	if (m_context != nullptr)
	{
		m_context->Render();
	}
}