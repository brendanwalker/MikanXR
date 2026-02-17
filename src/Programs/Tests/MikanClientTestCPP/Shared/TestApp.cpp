#include "TestApp.h"
#include "TestGraphicsContext_DX.h"
#include "TestGraphicsContext_GL.h"
#include "TestMikanClient.h"
#include "MikanAPI.h"
#include "Logger.h"

#if defined(_WIN32)
	#include <SDL.h>
	#include <SDL_events.h>
	#include <SDL_syswm.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_events.h>
	#include <SDL2/SDL_syswm.h>
#endif

static const int k_window_pixel_width = 1280;
static const int k_window_pixel_height = 720;
static const char* k_window_title = "MikanTestApp";

TestApp::TestApp()
	: m_graphicsContext()
	, m_mikanClient()
{
}

TestApp::~TestApp()
{
	shutdown();
}

int TestApp::exec(int argc, char** argv)
{
	int result = 0;

	if (startup(argc, argv))
	{
		SDL_Event e;

		while (!m_bShutdownRequested)
		{
			SDL_Window* sdlWindow = m_graphicsContext->getSDLWindow();

			// Update the frame rate
			const uint32_t now = SDL_GetTicks();
			const float deltaSeconds = fminf((float)(now - m_lastFrameTimestamp) / 1000.f, 0.1f);
			m_lastFrameTimestamp = now;

			// Update the current time
			m_timeSeconds += deltaSeconds;

			if (SDL_PollEvent(&e))
			{
				onSDLEvent(e);
			}

			// If the window is minimized, skip updating and rendering to avoid unnecessary CPU/GPU usage
			if (SDL_GetWindowFlags(sdlWindow) & SDL_WINDOW_MINIMIZED)
			{
				SDL_Delay(10);
				continue;
			}

			update(deltaSeconds);
			render();
		}
	}
	else
	{
		MIKAN_LOG_ERROR("exec") << "Failed to initialize application!";
		result = -1;
	}

	shutdown();

	return result;
}

bool TestApp::startup(int argc, char** argv)
{
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}

	// Create the graphics context based on command line arguments (if provided)
	if (argc > 1)
	{
		std::string graphicsApiArg = argv[1];

		if (graphicsApiArg == "-gl")
		{
			m_graphicsContext = std::make_unique<TestGraphicsContext_GL>(this);
		}
		else if (graphicsApiArg == "-dx")
		{
			m_graphicsContext = std::make_unique<TestGraphicsContext_DX>(this);
		}
		else
		{
			MIKAN_LOG_WARNING("startup") << "Invalid graphics API argument: " << graphicsApiArg << ". Using defaults.";
		}
	}

	// Create a default GL graphics context if one was not created based on command line arguments
	if (m_graphicsContext == nullptr)
	{
		m_graphicsContext = std::make_unique<TestGraphicsContext_GL>(this);
	}

	// Attempt to initialize the graphics context
	if (!m_graphicsContext->create(k_window_pixel_width, k_window_pixel_height))
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize graphics context";
	}

	// Initialize the Mikan API
	m_mikanClient = std::make_shared<TestMikanClient>(m_graphicsContext.get());
	if (!m_mikanClient->init(k_window_title))
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize Mikan client";
		return false;
	}

	return true;
}

void TestApp::shutdown()
{
	if (m_mikanClient)
	{
		m_mikanClient->dispose();
		m_mikanClient.reset();
	}

	if (m_graphicsContext)
	{
		m_graphicsContext->dispose();
		m_graphicsContext.reset();
	}

	SDL_Quit();
}

void TestApp::onSDLEvent(SDL_Event& e)
{
	SDL_Window* sdlWindow = m_graphicsContext->getSDLWindow();

	if (e.type == SDL_QUIT)
	{
		MIKAN_LOG_INFO("exec") << "QUIT message received.";
		requestShutdown();
	}
	else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
	{
		MIKAN_LOG_INFO("exec") << "ESC key press. Closing.";
		requestShutdown();
	}
	else if (e.type == SDL_WINDOWEVENT &&
			e.window.event == SDL_WINDOWEVENT_CLOSE && 
			e.window.windowID == SDL_GetWindowID(sdlWindow))
	{
		MIKAN_LOG_INFO("exec") << "Window close message received.";
		requestShutdown();
	}
	else if (e.type == SDL_WINDOWEVENT && 
			e.window.event == SDL_WINDOWEVENT_RESIZED && 
			e.window.windowID == SDL_GetWindowID(sdlWindow))
	{
		MIKAN_LOG_INFO("exec") << "Window resize event received.";
		m_graphicsContext->recreateMainRenderTarget();
	}
	else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
	{
		if (e.key.keysym.sym == SDLK_1)
		{
			m_renderMode = TestRenderMode::Color;
		}
		else if (e.key.keysym.sym == SDLK_2)
		{
			m_renderMode = TestRenderMode::DepthNormalize;
		}
		else if (e.key.keysym.sym == SDLK_w)
		{
			m_cubeOffset.z += 0.1f;
		}
		else if (e.key.keysym.sym == SDLK_s)
		{
			m_cubeOffset.z -= 0.1f;
		}
		else if (e.key.keysym.sym == SDLK_a)
		{
			m_cubeOffset.x += 0.1f;
		}
		else if (e.key.keysym.sym == SDLK_d)
		{
			m_cubeOffset.x -= 0.1f;
		}
		else if (e.key.keysym.sym == SDLK_q)
		{
			m_cubeOffset.y += 0.1f;
		}
		else if (e.key.keysym.sym == SDLK_e)
		{
			m_cubeOffset.y -= 0.1f;
		}
	}
}

void TestApp::update(float deltaSeconds)
{
	m_mikanClient->update(deltaSeconds);
}

void TestApp::render()
{
	m_graphicsContext->renderMainTarget();
}