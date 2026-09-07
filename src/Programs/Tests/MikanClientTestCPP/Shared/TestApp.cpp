#include "TestApp.h"
#include "TestGraphicsContext_DX.h"
#include "TestGraphicsContext_GL.h"
#include "TestGraphicsContext_VK.h"
#include "TestCameraRenderTarget.h"
#include "TestFrameDump.h"
#include "TestMikanClient.h"
#include "TestSpoutProbe.h"
#include "MikanAPI.h"
#include "SharedTextureUtility.h"
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

#include <cstdlib>

static const int k_window_pixel_width= 1280;
static const int k_window_pixel_height= 720;
static const char* k_window_title= "MikanTestApp";

TestApp::TestApp()
	: m_graphicsContext()
	, m_mikanClient()
{
}

TestApp::~TestApp() { shutdown(); }

int TestApp::exec(int argc, char** argv)
{
	int result= 0;

	if (startup(argc, argv))
	{
		SDL_Event e;

		while (!m_bShutdownRequested)
		{
			SDL_Window* sdlWindow= m_graphicsContext->getSDLWindow();

			// Update the frame rate
			const uint32_t now= SDL_GetTicks();
			const float deltaSeconds= fminf((float)(now - m_lastFrameTimestamp) / 1000.f, 0.1f);
			m_lastFrameTimestamp= now;

			// Update the current time
			m_timeSeconds+= deltaSeconds;

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
		result= -1;
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
		std::string graphicsApiArg= argv[1];

		if (graphicsApiArg == "-gl")
		{
			m_graphicsContext= std::make_unique<TestGraphicsContext_GL>(this);
		}
		else if (graphicsApiArg == "-dx")
		{
			m_graphicsContext= std::make_unique<TestGraphicsContext_DX>(this);
		}
		else if (graphicsApiArg == "-vk")
		{
			m_graphicsContext= std::make_unique<TestGraphicsContext_VK>(this);
		}
		else
		{
			MIKAN_LOG_WARNING("startup") << "Invalid graphics API argument: " << graphicsApiArg << ". Using defaults.";
		}
	}

	// Optional arguments after the graphics API:
	//   -dump <png path>   write the connected camera's frame and its Spout senders once
	//   -cube <x> <y> <z>  place the cube in meters from the camera (x right, y up, z forward)
	for (int argIndex= 2; argIndex < argc; ++argIndex)
	{
		const std::string argument= argv[argIndex];

		if (argument == "-dump" && argIndex + 1 < argc)
		{
			m_dumpPath= argv[argIndex + 1];
			m_bDumpPending= true;
		}
		else if (argument == "-cube" && argIndex + 3 < argc)
		{
			m_cubeOffset.x= (float)std::atof(argv[argIndex + 1]);
			m_cubeOffset.y= (float)std::atof(argv[argIndex + 2]);
			m_cubeOffset.z= (float)std::atof(argv[argIndex + 3]);
		}
	}

	// Create a default GL graphics context if one was not created based on command line arguments
	if (m_graphicsContext == nullptr)
	{
		m_graphicsContext= std::make_unique<TestGraphicsContext_GL>(this);
	}

	// Attempt to initialize the graphics context
	if (!m_graphicsContext->create(k_window_pixel_width, k_window_pixel_height))
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize graphics context";
	}

	// Initialize the Mikan API
	m_mikanClient= std::make_shared<TestMikanClient>(m_graphicsContext.get());
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

IMikanAPIPtr TestApp::getMikanAPI() const { return m_mikanClient->getMikanAPI(); }

void TestApp::onSDLEvent(SDL_Event& e)
{
	SDL_Window* sdlWindow= m_graphicsContext->getSDLWindow();

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
	else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE
			 && e.window.windowID == SDL_GetWindowID(sdlWindow))
	{
		MIKAN_LOG_INFO("exec") << "Window close message received.";
		requestShutdown();
	}
	else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED
			 && e.window.windowID == SDL_GetWindowID(sdlWindow))
	{
		MIKAN_LOG_INFO("exec") << "Window resize event received.";
		m_graphicsContext->recreateMainRenderTarget();
	}
	else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
	{
		const bool bIsShiftDown = (SDL_GetModState() & KMOD_SHIFT) != 0;

		if (e.key.keysym.sym == SDLK_1)
		{
			m_renderMode= TestRenderMode::Color;
		}
		else if (e.key.keysym.sym == SDLK_2)
		{
			m_renderMode= TestRenderMode::DepthNormalize;
		}
		else if (e.key.keysym.sym == SDLK_3)
		{
			m_renderMode= TestRenderMode::PackedDepth;
		}
		else if (e.key.keysym.sym == SDLK_w)
		{
			m_cubeOffset.z+= bIsShiftDown ? 0.1f : 0.01f;
		}
		else if (e.key.keysym.sym == SDLK_s)
		{
			m_cubeOffset.z-= bIsShiftDown ? 0.1f : 0.01f;
		}
		else if (e.key.keysym.sym == SDLK_a)
		{
			m_cubeOffset.x+= bIsShiftDown ? 0.1f : 0.01f;
		}
		else if (e.key.keysym.sym == SDLK_d)
		{
			m_cubeOffset.x-= bIsShiftDown ? 0.1f : 0.01f;
		}
		else if (e.key.keysym.sym == SDLK_q)
		{
			m_cubeOffset.y+= bIsShiftDown ? 0.1f : 0.01f;
		}
		else if (e.key.keysym.sym == SDLK_e)
		{
			m_cubeOffset.y-= bIsShiftDown ? 0.1f : 0.01f;
		}
	}
}

void TestApp::update(float deltaSeconds) { m_mikanClient->update(deltaSeconds); }

void TestApp::render()
{
	m_graphicsContext->renderMainTarget();

	if (m_bDumpPending && m_timeSeconds >= k_dumpDelaySeconds)
	{
		dumpCameraFrame();
		m_bDumpPending= false;
	}
}

void TestApp::dumpCameraFrame()
{
	const MikanCameraID cameraId= m_graphicsContext->getLastRenderedCameraId();
	TestCameraRenderTargetPtr renderTarget= m_graphicsContext->getCameraRenderTarget(cameraId);
	if (!renderTarget)
	{
		MIKAN_LOG_ERROR("dumpCameraFrame") << "No camera has been rendered yet";
		return;
	}

	std::vector<uint8_t> rgbaPixels;
	int width= 0;
	int height= 0;
	if (!m_graphicsContext->readCameraTargetPixels(renderTarget.get(), rgbaPixels, width, height))
	{
		MIKAN_LOG_ERROR("dumpCameraFrame") << "The graphics context could not read back the camera target";
		return;
	}

	if (writeRgbaPng(m_dumpPath, rgbaPixels, width, height))
	{
		MIKAN_LOG_INFO("dumpCameraFrame")
			<< "Wrote camera " << cameraId << " (" << width << "x" << height << ") to " << m_dumpPath;
	}
	else
	{
		MIKAN_LOG_ERROR("dumpCameraFrame") << "Failed to write " << m_dumpPath;
	}

	// For a camera Mikan knows, also read the shared textures back through Spout so the dump shows
	// what the editor's reader would see, beside what the client rendered
	if (cameraId == INVALID_MIKAN_ID)
		return;

	const struct
	{
		SharedTextureType type;
		const char* suffix;
	} senders[]= {{SharedTextureType::COLOR, ".spout.png"}, {SharedTextureType::DEPTH, ".depth.spout.png"}};

	for (const auto& sender : senders)
	{
		std::string senderName;
		if (!makeSpoutSenderName(k_window_title, cameraId, sender.type, senderName))
			continue;

		std::vector<uint8_t> senderPixels;
		int senderWidth= 0;
		int senderHeight= 0;
		const std::string senderDumpPath= m_dumpPath + sender.suffix;
		if (readSpoutSenderRgba(senderName, senderPixels, senderWidth, senderHeight)
			&& writeRgbaPng(senderDumpPath, senderPixels, senderWidth, senderHeight))
		{
			MIKAN_LOG_INFO("dumpCameraFrame") << "Wrote Spout sender " << senderName << " (" << senderWidth << "x"
											  << senderHeight << ") to " << senderDumpPath;
		}
		else
		{
			MIKAN_LOG_ERROR("dumpCameraFrame") << "Failed to read back Spout sender " << senderName;
		}
	}
}