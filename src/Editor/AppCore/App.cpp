//-- includes -----
#include "App.h"
#include "FrameTimer.h"
#include "Graphs/CompositorNodeGraph.h"
#include "SdlCommon.h"
#include "MikanShaderCache.h"
#include "GlFrameCompositor.h"
#include "MkStateStack.h"
#include "MkError.h"
#include "MikanTextRenderer.h"
#include "LocalizationManager.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MikanModuleManager.h"
#include "PathUtils.h"
#include "ProfileConfig.h"
#include "SdlManager.h"
#include "SdlWindow.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"

//#include "Windows/TestNodeEditorWindow.h"
#include "Windows/CompositorNodeEditorWindow.h"

#include "SDL_timer.h"

#include <easy/profiler.h>

#ifdef _WIN32
#include "Objbase.h"
#endif //_WIN32

#define PROFILE_SAVE_COOLDOWN	3.f

//-- static members -----
App* App::m_instance= nullptr;

//-- App -----
App::App()
	: m_profileConfig(std::make_shared<ProfileConfig>())
	, m_localizationManager(new LocalizationManager())	
	, m_sdlManager(new SdlManager)
	, m_bShutdownRequested(false)
{
	m_instance= this;
}

App::~App()
{
	m_mainWindow = nullptr;

	delete m_sdlManager;
	delete m_localizationManager;

	m_profileConfig.reset();

	m_instance= nullptr;
}

int App::exec(int argc, char** argv)
{
	int result = 0;

	if (startup(argc, argv))
	{
		SDL_Event e;

		while (!m_bShutdownRequested && m_mainWindow != nullptr)
		{
			FrameTimer frameTimer(11); // 11ms = 90fps

			tick();

			frameTimer.waitForNextFrame();
		}
	}
	else
	{
		MIKAN_LOG_ERROR("App::exec") << "Failed to initialize application!";
		result = -1;
	}

	shutdown();

	return result;
}

//-- private methods -----
bool App::startup(int argc, char** argv)
{
	bool success = true;

	LoggerSettings settings = {};
	settings.min_log_level = LogSeverityLevel::debug;
	settings.enable_console= true;
	settings.log_filename= "MikanXR.log";

	log_init(settings);

	profiler::startListen();

#ifdef _WIN32
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (!SUCCEEDED(hr))
	{
		MIKAN_LOG_ERROR("App::init") << "Could not initialize COM";
		success = false;
	}
#endif // _WIN32

	// Initialize the module manager
	if (success && !initMikanModuleManager())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize module manager!";
		success = false;
	}

	// Load any saved config
	if (success && !m_profileConfig->load())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to load profile config!";
		success = false;
	}

	if (success && !m_localizationManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize localization manager!";
		success = false;
	}

	if (success && !m_sdlManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize SDL manager!";
		success = false;
	}

	// Register node graph factories spawned by windows
	NodeGraphFactory::registerFactory<CompositorNodeGraphFactory>();

	// Create the main window
	m_mainWindow= createAppWindow<MainWindow>();
	if (success && m_mainWindow == nullptr)
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize Main App Window!";
		success = false;
	}

	if (success)
	{
		m_lastFrameTimestamp= SDL_GetTicks();
	}

	return success;
}

void App::shutdown()
{
	// Dispose all app windows (but the main window)
	while (m_appWindows.size() > 0)
	{
		ISdlMkWindow* appWindow= m_appWindows[0];

		if (m_mainWindow != appWindow)
		{
			destroyAppWindow(appWindow);
		}
		else
		{
			auto it = std::find(m_appWindows.begin(), m_appWindows.end(), appWindow);
			if (it != m_appWindows.end())
			{
				m_appWindows.erase(it);
			}
		}
	}

	// Dispose the main window last
	if (m_mainWindow != nullptr)
	{
		destroyAppWindow(m_mainWindow);
		m_mainWindow = nullptr;
	}

	assert(m_sdlManager != nullptr);
	m_sdlManager->shutdown();

	assert(m_localizationManager != nullptr);
	m_localizationManager->shutdown();

	// Shutdown the module manager
	shutdownMikanModuleManager();

#ifdef _WIN32
	CoUninitialize();
#endif // _WIN32
}

void App::tick()
{
	EASY_FUNCTION();

	// Update the frame rate
	const uint32_t now = SDL_GetTicks();
	const float deltaSeconds = fminf((float)(now - m_lastFrameTimestamp) / 1000.f, 0.1f);
	m_fps = deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
	m_lastFrameTimestamp = now;

	// Refresh the latest events from SDL
	// Each window will process the events it cares about
	m_sdlManager->pollEvents();

	// Tick the sim and then render each window
	tickWindows(deltaSeconds);

	// Update profile auto-save
	updateAutoSave(deltaSeconds);
}

void App::tickWindows(const float deltaSeconds)
{
	EASY_FUNCTION();

	assert(m_glContextStack.size() == 0);


	// Update each window
	static bool bDebugPrintStack = false;
	for (ISdlMkWindow* window : m_appWindows)
	{
		// Mark this window as the current window getting updated
		pushCurrentGLContext(window);

		// Process window simulation based on time
		{
			EASY_BLOCK("UpdateWindow");
			window->update(deltaSeconds);
		}

		// Render the window
		{
			EASY_BLOCK("RenderWindow");

			MkStateStack& MkStateStack = window->getMkStateStack();
			MkStateStack.setDebugPrintEnabled(bDebugPrintStack);

			m_renderingWindow = window;
			window->render();
			m_renderingWindow = nullptr;

			MkStateStack.setDebugPrintEnabled(false);
		}

		// Restore back to the main window
		popCurrentGlContext(window);
	}
	bDebugPrintStack = false;

	// Destroy any windows that have been marked for destruction
	for (int windowIndex= (int)m_appWindows.size() - 1; windowIndex >= 0; windowIndex--)
	{
		ISdlMkWindow* window = m_appWindows[windowIndex];

		if (window->getSdlWindow().wantsDestroy())
		{
			// remove the window from the window list
			destroyAppWindow(window);
		}
	}
}

void App::pushCurrentGLContext(ISdlMkWindow* window)
{
	if (m_glContextStack.size() == 0 || m_glContextStack.back() != window)
	{
		// Add the window to the window stack
		m_glContextStack.push_back(window);

		// Make the window's GL context current
		window->getSdlWindow().makeGlContextCurrent();
	}
	else
	{
		MIKAN_LOG_WARNING("App::popCurrentWindow")
			<< "Unable to push window "
			<< window->getSdlWindow().getTitle()
			<< " (already current)";
	}
}

ISdlMkWindow* App::getCurrentGlContext() const
{
	return m_glContextStack.size() > 0 ? m_glContextStack.back() : nullptr;
}

void App::popCurrentGlContext(ISdlMkWindow* window)
{
	if (checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__))
	{
		MIKAN_LOG_ERROR("App::popCurrentWindow") << "Unhandled GL error found before popping window";
	}

	if (m_glContextStack.size() > 0 && m_glContextStack.back() == window)
	{
		// Remove the window from the window stack
		m_glContextStack.pop_back();

		// Make the previous window's GL context current
		if (m_glContextStack.size() > 0)
		{
			m_glContextStack.back()->getSdlWindow().makeGlContextCurrent();
		}
	}
	else
	{
		MIKAN_LOG_ERROR("App::popCurrentWindow") 
			<< "Unable to pop window " 
			<< window->getSdlWindow().getTitle()
			<< " (not current)";
	}
}

void App::updateAutoSave(float deltaSeconds)
{
	// We change the profile constantly as changes are made in the UI
	// Put the save to disk on a cooldown so we aren't writing to disk constantly
	if (m_profileSaveCooldown >= 0.f)
	{
		if (m_profileConfig->isMarkedDirty())
		{
			m_profileSaveCooldown -= deltaSeconds;
			if (m_profileSaveCooldown < 0.f)
			{
				m_profileConfig->save();
				m_profileSaveCooldown = -1.f;
			}
		}
		else
		{
			m_profileSaveCooldown = -1.f;
		}
	}
	else if (m_profileConfig->isMarkedDirty())
	{
		m_profileSaveCooldown = PROFILE_SAVE_COOLDOWN;
	}
}