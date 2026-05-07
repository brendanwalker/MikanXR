#include "IMkGraphicsContext.h"
#include "IMkWindowEventListener.h"
#include "IMkWindowContextManager.h"
#include "SdlWindowContext.h"
#include "SdlWindowContextManager.h"
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

SdlWindowContext::SdlWindowContext(
	IMkWindowContextManagerPtr ownerWindowManager, 
	IMkGraphicsContextPtr graphicsContext)
	: m_ownerWindowManager(ownerWindowManager)
	, m_graphicsContext(graphicsContext)
{
}

SdlWindowContext::~SdlWindowContext()
{
	shutdown();
}

void SdlWindowContext::enableGLDataSharing()
{
	assert(m_sdlWindow == nullptr);
	m_bGLDataSharingEnabled = true;
}

void SdlWindowContext::useExistingGLContext()
{
	assert(m_sdlWindow == nullptr);  // must be called before startup()
	m_bOwnsGLContext = false;
}

void SdlWindowContext::setTitle(const std::string& title)
{
	m_title = title;

	if (m_sdlWindow != nullptr)
	{
		SDL_SetWindowTitle(m_sdlWindow, title.c_str());
	}
}

void SdlWindowContext::setSize(int width, int height)
{
	m_width = width;
	m_height = height;

	if (m_sdlWindow != nullptr)
	{
		SDL_SetWindowSize(m_sdlWindow, width, height);
	}
}

bool SdlWindowContext::startup()
{
	IMkWindowContextManagerPtr ownerWindowManager = m_ownerWindowManager.lock();
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
		MIKAN_LOG_ERROR("SdlWindowContext::startup") << "Unable to initialize SDL window: " << SDL_GetError();
		success = false;
	}
	else
	{
		graphicsContext->onWindowSizeChanged(m_width, m_height);
	}

	if (success)
	{
		if (m_bOwnsGLContext)
		{
			// Create a new GL context owned by this window
			m_glContext = SDL_GL_CreateContext(m_sdlWindow);
			if (m_glContext != NULL)
			{
				ownerWindowManager->pushCurrentWindowContext(this);
				SDL_GL_SetSwapInterval(0);
			}
			else
			{
				MIKAN_LOG_ERROR("SdlWindowContext::startup") << "Unable to initialize SDL OpenGL context: " << SDL_GetError();
				success = false;
			}
		}
		else
		{
			// Attach this window to the currently active GL context (owned by another window)
			m_glContext = SDL_GL_GetCurrentContext();
			if (m_glContext != NULL)
			{
				SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
				ownerWindowManager->pushCurrentWindowContext(this);
			}
			else
			{
				MIKAN_LOG_ERROR("SdlWindowContext::startup") << "No active GL context to attach to";
				success = false;
			}
		}
	}

	if (success && m_bOwnsGLContext)
	{
		// Only initialize the graphics context for the context owner.
		// Secondary windows share an already-initialized graphics context.
		graphicsContext->onNativeContextCreated(m_glContext);
		if (!graphicsContext->startup())
		{
			MIKAN_LOG_ERROR("SdlWindowContext::startup") << "Unable to initialize graphics context";
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

void SdlWindowContext::shutdown()
{
	IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();
	if (graphicsContext && m_bOwnsGLContext)
	{
		graphicsContext->shutdown();
	}

	if (m_glContext != NULL)
	{
		if (m_bOwnsGLContext)
		{
			SDL_GL_DeleteContext(m_glContext);
		}
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

void SdlWindowContext::update(float deltaSeconds)
{
	// Base implementation - subclasses override for app logic
}

void SdlWindowContext::render()
{
	// Base implementation - subclasses override for render logic
}

void SdlWindowContext::present()
{
	SDL_GL_SwapWindow(m_sdlWindow);
}

IMkViewportPtr SdlWindowContext::getRenderingViewport() const
{
	IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();

	return graphicsContext ? graphicsContext->getRenderingViewport() : IMkViewportPtr();
}

void SdlWindowContext::getMouseScreenPosition(int& outScreenX, int& outScreenY) const
{
	SDL_GetMouseState(&outScreenX, &outScreenY);
}

void SdlWindowContext::handleEvents(IMkWindowEventListener* eventListener)
{
	IMkWindowContextManagerPtr ownerWindowManager = m_ownerWindowManager.lock();
	assert(ownerWindowManager != nullptr);

	std::vector<SDL_Event>& events = static_cast<SdlWindowContextManager*>(ownerWindowManager.get())->getEvents();
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

bool SdlWindowContext::handleSDLWindowEvent(const SDL_Event* event)
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

void SdlWindowContext::focus()
{
	if (!m_isShown)
	{
		SDL_ShowWindow(m_sdlWindow);
	}

	SDL_RaiseWindow(m_sdlWindow);
}

void SdlWindowContext::requestClose()
{
	if (m_sdlWindow)
	{
		SDL_HideWindow(m_sdlWindow);
	}
	m_wantsDestroy = true;
}

void SdlWindowContext::makeContextCurrent()
{
	int result = SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
	if (result != 0)
	{
		const char* errorMessage = SDL_GetError();
		MIKAN_LOG_ERROR("SdlWindowContext::makeContextCurrent") << "Error with SDL_GL_MakeCurrent: " << errorMessage;
	}

	// Keep graphics context width/height in sync with whichever window is currently active.
	// This is required for text rendering and other screen-space calculations when a single
	// GL context is shared across multiple windows.
	IMkGraphicsContextPtr graphicsContext = m_graphicsContext.lock();
	if (graphicsContext)
	{
		graphicsContext->onWindowSizeChanged(m_width, m_height);
	}
}

IMkWindowContextPtr createMkWindowContext(
	IMkWindowContextManagerPtr ownerWindowManager,
	IMkGraphicsContextPtr graphicsContext)
{
	// TODO: Use the graphics context API to determine which window type to create 
	// (e.g. OpenGL = SdlGlWindow, Vulkan = SdlVulkanWindow, etc.)
	return std::make_shared<SdlWindowContext>(ownerWindowManager, graphicsContext);
}
