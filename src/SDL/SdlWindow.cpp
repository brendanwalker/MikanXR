#include "SdlWindow.h"
#include "SdlManager.h"
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

#include "GlCommon.h"

#include <easy/profiler.h>

SdlWindow::SdlWindow()
	: m_title("Mikan Window")
{

}

SdlWindow* SdlWindow::setTitle(const std::string& title)
{
	if (m_sdlWindow != nullptr)
	{
		SDL_SetWindowTitle(m_sdlWindow, title.c_str());
	}

	m_title = title;

	return this;
}

SdlWindow* SdlWindow::setSize(int width, int height)
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

	return this;
}

bool SdlWindow::startup()
{
	assert(SdlManager::getInstance()->getIsSdlInitialized());

	bool success= true;

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
			SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
			SDL_GL_SetSwapInterval(1); // Enable vsync
		}
		else
		{
			MIKAN_LOG_ERROR("SdlWindow::startup") << "Unable to initialize SDL OpenGL context: " << SDL_GetError();
			success = false;
		}
	}

	if (success)
	{
		// Initialize GL Extension Wrangler (GLEW)
		GLenum err;
		glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
		err = glewInit();
		if (err != GLEW_OK)
		{
			MIKAN_LOG_ERROR("SdlWindow::startup") << "Unable to initialize glew openGL backend: " << glewGetErrorString(err);
			success = false;
		}
	}

	if (success)
	{
		// Grab window identifier
		m_windowId = SDL_GetWindowID(m_sdlWindow);

		// By default, this window has focus
		m_isShown = true;
		m_hasMouseFocus = true;
		m_hasKeyboardFocus = true;
	}

	return success;
}

void SdlWindow::shutdown()
{
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

void SdlWindow::onSDLEvent(const SDL_Event* event)
{
	//If an event was detected for this window
	if (event->type == SDL_WINDOWEVENT && event->window.windowID == m_windowId)
	{
		switch (event->window.event)
		{
			//Window appeared
			case SDL_WINDOWEVENT_SHOWN:
				m_isShown = true;
				break;

				//Window disappeared
			case SDL_WINDOWEVENT_HIDDEN:
				m_isShown = false;
				break;

				//Get new dimensions and repaint
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				m_width = event->window.data1;
				m_height = event->window.data2;
				break;

				//Mouse enter
			case SDL_WINDOWEVENT_ENTER:
				m_hasMouseFocus = true;
				break;

				//Mouse exit
			case SDL_WINDOWEVENT_LEAVE:
				m_hasMouseFocus = false;
				break;

				//Keyboard focus gained
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				m_hasKeyboardFocus = true;
				break;

				//Keyboard focus lost
			case SDL_WINDOWEVENT_FOCUS_LOST:
				m_hasKeyboardFocus = false;
				break;

				//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
				m_isMinimized = true;
				break;

				//Window maximized
			case SDL_WINDOWEVENT_MAXIMIZED:
				m_isMinimized = false;
				break;

				//Window restored
			case SDL_WINDOWEVENT_RESTORED:
				m_isMinimized = false;
				break;

				//Hide on close
			case SDL_WINDOWEVENT_CLOSE:
				SDL_HideWindow(m_sdlWindow);
				break;
		}
	}
}

void SdlWindow::focus()
{
	// Restore window if needed
	if (!m_isShown)
	{
		SDL_ShowWindow(m_sdlWindow);
	}

	// Move window forward
	SDL_RaiseWindow(m_sdlWindow);
}

void SdlWindow::renderBegin()
{
	EASY_FUNCTION();

	SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SdlWindow::renderEnd()
{
	EASY_FUNCTION();

	SDL_GL_SwapWindow(m_sdlWindow);
}