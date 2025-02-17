#include "App.h"
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

#include "SdlCommon.h"

#include <easy/profiler.h>

static void APIENTRY GLDebugMessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

SdlWindow::SdlWindow(class ISdlMkWindow* ownerWindowInterface)
	: m_owner(ownerWindowInterface)
	, m_title("Mikan Window")
{
}

SdlWindow* SdlWindow::enableGLDataSharing()
{
	// This needs to be called before the SDL window is created to have any effect
	assert(m_sdlWindow == nullptr);
	m_bGLDataSharingEnabled= true;
	return this;
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

	// This flag allows new windows to share textures created in the main window.
	// (ex: video textures owned by the frame compositor)
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
			// Make this window the current GL context
			// While in this GL context scope, all GL calls will be made to this window
			App::getInstance()->pushCurrentGLContext(m_owner);

			// For now, disable vsync to since it causes SDL_GL_SwapWindow to subvert the frame delay
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
		// Initialize GL Extension Wrangler (GLEW)
		GLenum err;
		glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
		err = glewInit();
		if (err == GLEW_OK)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(GLDebugMessageCallback, m_glContext);
		}
		else
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

void SdlWindow::handleSDLEvents()
{
	SDL_Event event;

	std::vector<SDL_Event>& events= SdlManager::getInstance()->getEvents();
	auto it = events.begin();
	while (it != events.end())
	{
		SDL_Event& event= *it;

		// First see if this is an SDL window event
		bool bHandled = handleSDLWindowEvent(&event);

		// Then see if the owner wants to handle it
		if (!bHandled)
		{
			bHandled= m_owner->onSDLEvent(&event);
		}

		// Remove any SDL events that were handled
		if (bHandled)
		{
			it= events.erase(it);
		}
		else
		{
			it++;
		}
	}
}

bool SdlWindow::handleSDLWindowEvent(const SDL_Event* event)
{
	// Assume the event was handled
	bool bHandled = true;

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
				m_wantsDestroy= true;
				break;
			
			default:
				// Not a window event we care about, pass it along
				bHandled= false;
		}
	}
	else
	{
		// Not a window event, pass it along
		bHandled= false;
	}

	return bHandled;
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

void SdlWindow::makeGlContextCurrent()
{
	int result = SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
	if (result != 0)
	{
		const char* errorMessage = SDL_GetError();
		MIKAN_LOG_ERROR("App::update") << "Error with SDL_GL_MakeCurrent: " << errorMessage;
	}
}

void SdlWindow::renderBegin()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SdlWindow::renderEnd()
{
	EASY_FUNCTION();

	SDL_GL_SwapWindow(m_sdlWindow);
}

enum class eGLErrorSeverity : int
{
	unknown,
	notification,
	low,
	medium,
	high,
};
static const char* g_szGLErrorSeverityNames[] = {
	"UNKNOWN",
	"NOTIFICATION",
	"LOW",
	"MEDIUM",
	"HIGH",
};

static void APIENTRY GLDebugMessageCallback(
	GLenum glMesgSource,
	GLenum glMesgType,
	GLuint glMesgId,
	GLenum glMesgSeverity,
	GLsizei length,
	const GLchar* szGlMessage,
	const void* userParam)
{
	std::string glErrorSourceStr;
	std::string glErrorTypeStr;

	switch (glMesgSource)
	{
		case GL_DEBUG_SOURCE_API:
			glErrorSourceStr = "API";
			break;

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			glErrorSourceStr = "WINDOW SYSTEM";
			break;

		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			glErrorSourceStr = "SHADER COMPILER";
			break;

		case GL_DEBUG_SOURCE_THIRD_PARTY:
			glErrorSourceStr = "THIRD PARTY";
			break;

		case GL_DEBUG_SOURCE_APPLICATION:
			glErrorSourceStr = "APPLICATION";
			break;

		case GL_DEBUG_SOURCE_OTHER:
			glErrorSourceStr = "UNKNOWN";
			break;

		default:
			glErrorSourceStr = "UNKNOWN";
			break;
	}

	eGLErrorSeverity minSeverity= eGLErrorSeverity::low;
	switch (glMesgType)
	{
		case GL_DEBUG_TYPE_ERROR:
			glErrorTypeStr = "ERROR";
			minSeverity= eGLErrorSeverity::notification;
			break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			glErrorTypeStr = "DEPRECATED BEHAVIOR";
			minSeverity= eGLErrorSeverity::high;
			break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			glErrorTypeStr = "UDEFINED BEHAVIOR";
			minSeverity= eGLErrorSeverity::notification;
			break;

		case GL_DEBUG_TYPE_PORTABILITY:
			glErrorTypeStr = "PORTABILITY";
			minSeverity= eGLErrorSeverity::high;
			break;

		case GL_DEBUG_TYPE_PERFORMANCE:
			glErrorTypeStr = "PERFORMANCE";
			minSeverity= eGLErrorSeverity::high;
			break;

		case GL_DEBUG_TYPE_OTHER:
			glErrorTypeStr = "OTHER";
			minSeverity= eGLErrorSeverity::low;
			break;

		case GL_DEBUG_TYPE_MARKER:
			glErrorTypeStr = "MARKER";
			minSeverity= eGLErrorSeverity::low;
			break;

		default:
			glErrorTypeStr = "UNKNOWN";
			minSeverity= eGLErrorSeverity::low;
			break;
	}

	eGLErrorSeverity eventSeverity= eGLErrorSeverity::unknown;
	switch (glMesgSeverity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			eventSeverity= eGLErrorSeverity::high;
			break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			eventSeverity= eGLErrorSeverity::medium;
			break;

		case GL_DEBUG_SEVERITY_LOW:
			eventSeverity= eGLErrorSeverity::low;
			break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
			eventSeverity= eGLErrorSeverity::notification;
			break;

		default:
			eventSeverity= eGLErrorSeverity::unknown;
			break;
	}

	// ignore notification severity (you can add your own ignores)
	if ((int)eventSeverity >= (int)minSeverity)
	{
		switch (eventSeverity)
		{
			case eGLErrorSeverity::high:
			case eGLErrorSeverity::unknown:
				{
					MIKAN_LOG_ERROR("GlDebugCallback") <<
						"OpenGL debug event [" << glMesgId << "]: "
						<< glErrorTypeStr << " of "
						<< g_szGLErrorSeverityNames[(int)eventSeverity] << " severity, raised from "
						<< glErrorSourceStr << ": "
						<< szGlMessage;
				}
				break;
			case eGLErrorSeverity::medium:
				{
					MIKAN_LOG_WARNING("GlDebugCallback") <<
						"OpenGL debug event [" << glMesgId << "]: "
						<< glErrorTypeStr << " of "
						<< g_szGLErrorSeverityNames[(int)eventSeverity] << " severity, raised from "
						<< glErrorSourceStr << ": "
						<< szGlMessage;
				}
				break;
			case eGLErrorSeverity::low:
			case eGLErrorSeverity::notification:
				{
					MIKAN_LOG_INFO("GlDebugCallback") <<
						"OpenGL debug event [" << glMesgId << "]: "
						<< glErrorTypeStr << " of "
						<< g_szGLErrorSeverityNames[(int)eventSeverity] << " severity, raised from "
						<< glErrorSourceStr << ": "
						<< szGlMessage;
				}
				break;
			default:
				assert(false);
		}

	#ifdef _DEBUG
		if ((int)eventSeverity >= (int)eGLErrorSeverity::high)
		{ 
			__debugbreak();
		}
	#endif
	}
}
