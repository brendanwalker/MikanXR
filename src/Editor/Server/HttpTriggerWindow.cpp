//-- includes -----
#include "HttpTriggerWindow.h"
#include "App.h"
#include "HttpInterprocessMessageServer.h"
#include "IMkGraphicsContext.h"
#include "IMkState.h"
#include "LocText.h"
#include "MikanServer.h"
#include "MkGuiContext.h"
#include "MkGuiScopedUpdate.h"
#include "MkGuiScopedWindow.h"
#include "MkStateModifiers.h"
#include "MkStateStack.h"
#include "MkWindowEvent.h"

#include "imgui.h"

#include <easy/profiler.h>

//-- constants -----
static const int k_http_trigger_window_width= 480;
static const int k_http_trigger_window_height= 600;

//-- public methods -----
HttpTriggerWindow::HttpTriggerWindow(App* ownerApp)
	: EditorWindow(ownerApp)
{
	shareGraphicsContextWithMainWindow();
}

bool HttpTriggerWindow::startup()
{
	EASY_FUNCTION();

	bool success= true;

	if (success
		&& !startupWindow(locText("windows.httpTriggers"), k_http_trigger_window_width, k_http_trigger_window_height))
	{
		success= false;
	}

	if (success && !startupGuiContext("http_triggers"))
	{
		success= false;
	}

	if (success && !startupStyleManager())
	{
		success= false;
	}

	if (success && !startupTextureCache())
	{
		success= false;
	}

	if (success && !startupModelResourceManager())
	{
		success= false;
	}

	return success;
}

void HttpTriggerWindow::update(float deltaSeconds)
{
	EASY_FUNCTION();

	// Push the ImGui update scope
	MkGuiScopedUpdate scopedCtx(*m_guiContext);

	// Process most recent SDL events (keyboard, mouse, etc)
	m_mkWindowContext->handleEvents(this);

	// Process UI input and build ImGui draw lists
	updateUI();
}

void HttpTriggerWindow::render()
{
	EASY_FUNCTION();

	// Clear the window
	m_graphicsContext->renderBegin();

	{
		MkScopedState scopedState= m_graphicsContext->getMkStateStack().createScopedState("HttpTriggerWindow renderUI");
		IMkState* glState= scopedState.getStackState();

		mkStateSetViewport(glState, 0, 0, (int)m_mkWindowContext->getWidth(), (int)m_mkWindowContext->getHeight());

		// Submit the MkGui draw calls
		m_guiContext->submitDrawData();
	}

	// Finalize rendering
	m_graphicsContext->renderEnd();

	// Present the rendered frame
	m_mkWindowContext->present();
}

void HttpTriggerWindow::updateUI()
{
	ImGui::SetNextWindowSize(ImVec2(getWidth(), getHeight()), ImGuiCond_Once);
	MkGuiScopedWindow httpTriggerWindow(locWindowTitle("windows.httpTriggers"), nullptr,
										ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus
											| ImGuiWindowFlags_NoMove);

	HttpInterprocessMessageServer* httpServer= getMikanServer()->getHttpMessageServer();
	std::vector<std::string> routePaths= httpServer->getRegisteredRoutePaths();

	if (routePaths.empty())
	{
		ImGui::TextUnformatted(locText("httpTriggers.noRoutesRegistered"));
		return;
	}

	for (const std::string& path : routePaths)
	{
		if (ImGui::Button(path.c_str()))
		{
			HttpRouteResponse response;
			httpServer->invokeRouteHandler(path, response);
		}
	}
}

void HttpTriggerWindow::shutdown()
{
	shutdownModelResourceManager();
	shutdownTextureCache();
	shutdownStyleManager();
	shutdownGuiContext();
	shutdownWindow();
}

// -- IMkWindowEventListener
bool HttpTriggerWindow::onWindowEvent(const MkWindowEvent& event) { return m_guiContext->onWindowEvent(event); }
