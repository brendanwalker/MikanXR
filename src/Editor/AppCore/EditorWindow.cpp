//-- includes -----
#include "EditorWindow.h"
#include "Logger.h"

#include "IMkWindow.h"
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

// -- IMkWindow delegation ----

const char* EditorWindow::getTitle() const
{
	return m_mkWindow->getTitle();
}

float EditorWindow::getWidth() const
{
	return m_mkWindow->getWidth();
}

float EditorWindow::getHeight() const
{
	return m_mkWindow->getHeight();
}

float EditorWindow::getAspectRatio() const
{
	return m_mkWindow->getAspectRatio();
}

void EditorWindow::getMouseScreenPosition(int& outScreenX, int& outScreenY) const
{
	m_mkWindow->getMouseScreenPosition(outScreenX, outScreenY);
}

eWindowAPI EditorWindow::getWindowAPI() const
{
	return m_mkWindow->getWindowAPI();
}

void* EditorWindow::getNativeWindowHandle() const
{
	return m_mkWindow->getNativeWindowHandle();
}

IMkGraphicsContextPtr EditorWindow::getGraphicsContext() const
{
	return m_graphicsContext;
}

void EditorWindow::makeContextCurrent()
{
	m_mkWindow->makeContextCurrent();
}

bool EditorWindow::wantsDestroy() const
{
	return m_mkWindow->wantsDestroy();
}

void EditorWindow::present()
{
	m_mkWindow->present();
}

void EditorWindow::setTitle(const std::string& title)
{
	m_mkWindow->setTitle(title);
}

void EditorWindow::setSize(int width, int height)
{
	m_mkWindow->setSize(width, height);
}

void EditorWindow::handleEvents(IMkWindowEventListener* eventListener)
{
	m_mkWindow->handleEvents(eventListener);
}

bool EditorWindow::hasMouseFocus() const
{
	return m_mkWindow->hasMouseFocus();
}

bool EditorWindow::hasKeyboardFocus() const
{
	return m_mkWindow->hasKeyboardFocus();
}

// -- IEditorWindow ----

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

// -- Protected startup/shutdown helpers ----

bool EditorWindow::startupWindow(const std::string& title, int width, int height)
{
	m_mkWindow->setTitle(title);
	m_mkWindow->setSize(width, height);
	if (!m_mkWindow->startup())
	{
		MIKAN_LOG_ERROR("EditorWindow::startupWindow") << "Unable to initialize window: " << title;
		return false;
	}
	return true;
}

bool EditorWindow::startupGuiContext()
{
	m_guiContext = std::make_shared<MkGuiContext>(this);
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
	if (m_mkWindow != nullptr)
	{
		m_mkWindow->shutdown();
		m_mkWindow = nullptr;
	}
}
