#include "IMkGraphicsContext.h"
#include "IMkWindowEventListener.h"
#include "IMkWindowManager.h"
#include "SdlWindow.h"
#include "SdlWindowManager.h"
#include "SdlWindowEventListener.h"
#include "Logger.h"

#include <assert.h>

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_events.h>
	#include <SDL2/SDL_mouse.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_syswm.h>
#endif

#include "SdlCommon.h"

SdlWindow::SdlWindow(
	IMkWindowManagerPtr ownerWindowManager, 
	IMkGraphicsContextPtr graphicsContext)
	: m_ownerWindowManager(ownerWindowManager)
	, m_graphicsContext(graphicsContext)
{
}

SdlWindow::~SdlWindow()
{
	shutdown();
}

SdlWindow* SdlWindow::enableGLDataSharing()
{
	assert(m_sdlWindow == nullptr);
	m_bGLDataSharingEnabled = true;
	return this;
}

void SdlWindow::setTitle(const std::string& title)
{
	if (m_sdlWindow != nullptr)
	{
		SDL_SetWindowTitle(m_sdlWindow, title.c_str());
	}

	m_title = title;
}

void SdlWindow::setSize(int width, int height)
{
	if (m_sdlWindow != nullptr)
	{
		SDL_SetWindowSize(m_sdlWindow, width, height);
	}
	else
	{
		m_width = width;
		m_height = height;
	}
}

bool SdlWindow::startup()
{
	IMkWindowManagerPtr ownerWindowManager = m_ownerWindowManager.lock();
	assert(ownerWindowManager != nullptr && ownerWindowManager->getIsInitialized());
	IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();
	assert(graphicsContext != nullptr && graphicsContext->getGraphicsAPI() == eGraphicsAPI::OpenGL);

	bool success = true;

	if (m_bGLDataSharingEnabled)
	{
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	}

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_sdlWindow = SDL_CreateWindow(m_title.c_str(),
								   SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED,
								   m_width, m_height,
								   window_flags);
	if (m_sdlWindow == nullptr)
	{
		MIKAN_LOG_ERROR("SdlWindow::startup") << "Unable to initialize SDL window: " << SDL_GetError();
		success = false;
	}

	if (success)
	{
		m_glContext = SDL_GL_CreateContext(m_sdlWindow);
		if (m_glContext != NULL)
		{
			ownerWindowManager->pushCurrentWindowContext(this);
			SDL_GL_SetSwapInterval(0);
		}
		else
		{
			MIKAN_LOG_ERROR("SdlWindow::startup") << "Unable to initialize SDL OpenGL context: " << SDL_GetError();
			success = false;
		}
	}

	if (success)
	{
		graphicsContext->onNativeContextCreated(m_glContext);
		if (!graphicsContext->startup())
		{
			MIKAN_LOG_ERROR("SdlWindow::startup") << "Unable to initialize graphics context";
			success = false;
		}
	}

	if (success)
	{
		m_windowId = SDL_GetWindowID(m_sdlWindow);

		m_isShown = true;
		m_hasMouseFocus = true;
		m_hasKeyboardFocus = true;
	}

	return success;
}

void SdlWindow::shutdown()
{
	IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();
	if (graphicsContext)
	{
		graphicsContext->shutdown();
	}

	if (m_glContext != NULL)
	{
		SDL_GL_DeleteContext(m_glContext);
		m_glContext = NULL;
	}

	if (m_sdlWindow != NULL)
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow = NULL;
	}

	m_hasKeyboardFocus = false;
	m_hasMouseFocus = false;
	m_width = 0;
	m_height = 0;
}

void SdlWindow::update(float deltaSeconds)
{
	// Base implementation - subclasses override for app logic
}

void SdlWindow::render()
{
	// Base implementation - subclasses override for render logic
}

void SdlWindow::present()
{
	SDL_GL_SwapWindow(m_sdlWindow);
}

IMkViewportPtr SdlWindow::getRenderingViewport() const
{
	IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();

	return graphicsContext ? graphicsContext->getRenderingViewport() : IMkViewportPtr();
}

void SdlWindow::getMouseScreenPosition(int& outScreenX, int& outScreenY) const
{
	SDL_GetMouseState(&outScreenX, &outScreenY);
}

void SdlWindow::handleEvents(IMkWindowEventListener* eventListener)
{
	IMkWindowManagerPtr ownerWindowManager = m_ownerWindowManager.lock();
	assert(ownerWindowManager != nullptr);

	std::vector<SDL_Event>& events = static_cast<SdlWindowManager*>(ownerWindowManager.get())->getEvents();
	auto it = events.begin();
	while (it != events.end())
	{
		SDL_Event& sdlEvent = *it;

		bool bHandled = false;

		if (sdlEvent.window.windowID == m_windowId)
		{
			bHandled = handleSDLWindowEvent(&sdlEvent);

			if (!bHandled && eventListener != nullptr)
			{
				SdlWindowEvent mkEvent(&sdlEvent);
				bHandled = eventListener->onWindowEvent(mkEvent);
			}
		}

		if (bHandled)
		{
			it = events.erase(it);
		}
		else
		{
			it++;
		}
	}
}

bool SdlWindow::handleSDLWindowEvent(const SDL_Event* event)
{
	bool bHandled = true;

	if (event->type == SDL_WINDOWEVENT)
	{
		switch (event->window.event)
		{
			case SDL_WINDOWEVENT_SHOWN:
				m_isShown = true;
				break;

			case SDL_WINDOWEVENT_HIDDEN:
				m_isShown = false;
				break;

			case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					m_width = event->window.data1;
					m_height = event->window.data2;
					IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();
					if (graphicsContext)
					{
						graphicsContext->onWindowSizeChanged(m_width, m_height);
					}
				}
				break;

			case SDL_WINDOWEVENT_ENTER:
				m_hasMouseFocus = true;
				break;

			case SDL_WINDOWEVENT_LEAVE:
				m_hasMouseFocus = false;
				break;

			case SDL_WINDOWEVENT_FOCUS_GAINED:
				m_hasKeyboardFocus = true;
				break;

			case SDL_WINDOWEVENT_FOCUS_LOST:
				m_hasKeyboardFocus = false;
				break;

			case SDL_WINDOWEVENT_MINIMIZED:
				m_isMinimized = true;
				break;

			case SDL_WINDOWEVENT_MAXIMIZED:
				m_isMinimized = false;
				break;

			case SDL_WINDOWEVENT_RESTORED:
				m_isMinimized = false;
				break;

			case SDL_WINDOWEVENT_CLOSE:
				SDL_HideWindow(m_sdlWindow);
				m_wantsDestroy = true;
				break;

			default:
				bHandled = false;
		}
	}
	else
	{
		bHandled = false;
	}

	return bHandled;
}

void SdlWindow::focus()
{
	if (!m_isShown)
	{
		SDL_ShowWindow(m_sdlWindow);
	}

	SDL_RaiseWindow(m_sdlWindow);
}

void SdlWindow::makeContextCurrent()
{
	int result = SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
	if (result != 0)
	{
		const char* errorMessage = SDL_GetError();
		MIKAN_LOG_ERROR("SdlWindow::makeContextCurrent") << "Error with SDL_GL_MakeCurrent: " << errorMessage;
	}
}

IMkWindowPtr createMkWindow(
	IMkWindowManagerPtr ownerWindowManager,
	IMkGraphicsContextPtr graphicsContext)
{
	// TODO: Use the graphics context API to determine which window type to create 
	// (e.g. OpenGL = SdlGlWindow, Vulkan = SdlVulkanWindow, etc.)
	return std::make_shared<SdlWindow>(ownerWindowManager, graphicsContext);
}
