#include "App.h"
#include "AppStage.h"
#include "Renderer.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
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