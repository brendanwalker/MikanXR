//-- includes -----
#include "EditorWindow.h"
#include "Logger.h"

#include "IMkWindowContext.h"
#include "IMkGraphicsContext.h"
#include "MikanModelResourceManager.h"
#include "MkGuiContext.h"
#include "MkGuiStyleManager.h"
#include "PathUtils.h"

//-- public methods -----
EditorWindow::EditorWindow(App* ownerApp)
	: m_ownerApp(ownerApp)
{}

EditorWindow::~EditorWindow()
{}

// -- IEditorWindow ----
IMkGraphicsContextPtr EditorWindow::getGraphicsContext() const 
{ 
	return m_graphicsContext; 
}

IMkWindowContextPtr EditorWindow::getMkWindowContext() const 
{ 
	return m_mkWindowContext; 
}

const char* EditorWindow::getTitle() const
{
	return m_mkWindowContext->getTitle();
}

float EditorWindow::getWidth() const
{
	return m_mkWindowContext->getWidth();
}

float EditorWindow::getHeight() const
{
	return m_mkWindowContext->getHeight();
}

float EditorWindow::getAspectRatio() const
{
	return m_mkWindowContext->getAspectRatio();
}

void EditorWindow::getMouseScreenPosition(int& outScreenX, int& outScreenY) const
{
	m_mkWindowContext->getMouseScreenPosition(outScreenX, outScreenY);
}

MikanModelResourceManager* EditorWindow::getModelResourceManager()
{
	return m_modelResourceManager.get();
}

MkGuiStyleManager* EditorWindow::getMkGuiStyleManager() const
{
	return m_styleManager.get();
}

App* EditorWindow::getOwnerApp() const
{
	return m_ownerApp;
}

// -- IMkWindowContext delegation ----
eWindowAPI EditorWindow::getWindowAPI() const
{
	return m_mkWindowContext->getWindowAPI();
}

void* EditorWindow::getNativeWindowHandle() const
{
	return m_mkWindowContext->getNativeWindowHandle();
}

void EditorWindow::makeContextCurrent()
{
	m_mkWindowContext->makeContextCurrent();
}

bool EditorWindow::wantsDestroy() const
{
	return m_mkWindowContext->wantsDestroy();
}

void EditorWindow::present()
{
	m_mkWindowContext->present();
}

void EditorWindow::setTitle(const std::string& title)
{
	m_mkWindowContext->setTitle(title);
}

void EditorWindow::setSize(int width, int height)
{
	m_mkWindowContext->setSize(width, height);
}

void EditorWindow::handleEvents(IMkWindowEventListener* eventListener)
{
	m_mkWindowContext->handleEvents(eventListener);
}

bool EditorWindow::hasMouseFocus() const
{
	return m_mkWindowContext->hasMouseFocus();
}

bool EditorWindow::hasKeyboardFocus() const
{
	return m_mkWindowContext->hasKeyboardFocus();
}

// -- Protected startup/shutdown helpers ----

bool EditorWindow::startupWindow(const std::string& title, int width, int height)
{
	m_mkWindowContext->setTitle(title);
	m_mkWindowContext->setSize(width, height);
	if (!m_mkWindowContext->startup())
	{
		MIKAN_LOG_ERROR("EditorWindow::startupWindow") << "Unable to initialize window: " << title;
		return false;
	}
	return true;
}

bool EditorWindow::startupGuiContext()
{
	m_guiContext = std::make_shared<MkGuiContext>(getMkWindowContext().get());
	if (!m_guiContext->startup())
	{
		MIKAN_LOG_ERROR("EditorWindow::startupGuiContext") << "Unable to create GUI context";
		return false;
	}
	return true;
}

bool EditorWindow::startupStyleManager()
{
	m_styleManager = std::make_unique<MkGuiStyleManager>();
	const auto stylesPath = PathUtils::getResourceDirectory() / "gui_styles";
	if (!m_styleManager->startup(m_guiContext.get(), stylesPath))
	{
		MIKAN_LOG_ERROR("EditorWindow::startupStyleManager") << "Failed to initialize style manager";
		return false;
	}
	return true;
}

bool EditorWindow::startupModelResourceManager()
{
	if (!m_modelResourceManager->startup())
	{
		MIKAN_LOG_ERROR("EditorWindow::startupModelResourceManager") << "Unable to initialize model resource manager";
		return false;
	}
	return true;
}

void EditorWindow::shutdownModelResourceManager()
{
	if (m_modelResourceManager != nullptr)
	{
		m_modelResourceManager->shutdown();
		m_modelResourceManager = nullptr;
	}
}

void EditorWindow::shutdownStyleManager()
{
	if (m_styleManager != nullptr)
	{
		m_styleManager->shutdown();
		m_styleManager = nullptr;
	}
}

void EditorWindow::shutdownGuiContext()
{
	if (m_guiContext != nullptr)
	{
		m_guiContext->shutdown();
		m_guiContext = nullptr;
	}
}

void EditorWindow::shutdownWindow()
{
	if (m_mkWindowContext != nullptr)
	{
		m_mkWindowContext->shutdown();
		m_mkWindowContext = nullptr;
	}
}
