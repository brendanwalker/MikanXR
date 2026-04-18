//-- includes -----
#include "App.h"
#include "AppSettingsConfig.h"
#include "CommonConfig.h"
#include "EventBus.h"
#include "FrameTimer.h"
#include "Graphs/CompositorNodeGraph.h"
#include "IEditorWindow.h"
#include "IMkGraphicsContext.h"
#include "MkError.h"
#include "MkStateStack.h"
#include "LocalizationManager.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MikanModuleManager.h"
#include "PathUtils.h"
#include "ProjectConfig.h"
#include "IMkWindowManager.h"
#include "TypeRegistry.h"

//#include "Windows/TestNodeEditorWindow.h"
#include "Windows/CompositorNodeEditorWindow.h"

#include <easy/profiler.h>

#ifdef _WIN32
#include "Objbase.h"
#endif //_WIN32

#define SETTINGS_SAVE_COOLDOWN	3.f

//-- static members -----
App* App::m_instance= nullptr;

//-- App -----
App::App()
	: m_appSettings(std::make_shared<AppSettingsConfig>())
	, m_eventBus(std::make_unique<EventBus>())
	, m_localizationManager(new LocalizationManager())
	, m_windowManager(createMkWindowManager())
	, m_bShutdownRequested(false)
{
	m_instance= this;
}

App::~App()
{
	m_mainWindow = nullptr;

	m_windowManager.reset();
	delete m_localizationManager;

	m_appSettings.reset();

	// Clear the global event bus reference
	m_eventBus.reset();

	m_instance= nullptr;
}

int App::exec(int argc, char** argv)
{
	int result = 0;

	if (startup(argc, argv))
	{
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

IEditorWindow* App::getCurrentlyRenderingWindow() const
{ 
	return m_renderingWindow; 
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

	// Initialize registry of reflection types
	Serialization::TypeRegistry::buildFromRfkDatabase();

	// Initialize the module manager
	if (success && !initMikanModuleManager())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize module manager!";
		success = false;
	}

	// Load any saved app settings config
	if (success && !m_appSettings->load())
	{
		MIKAN_LOG_INFO("App::init") << "Failed to load app settings config. Creating new settings.";
	}

	// Enable auto-save on a cooldown when settings are changed
	m_appSettings->setAutoSaveCooldownDuration(SETTINGS_SAVE_COOLDOWN);

	if (success && !m_localizationManager->startup(m_appSettings))
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize localization manager!";
		success = false;
	}

	if (success && !m_windowManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize window manager!";
		success = false;
	}

	if (success)
	{
		// Register node graph factories spawned by windows
		NodeGraphFactory::registerFactory<CompositorNodeGraphFactory>();

		// Create the main window
		m_mainWindow = createAppWindow<MainWindow>();
		if (m_mainWindow == nullptr)
		{
			MIKAN_LOG_ERROR("App::init") << "Failed to initialize Main App Window!";
			success = false;
		}
	}

	if (success)
	{
		m_lastFrameTimestamp = std::chrono::steady_clock::now();
	}

	return success;
}

void App::shutdown()
{
	// Dispose all app windows (but the main window)
	while (m_appWindows.size() > 0)
	{
		EditorWindow* appWindow= m_appWindows[0];

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

	assert(m_windowManager != nullptr);
	m_windowManager->shutdown();

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
	const auto now = std::chrono::steady_clock::now();
	const float deltaSeconds = fminf(
		std::chrono::duration<float>(now - m_lastFrameTimestamp).count(),
		0.1f);
	m_fps = deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
	m_lastFrameTimestamp = now;

	// Refresh the latest events from SDL
	// Each window will process the events it cares about
	m_windowManager->pollEvents();

	// Tick the sim and then render each window
	tickWindows(deltaSeconds);

	// Update app settings auto-save
	m_appSettings->updateAutoSave(deltaSeconds);
}

void App::tickWindows(const float deltaSeconds)
{
	EASY_FUNCTION();

	assert(m_windowManager->getCurrentWindowContext() == nullptr);


	// Update each window
	static bool bDebugPrintStack = false;
	for (EditorWindow* appWindow : m_appWindows)
	{
		IMkWindow* appWindowContext = appWindow->getMkWindowContext().get();

		// Mark this window as the current window getting updated
		m_windowManager->pushCurrentWindowContext(appWindowContext);

		// Process window simulation based on time
		{
			EASY_BLOCK("UpdateWindow");
			appWindow->update(deltaSeconds);
		}

		// Render the window
		{
			EASY_BLOCK("RenderWindow");

			MkStateStack& mkStateStack = appWindow->getGraphicsContext()->getMkStateStack();
			mkStateStack.setDebugPrintEnabled(bDebugPrintStack);

			m_renderingWindow = appWindow;
			appWindowContext->render();
			m_renderingWindow = nullptr;

			mkStateStack.setDebugPrintEnabled(false);
		}

		// Restore back to the main window
		m_windowManager->popCurrentWindowContext(appWindowContext);
	}
	bDebugPrintStack = false;

	// Destroy any windows that have been marked for destruction
	for (int windowIndex= (int)m_appWindows.size() - 1; windowIndex >= 0; windowIndex--)
	{
		EditorWindow* window = m_appWindows[windowIndex];

		if (window->wantsDestroy())
		{
			// remove the window from the window list
			destroyAppWindow(window);
		}
	}
}

bool App::createAppWindowInternal(EditorWindow* appWindow)
{
	// Destroy the window if it fails to initialize properly
	if (!appWindow->startup())
	{
		destroyAppWindow(appWindow);
		return false;
	}

	// pop this window context this window added if it created one
	// and return back to the previous window context
	IMkWindow* appWindowContext = appWindow->getMkWindowContext().get();
	if (m_windowManager->getCurrentWindowContext() == appWindowContext)
	{
		m_windowManager->popCurrentWindowContext(appWindowContext);
	}

	// Add the window to the list of windows
	m_appWindows.push_back(appWindow);

	return true;
}

void App::destroyAppWindow(EditorWindow* appWindow)
{
	// If this window was the current window, pop it from the current window stack
	IMkWindow* appWindowContext = appWindow->getMkWindowContext().get();
	if (m_windowManager->getCurrentWindowContext() == appWindowContext)
	{
		m_windowManager->popCurrentWindowContext(appWindowContext);
	}

	// Tear down the window and graphics context it owns
	appWindow->shutdown();

	// Remove the window from the list of windows (should deallocate it)
	auto it = std::find(m_appWindows.begin(), m_appWindows.end(), appWindow);
	if (it != m_appWindows.end())
	{
		m_appWindows.erase(it);
	}

	// If this was the main window pointer, make sure to invalidate that pointer
	if (m_mainWindow == appWindow)
	{
		m_mainWindow = nullptr;
	}

	delete appWindow;
}