//-- includes -----
#include "App.h"
#include "AppStage.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "FontManager.h"
#include "FrameTimer.h"
#include "GlShaderCache.h"
#include "GlFrameCompositor.h"
#include "GlTextRenderer.h"
#include "InputManager.h"
#include "LocalizationManager.h"
#include "Logger.h"
#include "MainWindow.h"
#include "NodeEditorWindow.h"
#include "MikanServer.h"
#include "ObjectSystemManager.h"
#include "OpenCVManager.h"
#include "PathUtils.h"
#include "ProfileConfig.h"
#include "RmlManager.h"
#include "SdlManager.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"

#include "SDL_timer.h"

#include <easy/profiler.h>

#ifdef _WIN32
#include "Objbase.h"
#endif //_WIN32

#define PROFILE_SAVE_COOLDOWN	3.f

//-- static members -----
App* App::m_instance= nullptr;

//-- public methods -----
App::App()
	: m_profileConfig(std::make_shared<ProfileConfig>())
	, m_mikanServer(new MikanServer())
	, m_frameCompositor(new GlFrameCompositor())
	, m_inputManager(new InputManager())
	, m_rmlManager(new RmlManager(this))
	, m_localizationManager(new LocalizationManager())	
	, m_objectSystemManager(std::make_shared<ObjectSystemManager>())
	, m_openCVManager(new OpenCVManager())
	, m_sdlManager(std::unique_ptr<SdlManager>(new SdlManager))
	, m_fontManager(new FontManager())
	, m_videoSourceManager(new VideoSourceManager())
	, m_vrDeviceManager(new VRDeviceManager())
	, m_bShutdownRequested(false)
{
	m_instance= this;
}

App::~App()
{
	m_objectSystemManager = nullptr;
	m_mainWindow = nullptr;
	m_openCVManager= nullptr;

	delete m_vrDeviceManager;
	delete m_videoSourceManager;
	delete m_localizationManager;
	delete m_inputManager;
	delete m_rmlManager;
	delete m_mikanServer;
	delete m_frameCompositor;
	m_profileConfig.reset();

	m_instance= nullptr;
}

int App::exec(int argc, char** argv)
{
	int result = 0;

	if (startup(argc, argv))
	{
		SDL_Event e;

		pushAppStage<AppStage_MainMenu>();

		while (!m_bShutdownRequested)
		{
			FrameTimer frameTimer(11); // 11ms = 90fps

			if (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT ||
					(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
				{
					MIKAN_LOG_INFO("App::exec") << "QUIT message received";
					break;
				}
				else
				{
					onSDLEvent(e);
				}
			}

			update();
			render();

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

	// Load any saved config
	m_profileConfig->load();

	if (success && !m_rmlManager->preRendererStartup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize Rml UI manager!";
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

	if (success && !m_openCVManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize OpenCV manager!";
		success = false;
	}

	m_mainWindow= createAppWindow<MainWindow>();
	if (success && m_mainWindow == nullptr)
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize Main App Window!";
		success = false;
	}

	if (success && !m_fontManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize baked text cache!";
		success = false;
	}

	if (success && !m_videoSourceManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the video source manager";
		success = false;
	}

	if (success && !m_vrDeviceManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the vr tracker manager";
		success = false;
	}

	if (success && !m_objectSystemManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the object system manager";
		success = false;
	}

	if (success && !m_frameCompositor->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the frame compositor";
		success = false;
	}

	if (success && !m_mikanServer->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the MikanXR server";
		success = false;
	}

	if (success && !m_rmlManager->postRendererStartup(m_mainWindow))
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize Rml UI manager!";
		success = false;
	}

	// TODO: Test node editor
	createAppWindow<NodeEditorWindow>();

	if (success)
	{
		m_lastFrameTimestamp= SDL_GetTicks();
	}

	return success;
}

void App::shutdown()
{
	// Tear down all active app stages
	while (getCurrentAppStage() != nullptr)
	{
		popAppState();
	}
	processPendingAppStageOps();

	// Tear down all app systems
	assert(m_rmlManager != nullptr);
	m_rmlManager->shutdown();

	assert(m_mikanServer != nullptr);
	m_mikanServer->shutdown();

	assert(m_frameCompositor != nullptr);
	m_frameCompositor->shutdown();

	// Dispose all ObjectSystems
	assert(m_objectSystemManager != nullptr);
	m_objectSystemManager->shutdown();

	assert(m_videoSourceManager != nullptr);
	m_videoSourceManager->shutdown();

	assert(m_vrDeviceManager != nullptr);
	m_vrDeviceManager->shutdown();

	assert(m_fontManager != nullptr);
	m_fontManager->shutdown();

	// Dispose all app windows
	while (m_appWindows.size() > 0)
	{
		destroyAppWindow(m_appWindows[0]);
	}
	m_mainWindow= nullptr;

	assert(m_sdlManager != nullptr);
	m_sdlManager->shutdown();

	assert(m_openCVManager != nullptr);
	m_openCVManager->shutdown();

	assert(m_localizationManager != nullptr);
	m_localizationManager->shutdown();

#ifdef _WIN32
	CoUninitialize();
#endif // _WIN32
}

void App::onSDLEvent(SDL_Event& e)
{
	m_inputManager->onSDLEvent(e);

	for (IGlWindow* window : m_appWindows)
	{
		window->onSDLEvent(&e);
	}

	AppStage *appStage= getCurrentAppStage();
	if (appStage != nullptr)
	{
		appStage->onSDLEvent(&e);
	}
}

void App::update()
{
	EASY_FUNCTION();

	// Update the frame rate
	const uint32_t now = SDL_GetTicks();
	const float deltaSeconds= fminf((float)(now - m_lastFrameTimestamp) / 1000.f, 0.1f);
	m_fps= deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
	m_lastFrameTimestamp= now;

	// Update all connected devices
	m_videoSourceManager->update(deltaSeconds);
	m_vrDeviceManager->update(deltaSeconds);

	// Poll rendered frames from client connections
	m_mikanServer->update();

	// Update any frame compositing state based on new video frames or client render target updates
	m_frameCompositor->update();

	// Garbage collect stale baked text
	m_fontManager->garbageCollect();

	// Process any pending app stage operations queued by pushAppStage/popAppStage from last frame
	processPendingAppStageOps();

	// Update the current app stage last
	AppStage* appStage = getCurrentAppStage();
	if (appStage != nullptr && appStage->getIsUpdateActive())
	{
		EASY_BLOCK("appStage Update");
		appStage->update(deltaSeconds);
	}

	// Update the UI layout and data models
	m_rmlManager->update();

	// Update profile auto-save
	updateAutoSave(deltaSeconds);
}

void App::processPendingAppStageOps()
{
	// Disallow app stack operations during enter or exit
	bAppStackOperationAllowed = false;

	InputManager* inputManager = InputManager::getInstance();
	for (auto& pendingAppStageOp : m_pendingAppStageOps)
	{
		switch (pendingAppStageOp.op)
		{
			case AppStageOperation::enter:
			{
				EASY_BLOCK("appStage Enter");

				// Pause the parent app stage
				if (pendingAppStageOp.parentAppStage != nullptr)
					pendingAppStageOp.parentAppStage->pause();

				// Create a new input event set for the app state
				inputManager->pushEventBindingSet();

				// Enter the new app stage
				pendingAppStageOp.appStage->enter();

				// Notify any object systems that care about app stage transitions 
				if (OnAppStageEntered)
					OnAppStageEntered(pendingAppStageOp.appStage);
			} break;
			case AppStageOperation::exit:
			{
				EASY_BLOCK("appStage Exit");

				// Notify any object systems that care about app stage transitions 
				if (OnAppStageExited)
					OnAppStageEntered(pendingAppStageOp.appStage);

				// Exit the app stage we are leaving
				pendingAppStageOp.appStage->exit();

				// Clean up the input event set for the deactivated app stage
				inputManager->popEventBindingSet();

				// Resume the parent app stage we are restoring (if any)
				if (pendingAppStageOp.parentAppStage != nullptr)
					pendingAppStageOp.parentAppStage->resume();

				// Free the app state
				delete pendingAppStageOp.appStage;
			} break;
		}
	}
	m_pendingAppStageOps.clear();

	// App stack operations allowed during update
	bAppStackOperationAllowed = true;
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

void App::render()
{
	EASY_FUNCTION();

	for (IGlWindow* window : m_appWindows)
	{
		m_renderingWindow = window;
		window->render();
		m_renderingWindow = nullptr;
	}
}
